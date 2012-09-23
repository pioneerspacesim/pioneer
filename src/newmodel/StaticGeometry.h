#ifndef _STATICGEOMETRY_H
#define _STATICGEOMETRY_H
/*
 * Geometry node containing one StaticMesh. Nothing fancy.
 */
#include "Node.h"
#include "Aabb.h"
#include "graphics/Renderer.h"
#include "graphics/StaticMesh.h"
#include "SmartPtr.h"

namespace Newmodel {

class NodeVisitor;

class StaticGeometry : public Node
{
public:
	StaticGeometry();
	virtual const char *GetTypeName() { return "StaticGeometry"; }
	virtual void Accept(NodeVisitor &nv);
	virtual void Render(Graphics::Renderer *r, const matrix4x4f &trans, RenderData *rd);
	void AddMesh(RefCountedPtr<Graphics::StaticMesh>);
	Aabb m_boundingBox;
	Graphics::BlendMode m_blendMode;

protected:
	~StaticGeometry();
	void DrawBoundingBox(Graphics::Renderer *r, const Aabb &bb);
	std::vector<RefCountedPtr<Graphics::StaticMesh> > m_meshes;
	typedef std::vector<RefCountedPtr<Graphics::StaticMesh> > MeshContainer;
};

}
#endif
