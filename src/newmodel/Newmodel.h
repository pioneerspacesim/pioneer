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
 * Animation: to be specified. Something to do with bones (even if meshes are rigid)
 */
#include "Model.h"
#include "Group.h"

namespace Graphics { class Renderer; }

namespace Newmodel
{

class NModel : public Model
{
public:
	NModel();
	~NModel();
	float GetDrawClipRadius() const { return 10.f; }
	//Render begins the graph traversal. Only geometry nodes actually render something.
	//might be worthwhile to implement Visitors (DrawVisitor, CreateCollisionMeshVisitor)
	void Render(Graphics::Renderer *r, const matrix4x4f &trans, const LmrObjParams *params);
	CollMesh *CreateCollisionMesh(const LmrObjParams *p) { return 0; }
	RefCountedPtr<Group> m_root;
	//materials used in the nodes should be accessible from here for convenience
};

}

#endif
