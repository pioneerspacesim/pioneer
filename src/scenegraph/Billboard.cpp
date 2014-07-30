// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Billboard.h"
#include "NodeVisitor.h"
#include "graphics/Graphics.h"
#include "graphics/Renderer.h"
#include "graphics/VertexArray.h"

namespace SceneGraph {

Billboard::Billboard(Graphics::Renderer *r, RefCountedPtr<Graphics::Material> mat, const vector3f &offset, float size)
: Node(r, NODE_TRANSPARENT)
, m_size(size)
, m_material(mat)
, m_offset(offset)
{
	Graphics::RenderStateDesc rsd;
	rsd.blendMode = Graphics::BLEND_ADDITIVE;
	rsd.depthWrite = false;
	m_renderState = r->CreateRenderState(rsd);
}

Billboard::Billboard(const Billboard &billboard, NodeCopyCache *cache)
: Node(billboard, cache)
, m_size(billboard.m_size)
, m_material(billboard.m_material)
, m_renderState(billboard.m_renderState)
, m_offset(billboard.m_offset)
{
}

Node* Billboard::Clone(NodeCopyCache *cache)
{
	return new Billboard(*this, cache);
}

void Billboard::Accept(NodeVisitor &nv)
{
	nv.ApplyBillboard(*this);
}

void Billboard::Render(const matrix4x4f &trans, const RenderData *rd)
{
	Graphics::Renderer *r = GetRenderer();

	Graphics::VertexArray va(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_UV0, 6);

	const matrix3x3f rot = trans.GetOrient().Transpose();

	//some hand-tweaked scaling, to make the lights seem larger from distance
	const float size = m_size * Graphics::GetFovFactor() * Clamp(trans.GetTranslate().Length() / 500.f, 0.25f, 15.f);

	const vector3f rotv1 = rot * vector3f(size/2.f, -size/2.f, 0.0f);
	const vector3f rotv2 = rot * vector3f(size/2.f, size/2.f, 0.0f);

	va.Add(m_offset-rotv1, vector2f(0.f, 0.f)); //top left
	va.Add(m_offset-rotv2, vector2f(0.f, 1.f)); //bottom left
	va.Add(m_offset+rotv2, vector2f(1.f, 0.f)); //top right

	va.Add(m_offset+rotv2, vector2f(1.f, 0.f)); //top right
	va.Add(m_offset-rotv2, vector2f(0.f, 1.f)); //bottom left
	va.Add(m_offset+rotv1, vector2f(1.f, 1.f)); //bottom right

	r->SetTransform(trans);
	r->DrawTriangles(&va, m_renderState, m_material.Get());
}

}
