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
	const vector3f verts[] = {
		vector3f(0.f, 0.f, 0.f),
		vector3f(100.f, 0.f, 0.f),
		vector3f(100.f, 100.f, 0.f)
	};
	renderer->DrawLines(3, &verts[0], Color(1.f, 0.f, 0.f), Graphics::LINE_LOOP);
	//renderer->DrawStaticMesh(m_mesh.Get());
}

}