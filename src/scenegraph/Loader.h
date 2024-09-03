// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SCENEGRAPH_ASSIMPLOADER_H
#define _SCENEGRAPH_ASSIMPLOADER_H
/**
 * Model loader using Assimp
 */
#include "BaseLoader.h"
#include "CollisionGeometry.h"
#include "graphics/Material.h"

// Disable some GCC diagnostics errors.
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <assimp/types.h>
#pragma GCC diagnostic pop
#else
#include <assimp/types.h>
#endif

struct aiNode;
struct aiMesh;
struct aiScene;
struct aiNodeAnim;

namespace SceneGraph {

	class Loader : public BaseLoader {
	public:
		Loader(Graphics::Renderer *r, bool logWarnings = false, bool loadSGMfiles = true);

		//find & attempt to load a model, based on filename (without path or .model suffix)
		Model *LoadModel(const std::string &name);
		Model *LoadModel(const std::string &name, const std::string &basepath);

		const std::vector<std::string> &GetLogMessages() const { return m_logMessages; }

	protected:
		// store the format of the mesh file we're currently importing to
		// enable importer-specific quirks/workarounds
		enum class ModelFormat : uint8_t {
			UNKNOWN = 0,
			GLTF,
			COLLADA,
			WAVEFRONT
		};

		bool m_doLog;
		bool m_loadSGMs;
		bool m_mostDetailedLod;
		ModelFormat m_modelFormat;
		std::vector<std::string> m_logMessages;
		std::string m_curMeshDef; //for logging

		RefCountedPtr<Group> m_thrustersRoot;
		RefCountedPtr<Group> m_billboardsRoot;

		bool CheckKeysInRange(const aiNodeAnim *, double start, double end);
		matrix4x4f ConvertMatrix(const aiMatrix4x4 &) const;
		Model *CreateModel(ModelDefinition &def);
		RefCountedPtr<Node> LoadMesh(const std::string &filename, const std::vector<AnimDefinition> &animDefs); //load one mesh file so it can be added to the model scenegraph. Materials should be created before this!
		void AddLog(const std::string &);
		void CheckAnimationConflicts(const Animation *, const std::vector<Animation *> &); //detect animation overlap
		void ConvertAiMeshes(std::vector<RefCountedPtr<StaticGeometry>> &, const aiScene *); //model is only for material lookup
		void ConvertAnimations(const aiScene *, const std::vector<AnimDefinition> &, Node *meshRoot);
		void ConvertNodes(aiNode *node, Group *parent, std::vector<RefCountedPtr<StaticGeometry>> &meshes, const matrix4x4f &);
		void CreateLabel(const std::string &name, Group *parent, const matrix4x4f &);
		void CreateThruster(const std::string &name, const matrix4x4f &nodeTrans);
		void CreateNavlight(const std::string &name, const matrix4x4f &nodeTrans);
		RefCountedPtr<CollisionGeometry> CreateCollisionGeometry(RefCountedPtr<StaticGeometry>, unsigned int collFlag);
		void LoadCollision(const std::string &filename);

		unsigned int GetGeomFlagForNodeName(const std::string &);
	};

} // namespace SceneGraph
#endif
