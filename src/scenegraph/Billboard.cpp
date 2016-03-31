// Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Billboard.h"
#include "Model.h"
#include "NodeVisitor.h"
#include "graphics/Graphics.h"
#include "graphics/Renderer.h"
#include "graphics/VertexArray.h"
#include "graphics/VertexBuffer.h"
#include "graphics/Material.h"
#include "graphics/RenderState.h"
#include "graphics/Stats.h"

namespace SceneGraph {

Billboard::Billboard(SceneGraph::Model *model, Graphics::Renderer *r, float size)
: Node(r, NODE_TRANSPARENT)
, m_model(model)
, m_size(size)
{
}

Billboard::Billboard(const Billboard &billboard, NodeCopyCache *cache)
: Node(billboard, cache)
, m_model(billboard.m_model)
, m_size(billboard.m_size)
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

	Graphics::VertexArray& bbVA = m_model->GetBillboardVA();
	const matrix3x3f rot = trans.GetOrient().Transpose();

	//some hand-tweaked scaling, to make the lights seem larger from distance
	const float size = m_size * Graphics::GetFovFactor() * Clamp(trans.GetTranslate().Length() / 500.f, 0.25f, 15.f);

	const vector3f rotv1 = rot * vector3f(size*0.5f, -size*0.5f, 0.0f);
	const vector3f rotv2 = rot * vector3f(size*0.5f, size*0.5f, 0.0f);
	bbVA.Add(trans * (-rotv1), m_color, vector2f(0.f, 0.f)); //top left
	bbVA.Add(trans * (-rotv2), m_color, vector2f(0.f, 1.f)); //bottom left
	bbVA.Add(trans * ( rotv2), m_color, vector2f(1.f, 0.f)); //top right
	bbVA.Add(trans * ( rotv2), m_color, vector2f(1.f, 0.f)); //top right
	bbVA.Add(trans * (-rotv2), m_color, vector2f(0.f, 1.f)); //bottom left
	bbVA.Add(trans * ( rotv1), m_color, vector2f(1.f, 1.f)); //bottom right
}

}
