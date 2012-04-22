#include "StaticGeometry.h"
#include "NodeVisitor.h"
#include "graphics/Renderer.h"

namespace Newmodel {

StaticGeometry::StaticGeometry() :
	Node()
{
	m_mesh.Reset(new Graphics::StaticMesh(Graphics::TRIANGLES));
}

StaticGeometry::~StaticGeometry()
{
}

void StaticGeometry::Accept(NodeVisitor &nv)
{
	nv.ApplyStaticGeometry(*this);
}

void StaticGeometry::Render(Graphics::Renderer *r, const matrix4x4f &trans)
{
	r->DrawStaticMesh(GetMesh());
}

}