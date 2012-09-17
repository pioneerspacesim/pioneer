#include "Loader.h"
#include "FileSystem.h"
#include "LOD.h"
#include "Newmodel.h"
#include "Parser.h"
#include "StaticGeometry.h"
#include "StringF.h"
#include "utils.h"
#include "graphics/Renderer.h"
#include "graphics/Surface.h"
#include "graphics/TextureBuilder.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/material.h>

//debugging
#include <iostream>

namespace Newmodel {

Loader::Loader(Graphics::Renderer *r) :
	m_renderer(r),
	m_model(0)
{
	Graphics::Texture *sdfTex = Graphics::TextureBuilder("fonts/label3d.png", Graphics::LINEAR_CLAMP, true, true, true).GetOrCreateTexture(r, "model");
	m_labelFont.Reset(new Text::DistanceFieldFont("fonts/sdf_definition.fnt", sdfTex));
}

Loader::~Loader()
{
}

NModel *Loader::LoadModel(const std::string &filename)
{
	NModel *m = LoadModel(filename, "newmodels");
	return m;
}

NModel *Loader::LoadModel(const std::string &shortname, const std::string &basepath)
{
	for (FileSystem::FileEnumerator files(FileSystem::gameDataFiles, basepath, FileSystem::FileEnumerator::Recurse); !files.Finished(); files.Next())
	{
		const FileSystem::FileInfo &info = files.Current();
		const std::string &fpath = info.GetAbsolutePath();

		//check it's the expected type
		if (info.IsFile() && ends_with(fpath, ".model")) {
			//check it's the wanted name & load it
			const std::string name = info.GetName();

			if (shortname == name.substr(0, name.length()-6)) {
				ModelDefinition modelDefinition;
				try {
					//curPath is used to find patterns, possibly other data
					//files for this model. Strip trailing slash
					m_curPath = info.GetDir();
					assert(!m_curPath.empty());
					if (m_curPath[m_curPath.length()-1] == '/')
						m_curPath = m_curPath.substr(0, m_curPath.length()-1);

					Parser p(fpath, m_curPath);
					p.Parse(&modelDefinition);
				} catch (const std::string &str) {
					std::cerr << str << std::endl;
					throw LoadingError(str);
				}
				modelDefinition.name = shortname;
				return CreateModel(modelDefinition);
			}
		}

	}
	throw (LoadingError("File not found"));
}

Graphics::Texture *Loader::GetWhiteTexture() const
{
	return Graphics::TextureBuilder::Model("textures/white.png").GetOrCreateTexture(m_renderer, "model");
}

NModel *Loader::CreateModel(ModelDefinition &def)
{
	using Graphics::Material;
	if (def.matDefs.empty()) return 0;
	if (def.lodDefs.empty()) return 0;

	NModel *model = new NModel(def.name);
	m_model = model;
	bool patternsUsed = false;

	//create materials from definitions
	for(std::vector<MaterialDefinition>::const_iterator it = def.matDefs.begin();
		it != def.matDefs.end(); ++it)
	{
		//Build material descriptor
		assert(!(*it).name.empty());
		const std::string &diffTex = (*it).tex_diff;
		const std::string &specTex = (*it).tex_spec;
		const std::string &glowTex = (*it).tex_glow;

		Graphics::MaterialDescriptor matDesc;
		matDesc.lighting = true;

		if ((*it).use_pattern) {
			patternsUsed = true;
			matDesc.usePatterns = true;
		}

		//diffuse texture is a must. Will create a white dummy texture if one is not supplied
		matDesc.textures = 1;
		matDesc.specularMap = !specTex.empty();
		matDesc.glowMap = !glowTex.empty();

		//Create material and set parameters
		RefCountedPtr<Material> mat(m_renderer->CreateMaterial(matDesc));
		mat->diffuse = (*it).diffuse;
		mat->specular = (*it).specular;
		mat->emissive = (*it).emissive;
		mat->shininess = (*it).shininess;

		if (!diffTex.empty())
			mat->texture0 = Graphics::TextureBuilder::Model(diffTex).GetOrCreateTexture(m_renderer, "model");
		else
			mat->texture0 = GetWhiteTexture();
		if (!specTex.empty())
			mat->texture1 = Graphics::TextureBuilder::Model(specTex).GetOrCreateTexture(m_renderer, "model");
		if (!glowTex.empty())
			mat->texture2 = Graphics::TextureBuilder::Model(glowTex).GetOrCreateTexture(m_renderer, "model");
		//texture3 is reserved for pattern
		//texture4 is reserved for color gradient

		model->m_materials.push_back(std::make_pair<std::string, RefCountedPtr<Material> >((*it).name, mat));
	}
	//printf("Loaded %d materials\n", int(model->m_materials.size()));

	//load meshes
	std::map<std::string, Node*> meshCache;
	LOD *lodNode = 0;
	if (def.lodDefs.size() > 1) { //don't bother with a lod node if only one level
		lodNode = new LOD();
		model->GetRoot()->AddChild(lodNode);
	}
	for(std::vector<LodDefinition>::const_iterator lod = def.lodDefs.begin();
		lod != def.lodDefs.end(); ++lod)
	{
		//does a detail level have multiple meshes? If so, we need a Group.
		Group *group = 0;
		if (lodNode && (*lod).meshNames.size() > 1) {
			group = new Group();
			lodNode->AddLevel((*lod).pixelSize, group);
		}
		for(std::vector<std::string>::const_iterator it = (*lod).meshNames.begin();
			it != (*lod).meshNames.end(); ++it)
		{
			try {
				//multiple lods might use the same mesh
				Node *mesh = 0;
				std::map<std::string, Node*>::iterator cacheIt = meshCache.find((*it));
				if (cacheIt != meshCache.end())
					mesh = (*cacheIt).second;
				else {
					mesh = LoadMesh(*(it), model, def.animDefs, def.tagDefs);
					meshCache[*(it)] = mesh;
				}

				if (group)
					group->AddChild(mesh);
				else if(lodNode)
					lodNode->AddLevel((*lod).pixelSize, mesh);
				else
					model->GetRoot()->AddChild(mesh);
			} catch (LoadingError &) {
				delete model;
				throw;
			} catch (const std::string &s) {
				delete model;
				std::cerr << s << std::endl;
				throw LoadingError(s);
			}
		}
	}

	// Load collision meshes
	for (std::vector<std::string>::const_iterator it = def.collisionDefs.begin();
		it != def.collisionDefs.end(); ++it)
	{
		LoadCollision(*it);
	}
	// No CM supplied? Autogenerate a simple BB.
	if (!m_model->m_collMesh.Valid()) {
		m_model->CreateCollisionMesh(0);
	}

	Animation *anim = 0;

	//add some dummy tag points
	for(TagList::const_iterator it = def.tagDefs.begin();
		it != def.tagDefs.end();
		++it)
	{
		//if (!anim) anim = new Animation("wiggle", 100.0, Animation::ONCE);
		const vector3f &pos = (*it).position;
		MatrixTransform *tagTrans = new MatrixTransform(matrix4x4f::Translation(pos.x, pos.y, pos.z));
		//add a test animation for the tag (which is silly)
		/*anim->channels.push_back(AnimationChannel(tagTrans));
		AnimationChannel &chan = anim->channels.back();
		chan.rotationKeys.push_back(RotationKey(0.0, Quaternionf(1.f, 0.f, 0.f, 0.f)));
		chan.rotationKeys.push_back(RotationKey(50.0, Quaternionf(1.5707f, vector3f(1.f, 0.f, 0.f))));
		chan.rotationKeys.push_back(RotationKey(100.0, Quaternionf(1.f, 0.f, 0.f, 0.f)));
		chan.positionKeys.push_back(PositionKey(0.0, pos));
		chan.positionKeys.push_back(PositionKey(50.0, pos + vector3f(0.f, 3.f, 0.f)));
		chan.positionKeys.push_back(PositionKey(100.0, pos));*/
		model->AddTag((*it).name, tagTrans);
	}

	if (anim) model->m_animations.push_back(anim);

	//find usable pattern textures from the model directory
	if (patternsUsed) {
		FindPatterns(model->m_patterns);
		//set up some noticeable default colors
		std::vector<Color4ub> colors;
		colors.push_back(Color4ub(255, 0, 0, 0));
		colors.push_back(Color4ub(0, 255, 0, 0));
		colors.push_back(Color4ub(0, 0, 255, 0));
		model->SetColors(m_renderer, colors);
		model->SetPattern(0);
	}
	return model;
}

void Loader::FindPatterns(PatternContainer &output)
{
	for (FileSystem::FileEnumerator files(FileSystem::gameDataFiles, m_curPath); !files.Finished(); files.Next()) {
		const FileSystem::FileInfo &info = files.Current();
		if (info.IsFile()) {
			const std::string &name = info.GetName();
			if (ends_with(name, ".png")) { //correct type?
				if(name.compare(0, 7, "pattern") == 0) { //acceptable name?
					//load as a pattern
					const std::string &patternPath = FileSystem::JoinPathBelow(m_curPath, name);
					Graphics::Texture *pat = Graphics::TextureBuilder::Model(patternPath).CreateTexture(m_renderer);
					output.push_back(Pattern(name, pat));
				}
			}
			//std::cout << files.Current().GetName() << std::endl;
		}
	}
	//std::cout << m_curPath << std::endl;

}

Node *Loader::LoadMesh(const std::string &filename, NModel *model, const AnimList &animDefs, TagList &modelTags)
{
	m_model = model;

	Assimp::Importer importer;

	//Removing components is suggested to optimize loading. We do not care about vtx colors now.
	importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_COLORS);

