#include "Billboard.h"

namespace Newmodel {

Billboard::Billboard(const std::vector<vector3f> &pts, RefCountedPtr<Graphics::Material> mat, float size)
: Node()
, m_points(pts)
, m_material(mat)
, m_size(size)
{
}

void Billboard::Render(Graphics::Renderer *r, const matrix4x4f &trans, RenderData *rd)
{
	r->SetTransform(trans);
	r->DrawPointSprites(m_points.size(), &m_points[0], m_material.Get(), m_size);
}

}
