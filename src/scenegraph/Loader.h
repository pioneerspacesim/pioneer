// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LOADER_H
#define _LOADER_H
/*
 * Load models using the model config files under data/newmodels/
 *  .model files are simple text files
 *  they are read into definition structures using a crummy parser, and
 *  then a scenegraph can be created with meshes loaded by assimp.
 */
#include "libs.h"
#include "Model.h"
#include "LoaderDefinitions.h"
#include "graphics/Material.h"
#include "graphics/Surface.h"
#include "text/DistanceFieldFont.h"
#include <assimp/types.h>

struct aiNode;
struct aiMesh;
struct aiScene;
struct aiNodeAnim;

namespace Graphics { class Renderer; }

namespace SceneGraph {

class StaticGeometry;

class Loader {
public:
	Loader(Graphics::Renderer *r, bool logWarnings = false);
	~Loader();
	//find & attempt to load a model, based on filename (without path or .model suffix)
	Model *LoadModel(const std::string &name);
	Model *LoadModel(const std::string &name, const std::string &basepath);

	const std::vector<std::string> &GetLogMessages() const { return m_logMessages; }

private:
	Graphics::Renderer *m_renderer;
	Model *m_model;
	bool m_doLog;
	bool m_mostDetailedLod;
	RefCountedPtr<Text::DistanceFieldFont> m_labelFont;
	std::string m_curPath;
	std::vector<std::string> m_logMessages;
	std::string m_curMeshDef; //for logging

	RefCountedPtr<Group> m_thrustersRoot;
	RefCountedPtr<Group> m_billboardsRoot;

	bool CheckKeysInRange(const aiNodeAnim *, double start, double end);
	matrix4x4f ConvertMatrix(const aiMatrix4x4&) const;
	Model *CreateModel(ModelDefinition &def);
	RefCountedPtr<Graphics::Material> GetDecalMaterial(unsigned int index);
	RefCountedPtr<Node> LoadMesh(const std::string &filename, const AnimList &animDefs); //load one mesh file so it can be added to the model scenegraph. Materials should be created before this!
	void AddLog(const std::string&);
	void CheckAnimationConflicts(const Animation*, const std::vector<Animation*>&); //detect animation overlap
	void ConvertAiMeshes(std::vector<RefCountedPtr<StaticGeometry> >&, const aiScene*); //model is only for material lookup
	void ConvertAnimations(const aiScene *, const AnimList &, Node *meshRoot);
	void ConvertNodes(aiNode *node, Group *parent, std::vector<RefCountedPtr<StaticGeometry> >& meshes, const matrix4x4f&);
	void CreateLabel(Group *parent, const matrix4x4f&);
	void CreateThruster(const std::string &name, const matrix4x4f& nodeTrans);
	void CreateNavlight(const std::string &name, const matrix4x4f& nodeTrans);
	void FindPatterns(PatternContainer &output); //find pattern texture files from the model directory
	void LoadCollision(const std::string &filename);

	unsigned int GetGeomFlagForNodeName(const std::string&);
};

}
#endif