	//assimp needs the data dir too...
	//XXX check user dir first
	//XXX x2 the greater goal is not to use ReadFile but the other assimp data read functions + FileSystem. See assimp docs.
	const aiScene *scene = importer.ReadFile(
		FileSystem::JoinPath(FileSystem::GetDataDir(), filename),
		aiProcess_RemoveComponent |
		aiProcess_Triangulate	  |
		aiProcess_SortByPType	  | //ignore point, line primitive types (collada dummy nodes seem to be fine)
		aiProcess_GenUVCoords	  | //only if they don't exist
		aiProcess_FlipUVs		  |
		aiProcess_GenSmoothNormals); //only if normals not specified

	if(!scene)
		throw std::string("Couldn't load " + filename);

	if(scene->mNumMeshes == 0)
		throw std::string(filename + " has no geometry. How odd!");

	//turn all scene aiMeshes into Surfaces
	//Index matches assimp index.
	std::vector<Graphics::Surface*> surfaces;
	ConvertAiMeshesToSurfaces(surfaces, scene, model);

	Node *node = 0; //XXX don't return a node, pass in the root (of the current LOD, not model root)

	//XXX importing everything through the ConvertNodes scheme
#if 0
	if (false && !scene->HasAnimations()) {
		//XXX putting everything in one static mesh
		//XXX the plan: if scene has animation, go through the assimp node structure and
		//create the appropriate nodes (staticgeometry parented to matrixtransforms for animated nodes)
		StaticGeometry *geom = new StaticGeometry();
		Graphics::StaticMesh *smesh = geom->GetMesh();
		node = geom;

		//the entire mesh might be translated, the usual case is 90 deg X rotation
		MatrixTransform *trans = 0;
		if (!scene->mRootNode->mTransformation.IsIdentity()) {
			matrix4x4f m = ConvertMatrix(scene->mRootNode->mTransformation);
			trans = new MatrixTransform(m);
			trans->AddChild(geom);
			node = trans;
		}

		//update bounding box
		for (unsigned int i=0; i<surfaces.size(); i++) {
			Graphics::Surface *surf = surfaces[i];
			Graphics::VertexArray *vts = surf->GetVertices();
			for (unsigned int j=0; j<vts->position.size(); j++) {
				const vector3f &vtx = vts->position[j];
				geom->m_boundingBox.Update(vtx.x, vtx.y, vtx.z);
			}
			smesh->AddSurface(surf);
		}
	}
	else
#endif
	{
		Group *group = new Group;
		ConvertNodes(scene->mRootNode, group, surfaces, matrix4x4f::Identity());
		node = group;
	}

