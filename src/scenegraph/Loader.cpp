// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Loader.h"

#include "Animation.h"
#include "BinaryConverter.h"
#include "CollisionGeometry.h"
#include "LOD.h"
#include "Parser.h"
#include "SceneGraph.h"
#include "Tag.h"

#include "FileSystem.h"
#include "StringF.h"
#include "core/Log.h"
#include "graphics/RenderState.h"
#include "graphics/Renderer.h"
#include "graphics/TextureBuilder.h"
#include "scenegraph/Animation.h"
#include "scenegraph/LoaderDefinitions.h"
#include "utils.h"

#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/version.h>
#include <assimp/IOStream.hpp>
#include <assimp/IOSystem.hpp>
#include <assimp/Importer.hpp>

namespace {
	class AssimpFileReadStream : public Assimp::IOStream {
	public:
		explicit AssimpFileReadStream(const RefCountedPtr<FileSystem::FileData> &data) :
			m_data(data)
		{
			m_cursor = m_data->GetData();
		}

		virtual ~AssimpFileReadStream() {}

		virtual size_t FileSize() const { return m_data->GetSize(); }

		virtual size_t Read(void *buf, size_t size, size_t count)
		{
			const char *const data_end = m_data->GetData() + m_data->GetSize();
			const size_t remaining = (data_end - m_cursor);
			const size_t requested = size * count;
			const size_t len = std::min(remaining, requested);
			memcpy(static_cast<char *>(buf), m_cursor, len);
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
			if (offset > m_data->GetSize())
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

	class AssimpFileSystem : public Assimp::IOSystem {
	public:
		AssimpFileSystem(FileSystem::FileSource &fs) :
			m_fs(fs) {}
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
	Loader::Loader(Graphics::Renderer *r, bool logWarnings, bool loadSGMfiles) :
		BaseLoader(r),
		m_doLog(logWarnings),
		m_loadSGMs(loadSGMfiles),
		m_mostDetailedLod(false)
	{
	}

	Model *Loader::LoadModel(const std::string &filename)
	{
		return LoadModel(filename, "models");
	}

	Model *Loader::LoadModel(const std::string &shortname, const std::string &basepath)
	{
		PROFILE_SCOPED()
		m_logMessages.clear();

		std::vector<std::string> list_model;
		std::vector<std::string> list_sgm;
		FileSystem::FileSource &fileSource = FileSystem::gameDataFiles;
		for (FileSystem::FileEnumerator files(fileSource, basepath, FileSystem::FileEnumerator::Recurse); !files.Finished(); files.Next()) {
			const FileSystem::FileInfo &info = files.Current();
			const std::string &fpath = info.GetPath();

			//check it's the expected type
			if (info.IsFile()) {
				if (ends_with_ci(fpath, ".model")) { // store the path for ".model" files
					list_model.push_back(fpath);
				} else if (m_loadSGMs & ends_with_ci(fpath, ".sgm")) { // store only the shortname for ".sgm" files.
					list_sgm.push_back(info.GetName().substr(0, info.GetName().size() - 4));
				}
			}
		}

		if (m_loadSGMs) {
			for (auto &sgmname : list_sgm) {
				if (sgmname == shortname) {
					//binary loader expects extension-less name. Might want to change this.
					SceneGraph::BinaryConverter bc(m_renderer);
					m_model = bc.Load(shortname);
					if (m_model)
						return m_model;
					else
						break; // we'll have to load the non-sgm file
				}
			}
		}

		for (auto &fpath : list_model) {
			RefCountedPtr<FileSystem::FileData> filedata = FileSystem::gameDataFiles.ReadFile(fpath);
			if (!filedata) {
				Output("LoadModel: %s: could not read file\n", fpath.c_str());
				return nullptr;
			}

			//check it's the wanted name & load it
			const FileSystem::FileInfo &info = filedata->GetInfo();
			const std::string name = info.GetName();
			if (name.substr(0, name.length() - 6) == shortname) {
				ModelDefinition modelDefinition;
				try {
					//curPath is used to find textures, patterns,
					//possibly other data files for this model.
					//Strip trailing slash
					m_curPath = info.GetDir();
					assert(!m_curPath.empty());
					if (m_curPath[m_curPath.length() - 1] == '/')
						m_curPath = m_curPath.substr(0, m_curPath.length() - 1);

					Parser p(fileSource, fpath, m_curPath);
					p.Parse(&modelDefinition);
				} catch (ParseError &err) {
					Output("%s\n", err.what());
					throw LoadingError(err.what());
				}
				modelDefinition.name = shortname;
				return CreateModel(modelDefinition);
			}
		}
		throw(LoadingError("File not found"));
	}

	Model *Loader::CreateModel(ModelDefinition &def)
	{
		PROFILE_SCOPED()
		using Graphics::Material;
		if (def.matDefs.empty()) return 0;
		if (def.lodDefs.empty()) return 0;

		Model *model = new Model(m_renderer, def.name);
		m_model = model;
		bool patternsUsed = false;

		m_thrustersRoot.Reset(new Group(m_renderer));
		m_billboardsRoot.Reset(new Group(m_renderer));

		//create materials from definitions
		for (std::vector<MaterialDefinition>::const_iterator it = def.matDefs.begin();
			 it != def.matDefs.end(); ++it) {
			if (it->use_pattern) patternsUsed = true;
			ConvertMaterialDefinition(*it);
		}
		//Output("Loaded %d materials\n", int(model->m_materials.size()));

		//load meshes
		//"mesh" here refers to a "mesh xxx.yyy"
		//defined in the .model
		std::map<std::string, RefCountedPtr<Node>> meshCache;
		LOD *lodNode = 0;
		if (def.lodDefs.size() > 1) { //don't bother with a lod node if only one level
			lodNode = new LOD(m_renderer);
			model->GetRoot()->AddChild(lodNode);
		}
		for (std::vector<LodDefinition>::const_iterator lod = def.lodDefs.begin();
			 lod != def.lodDefs.end(); ++lod) {
			m_mostDetailedLod = (lod == def.lodDefs.end() - 1);

			//does a detail level have multiple meshes? If so, we need a Group.
			Group *group = 0;
			if (lodNode && (*lod).meshNames.size() > 1) {
				group = new Group(m_renderer);
				lodNode->AddLevel((*lod).pixelSize, group);
			}
			for (std::vector<std::string>::const_iterator it = (*lod).meshNames.begin();
				 it != (*lod).meshNames.end(); ++it) {
				try {
					//multiple lods might use the same mesh
					RefCountedPtr<Node> mesh;
					std::map<std::string, RefCountedPtr<Node>>::iterator cacheIt = meshCache.find((*it));
					if (cacheIt != meshCache.end())
						mesh = (*cacheIt).second;
					else {
						try {
							mesh = LoadMesh(*it, def.animDefs);
						} catch (LoadingError &err) {
							//append filename - easiest to do here
							throw(LoadingError(stringf("%0:\n%1", *it, err.what())));
						}
						meshCache[*(it)] = mesh;
					}
					assert(mesh.Valid());

					if (group)
						group->AddChild(mesh.Get());
					else if (lodNode) {
						lodNode->AddLevel((*lod).pixelSize, mesh.Get());
					} else
						model->GetRoot()->AddChild(mesh.Get());
				} catch (LoadingError &err) {
					delete model;
					Output("%s\n", err.what());
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
			 it != def.collisionDefs.end(); ++it) {
			try {
				LoadCollision(*it);
			} catch (LoadingError &err) {
				throw(LoadingError(stringf("%0:\n%1", *it, err.what())));
			}
		}

		// Run CollisionVisitor to create the initial CM and its GeomTree.
		// If no collision mesh is defined, a simple bounding box will be generated
		Output("CreateCollisionMesh for : (%s)\n", m_model->m_name.c_str());
		m_model->CreateCollisionMesh();

		// Do an initial animation update to get all the animation transforms correct
		m_model->InitAnimations();

		//find usable pattern textures from the model directory
		if (patternsUsed)
			SetUpPatterns();

		// initialize tag transforms
		m_model->UpdateTagTransforms();

		return model;
	}

	RefCountedPtr<Node> Loader::LoadMesh(const std::string &filename, const std::vector<AnimDefinition> &animDefs)
	{
		PROFILE_SCOPED()
		Log::Verbose("Loading mesh '{}'", filename);

		//remove path from filename for nicer logging
		size_t slashpos = filename.rfind("/");
		m_curMeshDef = filename.substr(slashpos + 1, filename.length() - slashpos);

		std::string_view ext = filename;
		ext = ext.substr(ext.rfind("."));

		if (ext == ".dae")
			m_modelFormat = ModelFormat::COLLADA;
		else if (ext == ".gltf")
			m_modelFormat = ModelFormat::GLTF;
		else if (ext == ".obj")
			m_modelFormat = ModelFormat::WAVEFRONT;
		else
			m_modelFormat = ModelFormat::UNKNOWN;

		Assimp::Importer importer;
		importer.SetIOHandler(new AssimpFileSystem(FileSystem::gameDataFiles));

		//Removing components is suggested to optimize loading. We do not care about vtx colors now.
		importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_COLORS);
		importer.SetPropertyInteger(AI_CONFIG_PP_SLM_VERTEX_LIMIT, AI_SLM_DEFAULT_MAX_VERTICES);

		//There are several optimizations assimp can do, intentionally skipping them now
		const aiScene *scene = importer.ReadFile(
			filename,
			aiProcess_RemoveComponent |
				aiProcess_Triangulate |
				aiProcess_SortByPType | //ignore point, line primitive types (collada dummy nodes seem to be fine)
				aiProcess_GenUVCoords |
				aiProcess_FlipUVs |
				aiProcess_CalcTangentSpace |
				aiProcess_JoinIdenticalVertices |
				aiProcess_GenSmoothNormals | //only if normals not specified
				aiProcess_ImproveCacheLocality |
				aiProcess_LimitBoneWeights |
				aiProcess_FindDegenerates |
				aiProcess_FindInvalidData);

		if (!scene) {
			// Assimp 3.1.1 doesn't have aiGetVersionPatch(), add it back in at some point
			std::string err = fmt::format("Assimp {}.{} importer error: {}\n",
				aiGetVersionMajor(), aiGetVersionMinor(), importer.GetErrorString());
			throw LoadingError(err);
		}

		if (scene->mNumMeshes == 0)
			throw LoadingError("No geometry found");

		//turn all scene aiMeshes into Surfaces
		//Index matches assimp index.
		std::vector<RefCountedPtr<StaticGeometry>> geoms;
		ConvertAiMeshes(geoms, scene);

		// Recursive structure conversion. Matrix needs to be accumulated for
		// special features that are absolute-positioned (thrusters)
		RefCountedPtr<Node> meshRoot(new Group(m_renderer));

		ConvertNodes(scene->mRootNode, static_cast<Group *>(meshRoot.Get()), geoms, matrix4x4f::Identity());
		ConvertAnimations(scene, animDefs, static_cast<Group *>(meshRoot.Get()));

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

		for (unsigned int k = 0; k < chan->mNumPositionKeys; k++) {
			const aiVectorKey &aikey = chan->mPositionKeys[k];
			if (in_range(aikey.mTime, start, end)) posKeysInRange++;
		}

		for (unsigned int k = 0; k < chan->mNumRotationKeys; k++) {
			const aiQuatKey &aikey = chan->mRotationKeys[k];
			if (in_range(aikey.mTime, start, end)) rotKeysInRange++;
		}

		for (unsigned int k = 0; k < chan->mNumScalingKeys; k++) {
			const aiVectorKey &aikey = chan->mScalingKeys[k];
			if (in_range(aikey.mTime, start, end)) sclKeysInRange++;
		}

		return (posKeysInRange > 0 || rotKeysInRange > 0 || sclKeysInRange > 0);
	}

	void Loader::AddLog(const std::string &msg)
	{
		if (m_doLog) m_logMessages.push_back(msg);
	}

	void Loader::CheckAnimationConflicts(const Animation *anim, const std::vector<Animation *> &otherAnims)
	{
		typedef std::vector<AnimationChannel>::const_iterator ChannelIterator;
		typedef std::vector<Animation *>::const_iterator AnimIterator;

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

#pragma pack(push, 4)
	struct ModelVtx {
		vector3f pos;
		vector3f nrm;
		vector2f uv0;
	};

	struct ModelTangentVtx {
		vector3f pos;
		vector3f nrm;
		vector2f uv0;
		vector3f tangent;
	};
#pragma pack(pop)

	void Loader::ConvertAiMeshes(std::vector<RefCountedPtr<StaticGeometry>> &geoms, const aiScene *scene)
	{
		PROFILE_SCOPED()
		//XXX sigh, workaround for obj loader
		int matIdxOffs = 0;
		if (scene->mNumMaterials > scene->mNumMeshes)
			matIdxOffs = 1;

		//turn meshes into static geometry nodes
		for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
			const aiMesh *mesh = scene->mMeshes[i];
			assert(mesh->HasNormals());

			RefCountedPtr<StaticGeometry> geom(new StaticGeometry(m_renderer));
			geom->SetName(stringf("sgMesh%0{u}", i));

			const bool hasUVs = mesh->HasTextureCoords(0);
			const bool hasTangents = mesh->HasTangentsAndBitangents();
			if (!hasUVs)
				AddLog(stringf("%0: missing UV coordinates", m_curMeshDef));
			if (!hasTangents)
				AddLog(stringf("%0: missing Tangents and Bitangents coordinates", m_curMeshDef));
			//sadly, aimesh name is usually empty so no help for logging

			//Material names are not consistent throughout formats.
			//try matching name first, if that fails use index
			RefCountedPtr<Graphics::Material> mat;
			const aiMaterial *amat = scene->mMaterials[mesh->mMaterialIndex];
			aiString aiMatName;
			if (AI_SUCCESS == amat->Get(AI_MATKEY_NAME, aiMatName))
				mat = m_model->GetMaterialByName(std::string(aiMatName.C_Str()));

			if (!mat.Valid()) {
				const unsigned int matIdx = mesh->mMaterialIndex - matIdxOffs;
				AddLog(stringf("%0: no material %1, using material %2{u} instead", m_curMeshDef, aiMatName.C_Str(), matIdx + 1));
				mat = m_model->GetMaterialByIndex(matIdx);
			}
			assert(mat.Valid());

			//turn on alpha blending and mark entire node as transparent
			//(all importers split by material so far)
			if (mat->diffuse.a < 255)
				geom->SetNodeMask(NODE_TRANSPARENT);

			Graphics::VertexBufferDesc vbd;
			vbd.attrib[0].semantic = Graphics::ATTRIB_POSITION;
			vbd.attrib[0].format = Graphics::ATTRIB_FORMAT_FLOAT3;
			vbd.attrib[0].offset = hasTangents ? offsetof(ModelTangentVtx, pos) : offsetof(ModelVtx, pos);
			vbd.attrib[1].semantic = Graphics::ATTRIB_NORMAL;
			vbd.attrib[1].format = Graphics::ATTRIB_FORMAT_FLOAT3;
			vbd.attrib[1].offset = hasTangents ? offsetof(ModelTangentVtx, nrm) : offsetof(ModelVtx, nrm);
			vbd.attrib[2].semantic = Graphics::ATTRIB_UV0;
			vbd.attrib[2].format = Graphics::ATTRIB_FORMAT_FLOAT2;
			vbd.attrib[2].offset = hasTangents ? offsetof(ModelTangentVtx, uv0) : offsetof(ModelVtx, uv0);
			if (hasTangents) {
				vbd.attrib[3].semantic = Graphics::ATTRIB_TANGENT;
				vbd.attrib[3].format = Graphics::ATTRIB_FORMAT_FLOAT3;
				vbd.attrib[3].offset = offsetof(ModelTangentVtx, tangent);
			}
			vbd.stride = hasTangents ? sizeof(ModelTangentVtx) : sizeof(ModelVtx);
			vbd.numVertices = mesh->mNumVertices;
			vbd.usage = Graphics::BUFFER_USAGE_STATIC;

			RefCountedPtr<Graphics::VertexBuffer> vb(m_renderer->CreateVertexBuffer(vbd));

			// huge meshes are split by the importer so this should not exceed 65K indices
			std::vector<Uint32> indices;
			if (mesh->mNumFaces > 0) {
				indices.reserve(mesh->mNumFaces * 3);
				for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
					const aiFace *face = &mesh->mFaces[f];
					for (unsigned int j = 0; j < face->mNumIndices; j++) {
						indices.push_back(face->mIndices[j]);
					}
				}
			} else {
				//generate dummy indices
				AddLog(stringf("Missing indices in mesh %0{u}", i));
				indices.reserve(mesh->mNumVertices);
				for (unsigned int v = 0; v < mesh->mNumVertices; v++)
					indices.push_back(v);
			}

			assert(indices.size() > 0);

			//create buffer & copy
			RefCountedPtr<Graphics::IndexBuffer> ib(m_renderer->CreateIndexBuffer(indices.size(), Graphics::BUFFER_USAGE_STATIC));
			Uint32 *idxPtr = ib->Map(Graphics::BUFFER_MAP_WRITE);
			for (Uint32 j = 0; j < indices.size(); j++)
				idxPtr[j] = indices[j];
			ib->Unmap();

			//copy vertices, always assume normals
			//replace nonexistent UVs with zeros
			if (!hasTangents) {
				ModelVtx *vtxPtr = vb->Map<ModelVtx>(Graphics::BUFFER_MAP_WRITE);
				for (unsigned int v = 0; v < mesh->mNumVertices; v++) {
					const aiVector3D &vtx = mesh->mVertices[v];
					const aiVector3D &norm = mesh->mNormals[v];
					const aiVector3D &uv0 = hasUVs ? mesh->mTextureCoords[0][v] : aiVector3D(0.f);
					vtxPtr[v].pos = vector3f(vtx.x, vtx.y, vtx.z);
					vtxPtr[v].nrm = vector3f(norm.x, norm.y, norm.z);
					vtxPtr[v].uv0 = vector2f(uv0.x, uv0.y);

					//update bounding box
					//untransformed points, collision visitor will transform
					geom->m_boundingBox.Update(vtx.x, vtx.y, vtx.z);
				}
				vb->Unmap();
			} else {
				ModelTangentVtx *vtxPtr = vb->Map<ModelTangentVtx>(Graphics::BUFFER_MAP_WRITE);
				for (unsigned int v = 0; v < mesh->mNumVertices; v++) {
					const aiVector3D &vtx = mesh->mVertices[v];
					const aiVector3D &norm = mesh->mNormals[v];
					const aiVector3D &uv0 = hasUVs ? mesh->mTextureCoords[0][v] : aiVector3D(0.f);
					const aiVector3D &tangents = mesh->mTangents[v];
					vtxPtr[v].pos = vector3f(vtx.x, vtx.y, vtx.z);
					vtxPtr[v].nrm = vector3f(norm.x, norm.y, norm.z);
					vtxPtr[v].uv0 = vector2f(uv0.x, uv0.y);
					vtxPtr[v].tangent = vector3f(tangents.x, tangents.y, tangents.z);

					//update bounding box
					//untransformed points, collision visitor will transform
					geom->m_boundingBox.Update(vtx.x, vtx.y, vtx.z);
				}
				vb->Unmap();
			}

			geom->AddMesh(vb, ib, mat);

			geoms.push_back(geom);
		}
	}

	void Loader::ConvertAnimations(const aiScene *scene, const std::vector<AnimDefinition> &animDefs, Node *meshRoot)
	{
		PROFILE_SCOPED()
		//Split convert assimp animations according to anim defs
		//This is very limited, and all animdefs are processed for all
		//meshes, potentially leading to duplicate and wrongly split animations
		if (animDefs.empty() || scene->mNumAnimations == 0)
			return;

		if (scene->mNumAnimations > 1)
			Output("File has %d animations, treating as one animation\n", scene->mNumAnimations);

		std::vector<Animation *> &animations = m_model->m_animations;

		for (const AnimDefinition &def : animDefs) {
			Log::Verbose("\tLoading animation definition {}\n", def.name);

			//XXX format differences: for a 40-frame animation exported from Blender,
			//.X results in duration 39 and Collada in Duration 1.25.
			//duration is calculated after adding all keys
			//take TPS from the first animation
			const aiAnimation *firstAnim = scene->mAnimations[0];
			double ticksPerSecond = firstAnim->mTicksPerSecond > 0.0 ? firstAnim->mTicksPerSecond : 24.0;
			double secondsPerTick = 1.0 / ticksPerSecond;

			// FIXME: we assume 24 frames per second here, this should be specified in the model file
			//Ranges are specified in frames (since that's nice) but Assimp
			//uses seconds. This is easiest to detect from ticksPerSecond,
			//but assuming 24 FPS here
			//Could make FPS an additional define or always require 24
			const double framesPerSecond = 24.0;

			double start = DBL_MAX;
			double end = -DBL_MAX;

			double defStart = def.start * ticksPerSecond / framesPerSecond;
			double defEnd = def.end * ticksPerSecond / framesPerSecond;

			// Add channels to current animation if it's already present
			// Necessary to make animations work in multiple LODs
			Animation *animation = m_model->FindAnimation(def.name);
			const bool newAnim = !animation;
			if (newAnim)
				animation = new Animation(def.name, 0.0);

			const size_t first_new_channel = animation->m_channels.size();

			for (unsigned int i = 0; i < scene->mNumAnimations; i++) {
				const aiAnimation *aianim = scene->mAnimations[i];
				Log::Verbose("\tProcessing model animation [{}] '{}' ({} channels)\n", i, aianim->mName.C_Str(), aianim->mNumChannels);

				for (unsigned int j = 0; j < aianim->mNumChannels; j++) {
					const aiNodeAnim *aichan = aianim->mChannels[j];

					//do a preliminary check that at least two keys in one channel are within range
					if (!CheckKeysInRange(aichan, defStart, defEnd))
						continue;

					const std::string channame(aichan->mNodeName.C_Str());
					MatrixTransform *trans = dynamic_cast<MatrixTransform *>(meshRoot->FindNode(channame));

					if (!trans) {
						// possibly dummy single-frame data for a node that's been converted to e.g. a label
						continue;
					}

					animation->m_channels.push_back(AnimationChannel(trans));
					AnimationChannel &chan = animation->m_channels.back();

					for (unsigned int k = 0; k < aichan->mNumPositionKeys; k++) {
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

					for (unsigned int k = 0; k < aichan->mNumRotationKeys; k++) {
						const aiQuatKey &aikey = aichan->mRotationKeys[k];
						const aiQuaternion &airot = aikey.mValue;
						if (in_range(aikey.mTime, defStart, defEnd)) {
							const double t = aikey.mTime * secondsPerTick;
							chan.rotationKeys.push_back(RotationKey(t, Quaternionf(airot.w, airot.x, airot.y, airot.z)));
							start = std::min(start, t);
							end = std::max(end, t);
						}
					}

					for (unsigned int k = 0; k < aichan->mNumScalingKeys; k++) {
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
			} catch (LoadingError &e) {
				Log::Warning("\tError processing animation conflicts for animation definition {}: {}", def.name, e.what());
				if (newAnim)
					delete animation;
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

	matrix4x4f Loader::ConvertMatrix(const aiMatrix4x4 &trans) const
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

	void Loader::CreateLabel(const std::string &name, Group *parent, const matrix4x4f &m)
	{
		PROFILE_SCOPED()
		MatrixTransform *trans = new MatrixTransform(m_renderer, m);

		Label3D *label = new Label3D(m_renderer, m_labelFont);
		label->SetText("Bananas");
		label->SetName(name);

		trans->AddChild(label);
		parent->AddChild(trans);
	}

	void Loader::CreateThruster(const std::string &name, const matrix4x4f &m)
	{
		PROFILE_SCOPED()
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
		PROFILE_SCOPED()
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

	RefCountedPtr<CollisionGeometry> Loader::CreateCollisionGeometry(RefCountedPtr<StaticGeometry> geom, unsigned int collFlag)
	{
		PROFILE_SCOPED()
		//Convert StaticMesh points & indices into cgeom
		//note: it's not slow, but the amount of data being copied is just stupid:
		//assimp to vtxbuffer, vtxbuffer to vector, vector to cgeom, cgeom to geomtree...
		assert(geom->GetNumMeshes() == 1);
		StaticGeometry::Mesh mesh = geom->GetMeshAt(0);

		const Uint32 posOffs = mesh.vertexBuffer->GetDesc().GetOffset(Graphics::ATTRIB_POSITION);
		const Uint32 stride = mesh.vertexBuffer->GetDesc().stride;
		const Uint32 numVtx = mesh.vertexBuffer->GetDesc().numVertices;
		const Uint32 numIdx = mesh.indexBuffer->GetSize();

		//copy vertex positions from buffer
		std::vector<vector3f> pos;
		pos.reserve(numVtx);

		Uint8 *vtxPtr = mesh.vertexBuffer->Map<Uint8>(Graphics::BUFFER_MAP_READ);
		for (Uint32 i = 0; i < numVtx; i++)
			pos.push_back(*reinterpret_cast<vector3f *>(vtxPtr + (i * stride) + posOffs));
		mesh.vertexBuffer->Unmap();

		//copy indices from buffer
		std::vector<Uint32> idx;
		idx.reserve(numIdx);

		Uint32 *idxPtr = mesh.indexBuffer->Map(Graphics::BUFFER_MAP_READ);
		for (Uint32 i = 0; i < numIdx; i++)
			idx.push_back(idxPtr[i]);
		mesh.indexBuffer->Unmap();
		RefCountedPtr<CollisionGeometry> cgeom(new CollisionGeometry(m_renderer, pos, idx, collFlag));
		return cgeom;
	}

	void Loader::ConvertNodes(aiNode *node, Group *_parent, std::vector<RefCountedPtr<StaticGeometry>> &geoms, const matrix4x4f &accum)
	{
		PROFILE_SCOPED()
		Group *parent = _parent;
		const std::string nodename(node->mName.C_Str());
		const aiMatrix4x4 &trans = node->mTransformation;
		matrix4x4f m = ConvertMatrix(trans);

		bool isLeafNode = node->mNumChildren == 0 && node->mNumMeshes == 0;

		if (m_modelFormat == ModelFormat::GLTF) {
			// Blender's GLTF exporter writes text nodes as mesh nodes,
			// so treat label_xxxx nodes as leaf nodes even though they have the text mesh present
			isLeafNode |= node->mNumChildren == 0 && starts_with(nodename, "label_");
		}

		//lights, and possibly other special nodes should be leaf nodes (without meshes)
		if (isLeafNode) {
			if (starts_with(nodename, "navlight_")) {
				CreateNavlight(nodename, accum * m);
			} else if (starts_with(nodename, "thruster_")) {
				CreateThruster(nodename, accum * m);
			} else if (starts_with(nodename, "label_")) {
				// labels point to +Z which matches Blender output to Collada but not GLTF (which has a correct node orientation)
				if (m_modelFormat == ModelFormat::GLTF)
					CreateLabel(nodename, parent, m * matrix4x4f::RotateXMatrix(M_PI_2));
				else
					CreateLabel(nodename, parent, m);
			} else if (starts_with(nodename, "tag_")) {
				m_model->AddTag(nodename, parent, new Tag(m_renderer, m));
			} else if (starts_with(nodename, "entrance_")) {
				m_model->AddTag(nodename, parent, new Tag(m_renderer, m));
			} else if (starts_with(nodename, "loc_")) {
				m_model->AddTag(nodename, parent, new Tag(m_renderer, m));
			} else if (starts_with(nodename, "exit_")) {
				m_model->AddTag(nodename, parent, new Tag(m_renderer, m));
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
			RefCountedPtr<CollisionGeometry> cgeom = CreateCollisionGeometry(geoms.at(node->mMeshes[0]), collflag);
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
				numDecal = atoi(nodename.substr(7, 1).c_str());
				if (numDecal > 4)
					throw LoadingError("More than 4 different decals");
			}

			for (unsigned int i = 0; i < node->mNumMeshes; i++) {
				RefCountedPtr<StaticGeometry> geom = geoms.at(node->mMeshes[i]);

				//handle special decal material
				//set special material for decals
				if (numDecal > 0) {
					geom->SetNodeMask(NODE_TRANSPARENT);
					geom->GetMeshAt(0).material = GetDecalMaterial(numDecal);
					geom->SetNodeFlags(geom->GetNodeFlags() | NODE_DECAL);
				}

				parent->AddChild(geom.Get());
			}
		}

		for (unsigned int i = 0; i < node->mNumChildren; i++) {
			aiNode *child = node->mChildren[i];
			ConvertNodes(child, parent, geoms, accum * m);
		}
	}

	void Loader::LoadCollision(const std::string &filename)
	{
		PROFILE_SCOPED()
		//Convert all found aiMeshes into a geomtree. Materials,
		//Animations and node structure can be ignored
		assert(m_model);

		Assimp::Importer importer;
		importer.SetIOHandler(new AssimpFileSystem(FileSystem::gameDataFiles));

		//discard extra data
		importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS,
			aiComponent_COLORS |
				aiComponent_TEXCOORDS |
				aiComponent_NORMALS |
				aiComponent_MATERIALS);
		const aiScene *scene = importer.ReadFile(
			filename,
			aiProcess_RemoveComponent |
				aiProcess_Triangulate |
				aiProcess_PreTransformVertices //"bake" transformations so we can disregard the structure
		);

		if (!scene)
			throw LoadingError("Could not load file");

		if (scene->mNumMeshes == 0)
			throw LoadingError("No geometry found");

		std::vector<Uint32> indices;
		std::vector<vector3f> vertices;
		Uint32 indexOffset = 0;

		for (Uint32 i = 0; i < scene->mNumMeshes; i++) {
			aiMesh *mesh = scene->mMeshes[i];

			//copy indices
			//we assume aiProcess_Triangulate does its job
			assert(mesh->mNumFaces > 0);
			for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
				const aiFace *face = &mesh->mFaces[f];
				for (unsigned int j = 0; j < face->mNumIndices; j++) {
					indices.push_back(indexOffset + face->mIndices[j]);
				}
			}
			indexOffset += mesh->mNumFaces * 3;

			//vertices
			for (unsigned int v = 0; v < mesh->mNumVertices; v++) {
				const aiVector3D &vtx = mesh->mVertices[v];
				vertices.push_back(vector3f(vtx.x, vtx.y, vtx.z));
			}
		}

		assert(!vertices.empty() && !indices.empty());

		//add pre-transformed geometry at the top level
		m_model->GetRoot()->AddChild(new CollisionGeometry(m_renderer, vertices, indices, 0));
	}

	unsigned int Loader::GetGeomFlagForNodeName(const std::string &nodename)
	{
		PROFILE_SCOPED()
		//special names after collision_
		if (nodename.length() > 10) {
			//landing pads
			if (nodename.length() >= 13 && std::string_view{ nodename }.substr(10, 3) == "pad") {
				return SceneGraph::CollisionGeometry::DOCKING;
			//entrance
			} else if (nodename.length() >= 14 && std::string_view{ nodename }.substr(10, 4) == "port") {
				return SceneGraph::CollisionGeometry::ENTRANCE;
			}
		}
		//anything else is static collision
		return 0x0;
	}

} // namespace SceneGraph
