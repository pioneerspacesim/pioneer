// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
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
Loader::Loader(Graphics::Renderer *r, bool logWarnings)
: m_renderer(r)
, m_model(0)
, m_doLog(logWarnings)
, m_mostDetailedLod(false)
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
	m_logMessages.clear();

	FileSystem::FileSource &fileSource = FileSystem::gameDataFiles;
	for (FileSystem::FileEnumerator files(fileSource, basepath, FileSystem::FileEnumerator::Recurse); !files.Finished(); files.Next())
	{
		const FileSystem::FileInfo &info = files.Current();
		const std::string &fpath = info.GetPath();

		//check it's the expected type
		if (info.IsFile() && ends_with_ci(fpath, ".model")) {
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

Model *Loader::CreateModel(ModelDefinition &def)
{
	using Graphics::Material;
	if (def.matDefs.empty()) return 0;
	if (def.lodDefs.empty()) return 0;

	Model *model = new Model(m_renderer, def.name);
	m_model = model;
	bool patternsUsed = false;

	m_thrustersRoot.Reset(new Group(m_renderer));
	m_billboardsRoot.Reset(new Group(m_renderer));

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
			mat->diffuse.a = (float((*it).opacity) / 100.f) * 255;

		if (!diffTex.empty())
			mat->texture0 = Graphics::TextureBuilder::Model(diffTex).GetOrCreateTexture(m_renderer, "model");
		else
			mat->texture0 = Graphics::TextureBuilder::GetWhiteTexture(m_renderer);
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
		lodNode = new LOD(m_renderer);
		model->GetRoot()->AddChild(lodNode);
	}
	for(std::vector<LodDefinition>::const_iterator lod = def.lodDefs.begin();
		lod != def.lodDefs.end(); ++lod)
	{
		m_mostDetailedLod = (lod == def.lodDefs.end() - 1);

		//does a detail level have multiple meshes? If so, we need a Group.
		Group *group = 0;
		if (lodNode && (*lod).meshNames.size() > 1) {
			group = new Group(m_renderer);
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
						mesh = LoadMesh(*it, def.animDefs);
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

	if (m_thrustersRoot->GetNumChildren() > 0) {
		m_thrustersRoot->SetName("thrusters");
		m_thrustersRoot->SetNodeMask(NODE_TRANSPARENT);
		model->GetRoot()->AddChild(m_thrustersRoot.Get());
	}

	if (m_billboardsRoot->GetNumChildren() > 0) {
		m_billboardsRoot->SetName("navlights");
		m_billboardsRoot->SetNodeMask(NODE_TRANSPARENT);
		model->GetRoot()->AddChild(m_billboardsRoot.Get());
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
	m_model->CreateCollisionMesh();

	// Do an initial animation update to get all the animation transforms correct
	m_model->UpdateAnimations();

	//find usable pattern textures from the model directory
	if (patternsUsed) {
		FindPatterns(model->m_patterns);

		if (model->m_patterns.empty()) {
			model->m_patterns.push_back(Pattern());
			Pattern &dumpat = m_model->m_patterns.back();
			dumpat.name = "Dummy";
			dumpat.texture = RefCountedPtr<Graphics::Texture>(Graphics::TextureBuilder::GetWhiteTexture(m_renderer));
		}

		//set up some noticeable default colors
		std::vector<Color> colors;
		colors.push_back(Color::RED);
		colors.push_back(Color::GREEN);
		colors.push_back(Color::BLUE);
		model->SetColors(colors);
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
			if (ends_with_ci(name, ".png") && starts_with(name, "pattern"))
				output.push_back(Pattern(name, m_curPath, m_renderer));
		}
	}
}

RefCountedPtr<Node> Loader::LoadMesh(const std::string &filename, const AnimList &animDefs)
{
	//remove path from filename for nicer logging
	size_t slashpos = filename.rfind("/");
	m_curMeshDef = filename.substr(slashpos+1, filename.length()-slashpos);

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
		aiProcess_GenUVCoords		|
		aiProcess_FlipUVs			|
		aiProcess_SplitLargeMeshes	|
		aiProcess_GenSmoothNormals);  //only if normals not specified

	if(!scene)
		throw LoadingError("Couldn't load file");

	if(scene->mNumMeshes == 0)
		throw LoadingError("No geometry found");

	//turn all scene aiMeshes into Surfaces
	//Index matches assimp index.
	std::vector<RefCountedPtr<StaticGeometry> > geoms;
	ConvertAiMeshes(geoms, scene);

	// Recursive structure conversion. Matrix needs to be accumulated for
	// special features that are absolute-positioned (thrusters)
	RefCountedPtr<Node> meshRoot(new Group(m_renderer));

	ConvertNodes(scene->mRootNode, static_cast<Group*>(meshRoot.Get()), geoms, matrix4x4f::Identity());
	ConvertAnimations(scene, animDefs, static_cast<Group*>(meshRoot.Get()));

	return meshRoot;
}

static bool in_range(double keytime, double start, double end)
{
	return (keytime >= start - 0.001 && keytime - 0.001 <= end);
}

// check animation channel has a key within time range
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

	return (posKeysInRange > 0 || rotKeysInRange > 0 || sclKeysInRange > 0);
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
		decMat->texture0 = Graphics::TextureBuilder::GetTransparentTexture(m_renderer);
		decMat->specular = Color::BLACK;
		decMat->diffuse = Color::WHITE;
	}
	return decMat;
}