	m_root = node;

	ConvertAnimations(scene, animDefs, model);

	return node;
}

//check animation channel has at least two position or two rotation keys within time range
bool Loader::CheckKeysInRange(const aiNodeAnim *chan, double start, double end)
{
	int posKeysInRange = 0;
	int rotKeysInRange = 0;

	for(unsigned int k=0; k<chan->mNumPositionKeys; k++) {
		const aiVectorKey &aikey = chan->mPositionKeys[k];
		if (aikey.mTime >= start && aikey.mTime <= end) posKeysInRange++;
	}

	for(unsigned int k=0; k<chan->mNumRotationKeys; k++) {
		const aiQuatKey &aikey = chan->mRotationKeys[k];
		if (aikey.mTime >= start && aikey.mTime <= end) rotKeysInRange++;
	}

	return (posKeysInRange > 1 || rotKeysInRange > 1);
}

RefCountedPtr<Graphics::Material> Loader::GetDecalMaterial(unsigned int index)
{
	assert(index < NModel::MAX_DECAL_MATERIALS);
	RefCountedPtr<Graphics::Material> &decMat = m_model->m_decalMaterials[index-1];
	if (!decMat.Valid()) {
		Graphics::MaterialDescriptor matDesc;
		matDesc.textures = 1;
		decMat.Reset(m_renderer->CreateMaterial(matDesc));
	}
	return decMat;
}

