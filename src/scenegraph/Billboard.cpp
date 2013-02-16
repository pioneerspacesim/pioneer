// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Billboard.h"
#include "NodeVisitor.h"
#include "graphics/Renderer.h"

namespace SceneGraph {

Billboard::Billboard(Graphics::Renderer *r, RefCountedPtr<Graphics::Material> mat, float size)
: Node(r, NODE_TRANSPARENT)
, m_size(size)
, m_material(mat)
{
}

Billboard::Billboard(Graphics::Renderer *r, const std::vector<vector3f> &pts, RefCountedPtr<Graphics::Material> mat, float size)
: Node(r, NODE_TRANSPARENT)
, m_size(size)
, m_material(mat)
, m_points(pts)
{
}

Billboard::Billboard(const Billboard &billboard)
: Node(billboard)
, m_size(billboard.m_size)
, m_material(billboard.m_material)
, m_points(billboard.m_points)
{
}

Node* Billboard::Clone()
{
	return new Billboard(*this);
}

void Billboard::Accept(NodeVisitor &nv)
{
	nv.ApplyBillboard(*this);
}

void Billboard::Render(const matrix4x4f &trans, RenderData *rd)
{
	Graphics::Renderer *r = GetRenderer();
	r->SetTransform(trans);
	r->SetBlendMode(Graphics::BLEND_ALPHA_ONE);
	r->DrawPointSprites(m_points.size(), &m_points[0], m_material.Get(), m_size);
}

void Billboard::AddPoint(const vector3f &pt)
{
	m_points.push_back(pt);
}

}