void Loader::AddLog(const std::string &msg)
{
	if (m_doLog) m_logMessages.push_back(msg);
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
			if (otherAnim == anim)
				continue;
			for (ChannelIterator otherChan = otherAnim->m_channels.begin(); otherChan != otherAnim->m_channels.end(); ++otherChan) {
				//warnings as errors mentality - this is not really fatal
				if (chan->node == otherChan->node)
					throw LoadingError(stringf("Animations %0 and %1 both control node: %2", anim->GetName(), otherAnim->GetName(), chan->node->GetName()));
			}
		}
	}
}

void Loader::ConvertAiMeshes(std::vector<RefCountedPtr<StaticGeometry> > &geoms, const aiScene *scene)
{
	//XXX sigh, workaround for obj loader
	int matIdxOffs = 0;
	if (scene->mNumMaterials > scene->mNumMeshes)
		matIdxOffs = 1;

	//turn meshes into static geometry nodes
	for (unsigned int i=0; i<scene->mNumMeshes; i++) {
		const aiMesh *mesh = scene->mMeshes[i];
		assert(mesh->HasNormals());

		RefCountedPtr<StaticGeometry> geom(new StaticGeometry(m_renderer));
		geom->SetName(stringf("sgMesh%0{u}", i));

		const bool hasUVs = mesh->HasTextureCoords(0);
		if (!hasUVs) AddLog(stringf("%0: missing UV coordinates", m_curMeshDef));
		//sadly, aimesh name is usually empty so no help for logging

		//Material names are not consistent throughout formats.
		//try matching name first, if that fails use index
		RefCountedPtr<Graphics::Material> mat;
		const aiMaterial *amat = scene->mMaterials[mesh->mMaterialIndex];
		aiString aiMatName;
		if(AI_SUCCESS == amat->Get(AI_MATKEY_NAME,aiMatName))
			mat = m_model->GetMaterialByName(std::string(aiMatName.C_Str()));

		if (!mat.Valid()) {
			const unsigned int matIdx = mesh->mMaterialIndex - matIdxOffs;
			AddLog(stringf("%0: no material %1, using material %2{u} instead", m_curMeshDef, aiMatName.C_Str(), matIdx+1));
			mat = m_model->GetMaterialByIndex(matIdx);
		}
		assert(mat.Valid());

		//turn on alpha blending and mark entire node as transparent
		//(all importers split by material so far)
		if (mat->diffuse.a < 255) {
			geom->SetNodeMask(NODE_TRANSPARENT);
			geom->m_blendMode = Graphics::BLEND_ALPHA;
		}

		const Graphics::AttributeSet vtxAttribs =
			Graphics::ATTRIB_POSITION |
			Graphics::ATTRIB_NORMAL |
			Graphics::ATTRIB_UV0;
		Graphics::VertexArray *vts = new Graphics::VertexArray(vtxAttribs, mesh->mNumVertices);

		// huge meshes are split by the importer so this should not exceed 65K indices
		RefCountedPtr<Graphics::Surface> surface(new Graphics::Surface(Graphics::TRIANGLES, vts, mat));
		std::vector<unsigned short> &indices = surface->GetIndices();
		indices.reserve(mesh->mNumFaces * 3);

		//copy indices first
		for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
			const aiFace *face = &mesh->mFaces[f];
			for (unsigned int j = 0; j < face->mNumIndices; j++) {
				indices.push_back(face->mIndices[j]);
			}
		}

		//copy vertices, always assume normals
		//replace nonexistent UVs with zeros
		for (unsigned int v = 0; v < mesh->mNumVertices; v++) {
			const aiVector3D &vtx = mesh->mVertices[v];
			const aiVector3D &norm = mesh->mNormals[v];
			const aiVector3D &uv0 = hasUVs ? mesh->mTextureCoords[0][v] : aiVector3D(0.f);
			vts->Add(vector3f(vtx.x, vtx.y, vtx.z),
				vector3f(norm.x, norm.y, norm.z),
				vector2f(uv0.x, uv0.y));

			//update bounding box
			//untransformed points, collision visitor will transform
			geom->m_boundingBox.Update(vtx.x, vtx.y, vtx.z);
		}

		RefCountedPtr<Graphics::StaticMesh> smesh(new Graphics::StaticMesh(Graphics::TRIANGLES));
		smesh->AddSurface(surface);
		geom->AddMesh(smesh);

		geoms.push_back(geom);
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
		const double secondsPerTick = 1.0 / ticksPerSecond;

		double start = DBL_MAX;
		double end = -DBL_MAX;

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

		// Add channels to current animation if it's already present
		// Necessary to make animations work in multiple LODs
		Animation *animation = m_model->FindAnimation(def->name);
		const bool newAnim = !animation;
		if (newAnim) animation = new Animation(def->name, 0.0);

		const size_t first_new_channel = animation->m_channels.size();

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
						const double t = aikey.mTime * secondsPerTick;
						chan.positionKeys.push_back(PositionKey(t, vector3f(aipos.x, aipos.y, aipos.z)));
						start = std::min(start, t);
						end = std::max(end, t);
					}
				}

				//scale interpolation will blow up without rotation keys,
				//so skipping them when rotkeys < 2 is correct
				if (aichan->mNumRotationKeys < 2) continue;

				for(unsigned int k=0; k<aichan->mNumRotationKeys; k++) {
					const aiQuatKey &aikey = aichan->mRotationKeys[k];
					const aiQuaternion &airot = aikey.mValue;
					if (in_range(aikey.mTime, defStart, defEnd)) {
						const double t = aikey.mTime * secondsPerTick;
						chan.rotationKeys.push_back(RotationKey(t, Quaternionf(airot.w, airot.x, airot.y, airot.z)));
						start = std::min(start, t);
						end = std::max(end, t);
					}
				}

				for(unsigned int k=0; k<aichan->mNumScalingKeys; k++) {
					const aiVectorKey &aikey = aichan->mScalingKeys[k];
					const aiVector3D &aipos = aikey.mValue;
					if (in_range(aikey.mTime, defStart, defEnd)) {
						const double t = aikey.mTime * secondsPerTick;
						chan.scaleKeys.push_back(ScaleKey(t, vector3f(aipos.x, aipos.y, aipos.z)));
						start = std::min(start, t);
						end = std::max(end, t);
					}
				}
			}
		}

		// convert remove initial offset (so the first keyframe is at exactly t=0)
		for (std::vector<AnimationChannel>::iterator chan = animation->m_channels.begin() + first_new_channel;
				chan != animation->m_channels.end(); ++chan) {
			for (unsigned int k = 0; k < chan->positionKeys.size(); ++k) {
				chan->positionKeys[k].time -= start;
				assert(chan->positionKeys[k].time >= 0.0);
			}
			for (unsigned int k = 0; k < chan->rotationKeys.size(); ++k) {
				chan->rotationKeys[k].time -= start;
				assert(chan->rotationKeys[k].time >= 0.0);
			}
			for (unsigned int k = 0; k < chan->scaleKeys.size(); ++k) {
				chan->scaleKeys[k].time -= start;
				assert(chan->scaleKeys[k].time >= 0.0);
			}
		}

		// set actual duration
		const double dur = end - start;
		animation->m_duration = newAnim ? dur : std::max(animation->m_duration, dur);

		//do final sanity checking before adding
		try {
			CheckAnimationConflicts(animation, animations);
		} catch (LoadingError &) {
			if (newAnim) delete animation;
			throw;
		}

		if (newAnim) {
			if (animation->m_channels.empty())
				delete animation;
			else
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
	MatrixTransform *trans = new MatrixTransform(m_renderer, m);
	Label3D *label = new Label3D(m_renderer, m_labelFont);
	label->SetText("Bananas");
	trans->AddChild(label);
	parent->AddChild(trans);
}

