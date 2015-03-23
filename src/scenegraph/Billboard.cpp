// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Billboard.h"
#include "NodeVisitor.h"
#include "graphics/Graphics.h"
#include "graphics/Renderer.h"
#include "graphics/VertexArray.h"
#include "graphics/VertexBuffer.h"
#include "graphics/Material.h"
#include "graphics/RenderState.h"
#include "graphics/Stats.h"

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
	PROFILE_SCOPED()
	Graphics::Renderer *r = GetRenderer();

	Graphics::VertexArray va(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_UV0, 6);

	const matrix3x3f rot = trans.GetOrient().Transpose();

	//some hand-tweaked scaling, to make the lights seem larger from distance
	const float size = m_size * Graphics::GetFovFactor() * Clamp(trans.GetTranslate().Length() / 500.f, 0.25f, 15.f);

	const vector3f rotv1 = rot * vector3f(size*0.5f, -size*0.5f, 0.0f);
	const vector3f rotv2 = rot * vector3f(size*0.5f, size*0.5f, 0.0f);

	va.Add(m_offset-rotv1, vector2f(0.f, 0.f)); //top left
	va.Add(m_offset-rotv2, vector2f(0.f, 1.f)); //bottom left
	va.Add(m_offset+rotv2, vector2f(1.f, 0.f)); //top right

	va.Add(m_offset+rotv2, vector2f(1.f, 0.f)); //top right
	va.Add(m_offset-rotv2, vector2f(0.f, 1.f)); //bottom left
	va.Add(m_offset+rotv1, vector2f(1.f, 1.f)); //bottom right

	if( !m_vbuffer.Valid() )
	{
		//create buffer and upload data
		Graphics::VertexBufferDesc vbd;
		vbd.attrib[0].semantic = Graphics::ATTRIB_POSITION;
		vbd.attrib[0].format   = Graphics::ATTRIB_FORMAT_FLOAT3;
		vbd.attrib[1].semantic = Graphics::ATTRIB_UV0;
		vbd.attrib[1].format   = Graphics::ATTRIB_FORMAT_FLOAT2;
		vbd.numVertices = va.GetNumVerts();
		vbd.usage = Graphics::BUFFER_USAGE_DYNAMIC;	// we could be updating this per-frame
		m_vbuffer.Reset( r->CreateVertexBuffer(vbd) );
	}
	m_vbuffer->Populate( va );

	r->SetTransform(trans);
	r->DrawBuffer(m_vbuffer.Get(), m_renderState, m_material.Get());

	r->GetStats().AddToStatCount(Graphics::Stats::STAT_BILLBOARD, 1);
}

}
