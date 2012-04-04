#include "StaticGeometry.h"
#include "NodeVisitor.h"
#include "graphics/Renderer.h"

namespace Newmodel {

StaticGeometry::StaticGeometry()
{
	m_mesh.Reset(new Graphics::StaticMesh(Graphics::TRIANGLES));
}

StaticGeometry::~StaticGeometry()
{
}

void StaticGeometry::Render(Graphics::Renderer *renderer)
{
	renderer->DrawStaticMesh(m_mesh.Get());
}

void StaticGeometry::Accept(NodeVisitor &nv)
{
	nv.ApplyStaticGeometry(*this);
}

}