void Loader::CreateThruster(const std::string &name, const matrix4x4f &m)
{
	if (!m_mostDetailedLod) return AddLog("Thruster outside highest LOD, ignored");

	const bool linear = starts_with(name, "thruster_linear");

	matrix4x4f transform = m;

	MatrixTransform *trans = new MatrixTransform(m_renderer, transform);

	const vector3f pos = transform.GetTranslate();
	transform.ClearToRotOnly();

	const vector3f direction = transform * vector3f(0.f, 0.f, 1.f);

	Thruster *thruster = new Thruster(m_renderer, linear,
		pos, direction.Normalized());

	thruster->SetName(name);
	trans->AddChild(thruster);

	m_thrustersRoot->AddChild(trans);
}

void Loader::CreateNavlight(const std::string &name, const matrix4x4f &m)
{
	if (!m_mostDetailedLod) return AddLog("Navlight outside highest LOD, ignored");

	//Create a MT, lights are attached by client
	//we only really need the final position, so this is
	//a waste of transform
	const matrix4x4f lightPos = matrix4x4f::Translation(m.GetTranslate());
	MatrixTransform *lightPoint = new MatrixTransform(m_renderer, lightPos);
	lightPoint->SetNodeMask(0x0); //don't render
	lightPoint->SetName(name);

	m_billboardsRoot->AddChild(lightPoint);
}

