// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Loader.h"
#include "CollisionGeometry.h"
#include "FileSystem.h"
#include "LOD.h"
#include "Parser.h"
#include "SceneGraph.h"
#include "StaticGeometry.h"
#include "StringF.h"
#include "utils.h"
#include "graphics/Renderer.h"
#include "graphics/Surface.h"
#include "graphics/TextureBuilder.h"
#include "FileSystem.h"
#include <assimp/Importer.hpp>
#include <assimp/IOStream.hpp>
#include <assimp/IOSystem.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/material.h>

namespace {

	class AssimpFileReadStream : public Assimp::IOStream
	{
	public:
		explicit AssimpFileReadStream(const RefCountedPtr<FileSystem::FileData>& data): m_data(data)
		{
			m_cursor = m_data->GetData();
		}

		virtual ~AssimpFileReadStream() {}

		virtual size_t FileSize() const { return m_data->GetSize(); }

		virtual size_t Read(void *buf, size_t size, size_t count)
		{
			const char * const data_end = m_data->GetData() + m_data->GetSize();
			const size_t remaining = (data_end - m_cursor);
			const size_t requested = size * count;
			const size_t len = std::min(remaining, requested);
			memcpy(static_cast<char*>(buf), m_cursor, len);
			m_cursor += len;
			return len;
		}

		virtual aiReturn Seek(size_t offset, aiOrigin origin)
		{
			switch (origin) {
				case aiOrigin_SET: break;
				case aiOrigin_CUR: offset += Tell(); break;
				case aiOrigin_END: offset += m_data->GetSize(); break;
				default: assert(0); break;
			}
			if (offset < 0 || offset > m_data->GetSize())
				return aiReturn_FAILURE;
			m_cursor = m_data->GetData() + offset;
			return aiReturn_SUCCESS;
		}

		virtual size_t Tell() const
		{
			return size_t(m_cursor - m_data->GetData());
		}

		virtual size_t Write(const void *buf, size_t size, size_t count) __attribute((noreturn))
		{
			assert(0);
			abort();
			RETURN_ZERO_NONGNU_ONLY;
		}

		virtual void Flush()
		{
			assert(0);
			abort();
		}

	private:
		RefCountedPtr<FileSystem::FileData> m_data;
		const char *m_cursor;
	};

	class AssimpFileSystem : public Assimp::IOSystem
	{
	public:
		AssimpFileSystem(FileSystem::FileSource& fs): m_fs(fs) {}
		virtual ~AssimpFileSystem() {}

		virtual bool Exists(const char *path) const
		{
			const FileSystem::FileInfo info = m_fs.Lookup(path);
			return info.Exists();
		}

		virtual char getOsSeparator() const { return '/'; }

		virtual Assimp::IOStream *Open(const char *path, const char *mode)
		{
			assert(mode[0] == 'r');
			assert(!strchr(mode, '+'));
			RefCountedPtr<FileSystem::FileData> data = m_fs.ReadFile(path);
			return (data ? new AssimpFileReadStream(data) : 0);
		}

		virtual void Close(Assimp::IOStream *file)
		{
			delete file;
		}

	private:
		FileSystem::FileSource &m_fs;
	};

} // anonymous namespace

