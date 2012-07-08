#include "DrawVisitor.h"
#include "StaticGeometry.h"
#include "graphics/Renderer.h"

namespace Newmodel {

void DrawVisitor::ApplyStaticGeometry(StaticGeometry &g)
{
	m_renderer->DrawStaticMesh(g.GetMesh());
}

}