void Loader::ConvertNodes(aiNode *node, Group *_parent, std::vector<RefCountedPtr<StaticGeometry> >& geoms, const matrix4x4f &accum)
{
	Group *parent = _parent;
	const std::string nodename(node->mName.C_Str());
	const aiMatrix4x4& trans = node->mTransformation;
	matrix4x4f m = ConvertMatrix(trans);

	//lights, and possibly other special nodes should be leaf nodes (without meshes)
	if (node->mNumChildren == 0 && node->mNumMeshes == 0) {
		if (starts_with(nodename, "navlight_")) {
			CreateNavlight(nodename, accum*m);
		} else if (starts_with(nodename, "thruster_")) {
			CreateThruster(nodename, accum*m);
		} else if (starts_with(nodename, "label_")) {
			CreateLabel(parent, m);
		} else if (starts_with(nodename, "tag_")) {
			m_model->AddTag(nodename, new MatrixTransform(m_renderer, accum*m));
		} else if (starts_with(nodename, "docking_")) {
			m_model->AddTag(nodename, new MatrixTransform(m_renderer, m));
		} else if (starts_with(nodename, "leaving_")) {
			m_model->AddTag(nodename, new MatrixTransform(m_renderer, m));
		} else if (starts_with(nodename, "approach_")) {
			m_model->AddTag(nodename, new MatrixTransform(m_renderer, m));
		}
		return;
	}

	//if the transform is identity and the node is not animated,
	//could just add a group
	parent = new MatrixTransform(m_renderer, m);
	_parent->AddChild(parent);
	parent->SetName(nodename);

	//nodes named collision_* are not added as renderable geometry
	if (node->mNumMeshes == 1 && starts_with(nodename, "collision_")) {
		const unsigned int collflag = GetGeomFlagForNodeName(nodename);
		RefCountedPtr<Graphics::Surface> surf = geoms.at(node->mMeshes[0])->GetMesh(0)->GetSurface(0);
		RefCountedPtr<CollisionGeometry> cgeom(new CollisionGeometry(m_renderer, surf.Get(), collflag));
		cgeom->SetName(nodename + "_cgeom");
		cgeom->SetDynamic(starts_with(nodename, "collision_d"));
		parent->AddChild(cgeom.Get());
		return;
	}

	//nodes with visible geometry (StaticGeometry and decals)
	if (node->mNumMeshes > 0) {
		//expecting decal_0X
		unsigned int numDecal = 0;
		if (starts_with(nodename, "decal_")) {
			numDecal = atoi(nodename.substr(7,1).c_str());
			if (numDecal > 4)
				throw LoadingError("More than 4 different decals");
		}

		for(unsigned int i=0; i<node->mNumMeshes; i++) {
			RefCountedPtr<StaticGeometry> geom = geoms.at(node->mMeshes[i]);

			//handle special decal material
			//set special material for decals
			if (numDecal > 0) {
				geom->SetNodeMask(NODE_TRANSPARENT);
				geom->m_blendMode = Graphics::BLEND_ALPHA;
				geom->GetMesh(0)->GetSurface(0)->SetMaterial(GetDecalMaterial(numDecal));
			}

			parent->AddChild(geom.Get());
		}
	}

	for(unsigned int i=0; i<node->mNumChildren; i++) {
		aiNode *child = node->mChildren[i];
		ConvertNodes(child, parent, geoms, accum * m);
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
	m_model->GetRoot()->AddChild(new CollisionGeometry(m_renderer, vertices, indices, 0));
}

unsigned int Loader::GetGeomFlagForNodeName(const std::string &nodename)
{
	//special names after collision_
	if (nodename.length() > 10) {
		//landing pads
		if (nodename.length() >= 14 && nodename.substr(10,3) == "pad") {
			const std::string pad = nodename.substr(13);
			const int padID = atoi(pad.c_str())-1;
			if(padID<240) {
				return 0x10 + padID;
			}
		}
	}
	//anything else is static collision
	return 0x0;
}
}