void Loader::ConvertAiMeshesToSurfaces(std::vector<Graphics::Surface*> &surfaces, const aiScene *scene, NModel *model)
{
	//XXX sigh, workaround for obj loader
	int matIdxOffs = 0;
	if (scene->mNumMaterials > scene->mNumMeshes)
		matIdxOffs = 1;

	//turn meshes into surfaces
	for (unsigned int i=0; i<scene->mNumMeshes; i++) {
		aiMesh *mesh = scene->mMeshes[i];
		assert(mesh->HasNormals());
		if (!mesh->HasTextureCoords(0))
			throw std::string("Mesh has no uv coordinates");

		//Material names are not consistent throughout formats...
		//try to figure out a material
		//try name first, if that fails use index
		RefCountedPtr<Graphics::Material> mat;
		const aiMaterial *amat = scene->mMaterials[mesh->mMaterialIndex];
		aiString s;
		if(AI_SUCCESS == amat->Get(AI_MATKEY_NAME,s)) {
			//std::cout << "Looking for " << std::string(s.data,s.length) << std::endl;
			const std::string aiMatName = std::string(s.data, s.length);
			mat = model->GetMaterialByName(aiMatName);
		}

		if (!mat.Valid()) {
			mat = model->GetMaterialByIndex(mesh->mMaterialIndex - matIdxOffs);
		}

		assert(mat.Valid());

		Graphics::VertexArray *vts =
			new Graphics::VertexArray(
				Graphics::ATTRIB_POSITION |
				Graphics::ATTRIB_NORMAL |
				Graphics::ATTRIB_UV0);

		Graphics::Surface *surface = new Graphics::Surface(Graphics::TRIANGLES, vts, mat);
		std::vector<unsigned short> &indices = surface->GetIndices();

		//copy indices first
		//note: index offsets are not adjusted, StaticMesh should do that for us
		for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
			const aiFace *face = &mesh->mFaces[f];
			for (unsigned int j = 0; j < face->mNumIndices; j++) {
				indices.push_back(face->mIndices[j]);
			}
		}

		//then vertices, making gross assumptions of the format
		for (unsigned int v = 0; v < mesh->mNumVertices; v++) {
			const aiVector3D &vtx = mesh->mVertices[v];
			const aiVector3D &norm = mesh->mNormals[v];
			const aiVector3D &uv0 = mesh->mTextureCoords[0][v];
			vts->Add(vector3f(vtx.x, vtx.y, vtx.z),
				vector3f(norm.x, norm.y, norm.z),
				vector2f(uv0.x, uv0.y));
		}

		surfaces.push_back(surface);
	}
}

