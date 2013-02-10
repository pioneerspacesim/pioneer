// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Billboard.h"
#include "NodeVisitor.h"
#include "graphics/Renderer.h"
#include "graphics/VertexArray.h"

namespace SceneGraph {

Billboard::Billboard(Graphics::Renderer *r, RefCountedPtr<Graphics::Material> mat, const vector3f &offset, float size)
: Node(r, NODE_TRANSPARENT)
, m_size(size)
, m_material(mat)
, m_offset(offset)
{
}

Billboard::Billboard(const Billboard &billboard)
: Node(billboard)
, m_size(billboard.m_size)
, m_material(billboard.m_material)
, m_offset(billboard.m_offset)
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

	Graphics::VertexArray va(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_UV0, 6);

	const matrix3x3f rot = trans.GetOrient().Transpose();

	const vector3f rotv1 = rot * vector3f(m_size/2.f, -m_size/2.f, 0.0f);
	const vector3f rotv2 = rot * vector3f(m_size/2.f, m_size/2.f, 0.0f);

	va.Add(m_offset-rotv1, vector2f(0.f, 0.f)); //top left
	va.Add(m_offset-rotv2, vector2f(0.f, 1.f)); //bottom left
	va.Add(m_offset+rotv2, vector2f(1.f, 0.f)); //top right

	va.Add(m_offset+rotv2, vector2f(1.f, 0.f)); //top right
	va.Add(m_offset-rotv2, vector2f(0.f, 1.f)); //bottom left
	va.Add(m_offset+rotv1, vector2f(1.f, 1.f)); //bottom right

	r->SetTransform(trans);
	r->SetBlendMode(Graphics::BLEND_ADDITIVE);
	r->SetDepthWrite(false);
	r->DrawTriangles(&va, m_material.Get());
	r->SetBlendMode(Graphics::BLEND_SOLID);
	r->SetDepthWrite(true);
}

}
