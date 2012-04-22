#ifndef _NEWMODEL_H
#define _NEWMODEL_H
/*
 * New internal model structure. Think of this as a mini-scenegraph.
 * For example:
 * RootNode
 *    MatrixTransformNode (applies a scale or something to child nodes)
 *        LodSwitchNode (picks 1-3)
 *            StaticGeometry_low
 *            StaticGeometry_med
 *            StaticGeometry_hi
 *
 * It's not supposed to be too complex. For example there are no "Material" nodes.
 * Geometry nodes can contain multiple separate meshes.
 * Animation: To Be Specified. Something to do with bones (even if meshes are rigid).
 * 		animated geometries are likely smaller, separate nodes so it's not necessary
 * 		to avoid software skinning huge models
 * Attaching models to other models (guns etc. to ships): TBS
 * Space stations might be "Scenes" consisting of multiple models. And lights and stuff.
 */
#include "Model.h"
#include "Group.h"
#include "libs.h"
#include <stdexcept>
#include "graphics/Material.h"

namespace Graphics { class Renderer; }

namespace Newmodel
{

struct LoadingError : public std::runtime_error {
	LoadingError() : std::runtime_error("NewModel::LoadingError") { }
};

typedef std::vector<std::pair<std::string, RefCountedPtr<Graphics::Material> > > MaterialContainer;

class NModel : public Model
{
public:
	friend class Loader;
	NModel(const std::string &name);
	~NModel();
	float GetDrawClipRadius() const { return m_boundingRadius; }
	//Render begins the graph traversal. Only geometry nodes actually render something.
	//might be worthwhile to implement Visitors (DrawVisitor, CreateCollisionMeshVisitor)
	void Render(Graphics::Renderer *r, const matrix4x4f &trans, const LmrObjParams *params);
	CollMesh *CreateCollisionMesh(const LmrObjParams *p);
	RefCountedPtr<Group> GetRoot() { return m_root; }
	RefCountedPtr<Group> m_root;
	//materials used in the nodes should be accessible from here for convenience
	RefCountedPtr<Graphics::Material> GetMaterialByName(const std::string &name) const;
	RefCountedPtr<Graphics::Material> GetMaterialByIndex(int) const;
private:
	std::string m_name;
	//materials are shared throughout the model graph
	MaterialContainer m_materials;
	float m_boundingRadius;
};

}

#endif
