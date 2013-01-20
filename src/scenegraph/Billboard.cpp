// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Billboard.h"
#include "graphics/Renderer.h"

namespace SceneGraph {

Billboard::Billboard(RefCountedPtr<Graphics::Material> mat, float size)
: Node(NODE_TRANSPARENT)
, m_size(size)
, m_material(mat)
{
}

Billboard::Billboard(const std::vector<vector3f> &pts, RefCountedPtr<Graphics::Material> mat, float size)
: Node(NODE_TRANSPARENT)
, m_size(size)
, m_material(mat)
, m_points(pts)
{
}

void Billboard::Render(Graphics::Renderer *r, const matrix4x4f &trans, RenderData *rd)
{
	r->SetTransform(trans);
	r->SetBlendMode(Graphics::BLEND_ALPHA_ONE);
	r->DrawPointSprites(m_points.size(), &m_points[0], m_material.Get(), m_size);
}

void Billboard::AddPoint(const vector3f &pt)
{
	m_points.push_back(pt);
}

}