void Loader::ConvertAnimations(const aiScene* scene, const AnimList &animDefs, NModel *model)
{
	if (animDefs.empty() || scene->mNumAnimations == 0) return;

	if (scene->mNumAnimations > 1) throw std::string("More than one animation in file! Your exporter is too good");

	//Blender .X exporter exports only one animation (without a name!) so
	//we read only one animation from the scene and split it according to animDefs
	std::vector<Animation*> &animations = model->m_animations;

	const aiAnimation* aianim = scene->mAnimations[0];
	for (AnimList::const_iterator def = animDefs.begin();
		def != animDefs.end();
		++def)
	{
		//XXX for assimp duration of frames 0 to 39 seems to be 39. Odd.
		Animation *animation = new Animation(
			def->name, def->end - def->start,
			def->loop ? Animation::LOOP : Animation::ONCE,
			aianim->mTicksPerSecond > 0.0 ? aianim->mTicksPerSecond : 24.0);
		for (unsigned int j=0; j<aianim->mNumChannels; j++) {
			const aiNodeAnim *aichan = aianim->mChannels[j];
			if (!CheckKeysInRange(aichan, def->start, def->end))
				continue;

			const std::string channame(aichan->mNodeName.C_Str());
			MatrixTransform *trans = dynamic_cast<MatrixTransform*>(m_root->FindNode(channame));
			assert(trans);
			animation->channels.push_back(AnimationChannel(trans));
			AnimationChannel &chan = animation->channels.back();

			for(unsigned int k=0; k<aichan->mNumPositionKeys; k++) {
				const aiVectorKey &aikey = aichan->mPositionKeys[k];
				const aiVector3D &aipos = aikey.mValue;
				if (aikey.mTime >= def->start && aikey.mTime <= def->end)
					chan.positionKeys.push_back(PositionKey(aikey.mTime - def->start, vector3f(aipos.x, aipos.y, aipos.z)));
			}

			if (aichan->mNumRotationKeys < 2) continue;
			for(unsigned int k=0; k<aichan->mNumRotationKeys; k++) {
				const aiQuatKey &aikey = aichan->mRotationKeys[k];
				const aiQuaternion &airot = aikey.mValue;
				if (aikey.mTime >= def->start && aikey.mTime <= def->end)
					chan.rotationKeys.push_back(RotationKey(aikey.mTime - def->start, Quaternionf(airot.w, airot.x, airot.y, airot.z)));
			}
		}

		if (animation->channels.empty())
			delete animation;
		else
			animations.push_back(animation);
	}
#if 0
	for (unsigned int i=0; i<scene->mNumAnimations; i++) {
		const aiAnimation* aianim = scene->mAnimations[i];
		const std::string animname(aianim->mName.C_Str());
		Animation *animation = new Animation(animname.empty() ? "wiggle" : animname, aianim->mDuration, Animation::LOOP);
		for (unsigned int j=0; j<aianim->mNumChannels; j++) {
			aiNodeAnim *aichan = aianim->mChannels[j];
			const std::string channame(aichan->mNodeName.C_Str());
			MatrixTransform *trans = dynamic_cast<MatrixTransform*>(m_root->FindNode(channame));
			assert(trans);
			animation->channels.push_back(AnimationChannel(trans));
			AnimationChannel &chan = animation->channels.back();

			for(unsigned int k=0; k<aichan->mNumPositionKeys; k++) {
				const aiVectorKey &aikey = aichan->mPositionKeys[k];
				const aiVector3D &aipos = aikey.mValue;
				chan.positionKeys.push_back(PositionKey(aikey.mTime, vector3f(aipos.x, aipos.y, aipos.z)));
			}

			if (aichan->mNumRotationKeys < 2) continue;
			for(unsigned int k=0; k<aichan->mNumRotationKeys; k++) {
				const aiQuatKey &aikey = aichan->mRotationKeys[k];
				const aiQuaternion &airot = aikey.mValue;
				chan.rotationKeys.push_back(RotationKey(aikey.mTime, Quaternionf(airot.w, airot.x, airot.y, airot.z)));
			}
		}
		animations.push_back(animation);
	}
#endif
}

