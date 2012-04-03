#include "StaticGeometry.h"
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

}