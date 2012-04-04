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

void StaticGeometry::Accept(NodeVisitor &nv)
{
	nv.ApplyStaticGeometry(*this);
}

}