namespace SceneGraph {

Loader::Loader(Graphics::Renderer *r) :
	m_renderer(r),
	m_model(0)
{
	Graphics::Texture *sdfTex = Graphics::TextureBuilder("fonts/label3d.png", Graphics::LINEAR_CLAMP, true, true, true).GetOrCreateTexture(r, "model");
	m_labelFont.Reset(new Text::DistanceFieldFont("fonts/sdf_definition.txt", sdfTex));
}

Loader::~Loader()
{
}

Model *Loader::LoadModel(const std::string &filename)
{
	Model *m = LoadModel(filename, "models");
	return m;
}

Model *Loader::LoadModel(const std::string &shortname, const std::string &basepath)
{
	FileSystem::FileSource &fileSource = FileSystem::gameDataFiles;
	for (FileSystem::FileEnumerator files(fileSource, basepath, FileSystem::FileEnumerator::Recurse); !files.Finished(); files.Next())
	{
		const FileSystem::FileInfo &info = files.Current();
		const std::string &fpath = info.GetPath();

		//check it's the expected type
		if (info.IsFile() && ends_with(fpath, ".model")) {
			//check it's the wanted name & load it
			const std::string name = info.GetName();

			if (shortname == name.substr(0, name.length()-6)) {
				ModelDefinition modelDefinition;
				try {
					//curPath is used to find textures, patterns,
					//possibly other data files for this model.
					//Strip trailing slash
					m_curPath = info.GetDir();
					assert(!m_curPath.empty());
					if (m_curPath[m_curPath.length()-1] == '/')
						m_curPath = m_curPath.substr(0, m_curPath.length()-1);

					Parser p(fileSource, fpath, m_curPath);
					p.Parse(&modelDefinition);
				} catch (ParseError &err) {
					fprintf(stderr, "%s\n", err.what());
					throw LoadingError(err.what());
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

Model *Loader::CreateModel(ModelDefinition &def)
{
	using Graphics::Material;
	if (def.matDefs.empty()) return 0;
	if (def.lodDefs.empty()) return 0;

	Model *model = new Model(def.name);
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
		matDesc.lighting = !it->unlit;
		matDesc.alphaTest = it->alpha_test;
		matDesc.twoSided = it->two_sided;

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

		//semitransparent material
		//the node must be marked transparent when using this material
		//and should not be mixed with opaque materials
		if ((*it).opacity < 100)
			mat->diffuse.a = float((*it).opacity) / 100.f;

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

		model->m_materials.push_back(std::make_pair((*it).name, mat));
	}
	//printf("Loaded %d materials\n", int(model->m_materials.size()));

	//load meshes
	//"mesh" here refers to a "mesh xxx.yyy"
	//defined in the .model
	std::map<std::string, RefCountedPtr<Node> > meshCache;
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
				RefCountedPtr<Node> mesh;
				std::map<std::string, RefCountedPtr<Node> >::iterator cacheIt = meshCache.find((*it));
				if (cacheIt != meshCache.end())
					mesh = (*cacheIt).second;
				else {
					try {
						mesh = LoadMesh(*it, def.animDefs, def.tagDefs);
					} catch (LoadingError &err) {
						//append filename - easiest to do here
						throw (LoadingError(stringf("%0:\n%1", *it, err.what())));
					}
					meshCache[*(it)] = mesh;
				}
				assert(mesh.Valid());

				if (group)
					group->AddChild(mesh.Get());
				else if(lodNode) {
					lodNode->AddLevel((*lod).pixelSize, mesh.Get());
				} else
					model->GetRoot()->AddChild(mesh.Get());
			} catch (LoadingError &err) {
				delete model;
				fprintf(stderr, "%s\n", err.what());
				throw;
			}
		}
	}

	// Load collision meshes
	// They are added at the top level of the model root as CollisionGeometry nodes
	for (std::vector<std::string>::const_iterator it = def.collisionDefs.begin();
		it != def.collisionDefs.end(); ++it)
	{
		try {
			LoadCollision(*it);
		} catch (LoadingError &err) {
			throw (LoadingError(stringf("%0:\n%1", *it, err.what())));
		}
	}

	// Run CollisionVisitor to create the initial CM and its GeomTree.
	// If no collision mesh is defined, a simple bounding box will be generated
	m_model->CreateCollisionMesh(0);

	// Add tag points
	// XXX defining tags in .model not implemented
	// the question is if everything can be expected to use collada/etc
	for(TagList::const_iterator it = def.tagDefs.begin();
		it != def.tagDefs.end();
		++it)
	{
		const vector3f &pos = (*it).position;
		RefCountedPtr<MatrixTransform> tagTrans(new MatrixTransform(matrix4x4f::Translation(pos.x, pos.y, pos.z)));
		model->AddTag((*it).name, tagTrans.Get());
	}

	//find usable pattern textures from the model directory
	if (patternsUsed) {
		FindPatterns(model->m_patterns);

		if (model->m_patterns.empty()) {
			model->m_patterns.push_back(Pattern());
			Pattern &dumpat = m_model->m_patterns.back();
			dumpat.name = "Dummy";
			dumpat.texture = RefCountedPtr<Graphics::Texture>(GetWhiteTexture());
		}

		//set up some noticeable default colors
		std::vector<Color4ub> colors;
		colors.push_back(Color4ub::RED);
		colors.push_back(Color4ub::GREEN);
		colors.push_back(Color4ub::BLUE);
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
			if (ends_with(name, ".png") && starts_with(name, "pattern"))
				output.push_back(Pattern(name, m_curPath, m_renderer));
		}
	}
}

RefCountedPtr<Node> Loader::LoadMesh(const std::string &filename, const AnimList &animDefs, TagList &modelTags)
{
	Assimp::Importer importer;
	importer.SetIOHandler(new AssimpFileSystem(FileSystem::gameDataFiles));

	//Removing components is suggested to optimize loading. We do not care about vtx colors now.
	importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_COLORS);
	importer.SetPropertyInteger(AI_CONFIG_PP_SLM_VERTEX_LIMIT, Graphics::StaticMesh::MAX_VERTICES);