matrix4x4f Loader::ConvertMatrix(const aiMatrix4x4& trans) const
{
	matrix4x4f m;
	m[0] = trans.a1;
	m[1] = trans.b1;
	m[2] = trans.c1;
	m[3] = trans.d1;

	m[4] = trans.a2;
	m[5] = trans.b2;
	m[6] = trans.c2;
	m[7] = trans.d2;

	m[8] = trans.a3;
	m[9] = trans.b3;
	m[10] = trans.c3;
	m[11] = trans.d3;

	m[12] = trans.a4;
	m[13] = trans.b4;
	m[14] = trans.c4;
	m[15] = trans.d4;
	return m;
}

//XXX temporary junk?
//yeah, definitely
static void create_light(Group* parent, const matrix4x4f &m, Graphics::Renderer *m_renderer)
{
		std::vector<vector3f> points;
		points.push_back(m.GetTranslate());
		Graphics::MaterialDescriptor desc;
		desc.twoSided = true;
		desc.textures = 1;
		RefCountedPtr<Graphics::Material> mat(m_renderer->CreateMaterial(desc));
		mat->texture0 = Graphics::TextureBuilder::Billboard("textures/halo.png").GetOrCreateTexture(m_renderer, "billboard");
		mat->diffuse = Color(1.f, 0.f, 0.f, 1.f);
		Billboard *bill = new Billboard(points, mat, 1.f);
		parent->AddChild(bill);
}

static void create_thruster(Group* parent, const matrix4x4f &m, Graphics::Renderer *m_renderer, const matrix4x4f& accum, bool linear)
{
	//not supposed to create a new thruster node every time since they contain their geometry
	//it is fine to create one thruster node and add that to various parents
	//(it wouldn't really matter, it's a tiny amount of geometry)

	MatrixTransform *trans = new MatrixTransform(m);

	//need the accumulated transform or the direction is off
	const matrix4x4f transform = m * accum;
	// XXX YZ swap. To be investigated...
	Thruster *thruster = new Thruster(m_renderer, linear,
		vector3f(transform[12], transform[14], transform[13]), transform.Back());
	trans->AddChild(thruster);
	parent->AddChild(trans);
}

void Loader::CreateLabel(Group *parent, const matrix4x4f &m)
{
	MatrixTransform *trans = new MatrixTransform(m);
	Label3D *label = new Label3D(m_labelFont, m_renderer);
	label->SetText("Boners");
	trans->AddChild(label);
	parent->AddChild(trans);
}

void Loader::ConvertNodes(aiNode *node, Group *_parent, std::vector<Graphics::Surface*>& surfaces, const matrix4x4f &accum)
{
	Group *parent = _parent;
	const std::string nodename(node->mName.C_Str());
	const aiMatrix4x4& trans = node->mTransformation;
	matrix4x4f m = ConvertMatrix(trans);

	//lights, and possibly other special nodes should be leaf nodes (without meshes)
	if (node->mNumChildren == 0 && node->mNumMeshes == 0) {
		if (starts_with(nodename, "navlight_")) {
			create_light(parent, m, m_renderer);
		} else if (starts_with(nodename, "thruster_")) {
			create_thruster(parent, m, m_renderer, accum, starts_with(nodename, "thruster_linear"));
		} else if (starts_with(nodename, "label_")) {
			CreateLabel(parent, m);
		} else if (starts_with(nodename, "tag_")) {
			vector3f tagpos = accum * m.GetTranslate();
			MatrixTransform *tagMt = new MatrixTransform(matrix4x4f::Translation(tagpos));
			m_model->AddTag(nodename, tagMt);
		}
		return;
	}

	//if the transform is identity and the node is not animated,
	//could just add a group
	parent = new MatrixTransform(m);
	_parent->AddChild(parent);
	parent->SetName(nodename);

	if (node->mNumMeshes > 0) {
		//is this node animated? add a transform
		//does this node have children? Add a group
		StaticGeometry *geom = new StaticGeometry();
		Graphics::StaticMesh *smesh = geom->GetMesh();
		unsigned int numDecal = 0;
		if (starts_with(nodename, "decal_")) {
			if (nodename.compare(7,1, "1") == 0)
				numDecal = 1;
			else if (nodename.compare(7,1, "2") == 0)
				numDecal = 2;
			else if (nodename.compare(7,1, "3") == 0)
				numDecal = 3;
			else if (nodename.compare(7,1, "4") == 0)
				numDecal = 4;
			else
				throw std::string("More than 4 different decals");
		}
		if (numDecal != 0) {//XXX could add a simple DecalGeometry node
			geom->SetNodeMask(NODE_TRANSPARENT);
			geom->m_blendMode = Graphics::BLEND_ALPHA;
		}

		for(unsigned int i=0; i<node->mNumMeshes; i++) {
			Graphics::Surface *surf = surfaces[node->mMeshes[i]];
			if (numDecal > 0) {
				surf->SetMaterial(GetDecalMaterial(numDecal));
			}
			//update bounding box
			Graphics::VertexArray *vts = surf->GetVertices();
			for (unsigned int j=0; j<vts->position.size(); j++) {
				const vector3f &vtx = m * vts->position[j];
				geom->m_boundingBox.Update(vtx.x, vtx.y, vtx.z);
			}
			smesh->AddSurface(surf);
		}

		parent->AddChild(geom);
	}

	for(unsigned int i=0; i<node->mNumChildren; i++) {
		aiNode *child = node->mChildren[i];
		ConvertNodes(child, parent, surfaces, m * accum);
	}
}