	//There are several optimizations assimp can do, intentionally skipping them now
	const aiScene *scene = importer.ReadFile(
		filename,
		aiProcess_RemoveComponent	|
		aiProcess_Triangulate		|
		aiProcess_SortByPType		| //ignore point, line primitive types (collada dummy nodes seem to be fine)
		aiProcess_GenUVCoords		| //only if they don't exist
		aiProcess_FlipUVs			|
		aiProcess_SplitLargeMeshes	|
		aiProcess_GenSmoothNormals);  //only if normals not specified

	if(!scene)
		throw LoadingError("Couldn't load file");

	if(scene->mNumMeshes == 0)
		throw LoadingError("No geometry found");

	//turn all scene aiMeshes into Surfaces
	//Index matches assimp index.
	std::vector<RefCountedPtr<Graphics::Surface> > surfaces;
	ConvertAiMeshesToSurfaces(surfaces, scene, m_model);

	// Recursive structure conversion. Matrix needs to be accumulated for
	// special features that are absolute-positioned (thrusters)
	RefCountedPtr<Node> meshRoot(new Group());

	ConvertNodes(scene->mRootNode, static_cast<Group*>(meshRoot.Get()), surfaces, matrix4x4f::Identity());
	ConvertAnimations(scene, animDefs, static_cast<Group*>(meshRoot.Get()));

	return meshRoot;
}

static bool in_range(double keytime, double start, double end)
{
	return (keytime >= start - 0.001 && keytime - 0.001 <= end);
}

//check animation channel has at least two P, R or S keys within time range
bool Loader::CheckKeysInRange(const aiNodeAnim *chan, double start, double end)
{
	int posKeysInRange = 0;
	int rotKeysInRange = 0;
	int sclKeysInRange = 0;

	for (unsigned int k=0; k<chan->mNumPositionKeys; k++) {
		const aiVectorKey &aikey = chan->mPositionKeys[k];
		if (in_range(aikey.mTime, start, end)) posKeysInRange++;
	}

	for (unsigned int k=0; k<chan->mNumRotationKeys; k++) {
		const aiQuatKey &aikey = chan->mRotationKeys[k];
		if (in_range(aikey.mTime, start, end)) rotKeysInRange++;
	}

	for (unsigned int k=0; k<chan->mNumScalingKeys; k++) {
		const aiVectorKey &aikey = chan->mScalingKeys[k];
		if (in_range(aikey.mTime, start, end)) sclKeysInRange++;
	}

	return (posKeysInRange > 1 || rotKeysInRange > 1 || sclKeysInRange > 1);
}

RefCountedPtr<Graphics::Material> Loader::GetDecalMaterial(unsigned int index)
{
	assert(index <= Model::MAX_DECAL_MATERIALS);
	RefCountedPtr<Graphics::Material> &decMat = m_model->m_decalMaterials[index-1];
	if (!decMat.Valid()) {
		Graphics::MaterialDescriptor matDesc;
		matDesc.textures = 1;
		matDesc.lighting = true;
		decMat.Reset(m_renderer->CreateMaterial(matDesc));
		decMat->specular = Color::BLACK;
		decMat->diffuse = Color::WHITE;
	}
	return decMat;
}

void Loader::CheckAnimationConflicts(const Animation* anim, const std::vector<Animation*> &otherAnims)
{
	typedef std::vector<AnimationChannel>::const_iterator ChannelIterator;
	typedef std::vector<Animation*>::const_iterator AnimIterator;

	if (anim->m_channels.empty() || otherAnims.empty()) return;

	//check all other animations that they don't control the same nodes as this animation, since
	//that is not supported at this point
	for (ChannelIterator chan = anim->m_channels.begin(); chan != anim->m_channels.end(); ++chan) {
		for (AnimIterator other = otherAnims.begin(); other != otherAnims.end(); ++other) {
			const Animation *otherAnim = (*other);
			assert(otherAnim != anim);
			for (ChannelIterator otherChan = otherAnim->m_channels.begin(); otherChan != otherAnim->m_channels.end(); ++otherChan) {
				//warnings as errors mentality - this is not really fatal
				if (chan->node == otherChan->node)
					throw LoadingError(stringf("Animations %0 and %1 both control node: %2", anim->GetName(), otherAnim->GetName(), chan->node->GetName()));
			}
		}
	}
}

void Loader::ConvertAiMeshesToSurfaces(std::vector<RefCountedPtr<Graphics::Surface> > &surfaces, const aiScene *scene, Model *model)
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
			throw LoadingError("Missing UV coordinates");

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

		RefCountedPtr<Graphics::Surface> surface(new Graphics::Surface(Graphics::TRIANGLES, vts, mat));
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