void Loader::LoadCollision(const std::string &filename)
{
	//Convert all found aiMeshes into a geomtree. Materials,
	//Animations and node structure can be ignored
	assert(m_model);

	if (!m_model->m_collMesh.Valid())
		m_model->m_collMesh.Reset(new CollMesh());

	Assimp::Importer importer;

	//discard extra data
	importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS,
		aiComponent_COLORS    |
		aiComponent_TEXCOORDS |
		aiComponent_NORMALS   |
		aiComponent_MATERIALS
		);
	const aiScene *scene = importer.ReadFile(
		FileSystem::JoinPath(FileSystem::GetDataDir(), filename),
		aiProcess_RemoveComponent |
		aiProcess_Triangulate     |
		aiProcess_PreTransformVertices //"bake" transformations so we can disregard the structure
		);

	if(!scene)
		throw std::string("Couldn't load " + filename);

	if(scene->mNumMeshes == 0)
		throw std::string(filename + " has no geometry");

	//note geomtree keeps a pointer to the arrays but doesn't own them
	//geomtree does not use vector3, so watch out
	std::vector<int> &indices = m_model->m_collMesh->m_indices;
	std::vector<vector3f> &vertices = m_model->m_collMesh->m_vertices;
	std::vector<unsigned int> &triFlags = m_model->m_collMesh->m_flags;
	unsigned int indexOffset = 0;

	for(unsigned int i=0; i<scene->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[i];

		//copy indices
		//we assume aiProcess_Triangulate does its job
		for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
			const aiFace *face = &mesh->mFaces[f];
			for (unsigned int j = 0; j < face->mNumIndices; j++) {
				indices.push_back(indexOffset + face->mIndices[j]);
			}
			//add some default collision flags. We do not care about this much now.
			triFlags.push_back(0);
		}
		indexOffset += mesh->mNumFaces*3;

		//vertices
		for (unsigned int v = 0; v < mesh->mNumVertices; v++) {
			const aiVector3D &vtx = mesh->mVertices[v];
			vertices.push_back(vector3f(vtx.x, vtx.y, vtx.z));
		}
	}

	GeomTree *tree = new GeomTree(
		vertices.size(),
		indices.size()/3,
		&(m_model->m_collMesh->m_vertices[0].x),
		&(m_model->m_collMesh->m_indices[0]),
		&(m_model->m_collMesh->m_flags[0])
	);
	m_model->m_collMesh->SetGeomTree(tree);
	m_model->m_boundingRadius = m_model->m_collMesh->GetGeomTree()->GetAabb().GetBoundingRadius();
}

}