void Loader::ConvertAnimations(const aiScene* scene, const AnimList &animDefs, Node *meshRoot)
{
	//Split convert assimp animations according to anim defs
	//This is very limited, and all animdefs are processed for all
	//meshes, potentially leading to duplicate and wrongly split animations
	if (animDefs.empty() || scene->mNumAnimations == 0) return;
	if (scene->mNumAnimations > 1) printf("File has %d animations, treating as one animation\n", scene->mNumAnimations);

	std::vector<Animation*> &animations = m_model->m_animations;

	for (AnimList::const_iterator def = animDefs.begin();
		def != animDefs.end();
		++def)
	{
		//XXX format differences: for a 40-frame animation exported from Blender,
		//.X results in duration 39 and Collada in Duration 1.25.
		//duration is calculated after adding all keys
		//take TPS from the first animation
		const aiAnimation* firstAnim = scene->mAnimations[0];
		const double ticksPerSecond = firstAnim->mTicksPerSecond > 0.0 ? firstAnim->mTicksPerSecond : 24.0;
		double start = DBL_MAX;
		double end = 0.0;

		//Ranges are specified in frames (since that's nice) but Collada
		//uses seconds. This is easiest to detect from ticksPerSecond,
		//but assuming 24 FPS here
		//Could make FPS an additional define or always require 24
		double defStart = def->start;
		double defEnd = def->end;
		if (is_equal_exact(ticksPerSecond, 1.0)) {
			defStart /= 24.0;
			defEnd /= 24.0;
		}
		Animation *animation = new Animation(
			def->name, 0.0,
			def->loop ? Animation::LOOP : Animation::ONCE,
			ticksPerSecond);

		for (unsigned int i=0; i < scene->mNumAnimations; i++) {
			const aiAnimation* aianim = scene->mAnimations[i];
			for (unsigned int j=0; j<aianim->mNumChannels; j++) {
				const aiNodeAnim *aichan = aianim->mChannels[j];
				//do a preliminary check that at least two keys in one channel are within range
				if (!CheckKeysInRange(aichan, defStart, defEnd))
					continue;

				const std::string channame(aichan->mNodeName.C_Str());
				MatrixTransform *trans = dynamic_cast<MatrixTransform*>(meshRoot->FindNode(channame));
				assert(trans);
				animation->m_channels.push_back(AnimationChannel(trans));
				AnimationChannel &chan = animation->m_channels.back();

				for(unsigned int k=0; k<aichan->mNumPositionKeys; k++) {
					const aiVectorKey &aikey = aichan->mPositionKeys[k];
					const aiVector3D &aipos = aikey.mValue;
					if (in_range(aikey.mTime, defStart, defEnd)) {
						chan.positionKeys.push_back(PositionKey(aikey.mTime - defStart, vector3f(aipos.x, aipos.y, aipos.z)));
						start = std::min(start, aikey.mTime);
						end = std::max(end, aikey.mTime);
					}
				}

				//scale interpolation will blow up without rotation keys,
				//so skipping them when rotkeys < 2 is correct
				if (aichan->mNumRotationKeys < 2) continue;

				for(unsigned int k=0; k<aichan->mNumRotationKeys; k++) {
					const aiQuatKey &aikey = aichan->mRotationKeys[k];
					const aiQuaternion &airot = aikey.mValue;
					if (in_range(aikey.mTime, defStart, defEnd)) {
						chan.rotationKeys.push_back(RotationKey(aikey.mTime - defStart, Quaternionf(airot.w, airot.x, airot.y, airot.z)));
						start = std::min(start, aikey.mTime);
						end = std::max(end, aikey.mTime);
					}
				}

				for(unsigned int k=0; k<aichan->mNumScalingKeys; k++) {
					const aiVectorKey &aikey = aichan->mScalingKeys[k];
					const aiVector3D &aipos = aikey.mValue;
					if (in_range(aikey.mTime, defStart, defEnd)) {
						chan.scaleKeys.push_back(ScaleKey(aikey.mTime - defStart, vector3f(aipos.x, aipos.y, aipos.z)));
						start = std::min(start, aikey.mTime);
						end = std::max(end, aikey.mTime);
					}
				}
			}
		}

		//set actual duration
		animation->m_duration = end - start;

		//do final sanity checking before adding
		if (animation->m_channels.empty()) {
			delete animation;
		} else {
			try {
				CheckAnimationConflicts(animation, animations);
			} catch (LoadingError &) {
				delete animation;
				throw;
			}
			animations.push_back(animation);
		}
	}
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

void Loader::CreateLabel(Group *parent, const matrix4x4f &m)
{
	MatrixTransform *trans = new MatrixTransform(m);
	Label3D *label = new Label3D(m_labelFont, m_renderer);
	label->SetText("Bananas");
	trans->AddChild(label);
	parent->AddChild(trans);
}

void Loader::CreateLight(Group *parent, const matrix4x4f &m)
{
	//One node per light, obviously not as optimal as intended
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

void Loader::CreateThruster(Group* parent, const matrix4x4f &m, const std::string &name, const matrix4x4f& accum)
{
	const bool linear = starts_with(name, "thruster_linear");
	//not supposed to create a new thruster node every time since they contain their geometry
	//it is fine to create one thruster node and add that to various parents
	//(it wouldn't really matter, it's a tiny amount of geometry)
	MatrixTransform *trans = new MatrixTransform(m);

	//need the accumulated transform or the direction is off
	matrix4x4f transform = accum * m;
	vector3f pos = transform.GetTranslate();
	transform.ClearToRotOnly();

	vector3f direction = transform * vector3f(0.f, 0.f, 1.f);

	Thruster *thruster = new Thruster(m_renderer, linear,
		pos, direction.Normalized());

	thruster->SetName(name);
	trans->AddChild(thruster);
	parent->AddChild(trans);
}

void Loader::ConvertNodes(aiNode *node, Group *_parent, std::vector<RefCountedPtr<Graphics::Surface> >& surfaces, const matrix4x4f &accum)
{
	Group *parent = _parent;
	const std::string nodename(node->mName.C_Str());
	const aiMatrix4x4& trans = node->mTransformation;
	matrix4x4f m = ConvertMatrix(trans);

	//lights, and possibly other special nodes should be leaf nodes (without meshes)
	if (node->mNumChildren == 0 && node->mNumMeshes == 0) {
		if (starts_with(nodename, "navlight_")) {
			CreateLight(parent, m);
		} else if (starts_with(nodename, "thruster_")) {
			CreateThruster(parent, m, nodename, accum);
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

	//nodes named collision_* are not added as renderable geometry
	if (node->mNumMeshes == 1 && starts_with(nodename, "collision_")) {
		const unsigned int collflag = GetGeomFlagForNodeName(nodename);
		RefCountedPtr<Graphics::Surface> surf = surfaces.at(node->mMeshes[0]);
		RefCountedPtr<CollisionGeometry> cgeom(new CollisionGeometry(surf.Get(), collflag));
		cgeom->SetName(nodename + "_cgeom");
		parent->AddChild(cgeom.Get());
		return;
	}

	//nodes with visible geometry (StaticGeometry and decals)
	if (node->mNumMeshes > 0) {
		//is this node animated? add a transform
		//does this node have children? Add a group
		RefCountedPtr<StaticGeometry> geom(new StaticGeometry());
		geom->SetName(nodename + "_mesh");
		RefCountedPtr<Graphics::StaticMesh> smesh(new Graphics::StaticMesh(Graphics::TRIANGLES));

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
				throw LoadingError("More than 4 different decals");
		}

		for(unsigned int i=0; i<node->mNumMeshes; i++) {
			RefCountedPtr<Graphics::Surface> surf = surfaces.at(node->mMeshes[i]);

			//turn on alpha blending and mark entire node as transparent
			//(all importers split by material so far)
			if (surf->GetMaterial()->diffuse.a < 0.99f) {
				geom->SetNodeMask(NODE_TRANSPARENT);
				geom->m_blendMode = Graphics::BLEND_ALPHA;
			}
			//set special material for decals
			if (numDecal > 0) {
				geom->SetNodeMask(NODE_TRANSPARENT);
				geom->m_blendMode = Graphics::BLEND_ALPHA;
				surf->SetMaterial(GetDecalMaterial(numDecal));
			}
			//update bounding box
			//untransformed points, collision visitor will transform
			Graphics::VertexArray *vts = surf->GetVertices();
			for (unsigned int j=0; j<vts->position.size(); j++) {
				const vector3f &vtx = vts->position[j];
				geom->m_boundingBox.Update(vtx.x, vtx.y, vtx.z);
			}

			//Out of space? Add a new mesh.
			if(smesh->GetAvailableVertexSpace() < surf->GetNumVerts()) {
				geom->AddMesh(smesh);
				smesh = RefCountedPtr<Graphics::StaticMesh>(new Graphics::StaticMesh(Graphics::TRIANGLES));
			}

			smesh->AddSurface(surf);
		}
		geom->AddMesh(smesh);

		parent->AddChild(geom.Get());
	}

	for(unsigned int i=0; i<node->mNumChildren; i++) {
		aiNode *child = node->mChildren[i];
		ConvertNodes(child, parent, surfaces, accum * m);
	}
}

void Loader::LoadCollision(const std::string &filename)
{
	//Convert all found aiMeshes into a geomtree. Materials,
	//Animations and node structure can be ignored
	assert(m_model);

	Assimp::Importer importer;
	importer.SetIOHandler(new AssimpFileSystem(FileSystem::gameDataFiles));

	//discard extra data
	importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS,
		aiComponent_COLORS    |
		aiComponent_TEXCOORDS |
		aiComponent_NORMALS   |
		aiComponent_MATERIALS
		);
	const aiScene *scene = importer.ReadFile(
		filename,
		aiProcess_RemoveComponent |
		aiProcess_Triangulate     |
		aiProcess_PreTransformVertices //"bake" transformations so we can disregard the structure
		);

	if(!scene)
		throw LoadingError("Could not load file");

	if(scene->mNumMeshes == 0)
		throw LoadingError("No geometry found");

	std::vector<unsigned short> indices;
	std::vector<vector3f> vertices;
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
		}
		indexOffset += mesh->mNumFaces*3;

		//vertices
		for (unsigned int v = 0; v < mesh->mNumVertices; v++) {
			const aiVector3D &vtx = mesh->mVertices[v];
			vertices.push_back(vector3f(vtx.x, vtx.y, vtx.z));
		}
	}

	assert(!vertices.empty() && !vertices.empty());

	//add pre-transformed geometry at the top level
	m_model->GetRoot()->AddChild(new CollisionGeometry(vertices, indices, 0));
}

unsigned int Loader::GetGeomFlagForNodeName(const std::string &nodename)
{
	if (nodename.length() >= 14) {
		const std::string pad = nodename.substr(10, 4);
		if (pad == "pad1")
			return 0x10;
		else if (pad == "pad2")
			return 0x11;
		else if (pad == "pad3")
			return 0x12;
		else if (pad == "pad4")
			return 0x14;
	}
	return 0x0;
}

}
