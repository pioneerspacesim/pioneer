// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Billboard.h"

#include "Model.h"
#include "NodeVisitor.h"
#include "graphics/Graphics.h"
#include "graphics/Material.h"
#include "graphics/RenderState.h"
#include "graphics/Renderer.h"
#include "graphics/Stats.h"
#include "graphics/VertexArray.h"
#include "graphics/VertexBuffer.h"

namespace SceneGraph {

	Billboard::Billboard(Graphics::VertexArray &bbVA, Graphics::Renderer *r, float size) :
		Node(r, NODE_TRANSPARENT),
		m_bbVA(bbVA),
		m_size(size)
	{
	}

	Billboard::Billboard(const Billboard &billboard, NodeCopyCache *cache) :
		Node(billboard, cache),
		m_bbVA(billboard.m_bbVA),
		m_size(billboard.m_size)
	{
	}

	Node *Billboard::Clone(NodeCopyCache *cache)
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

		//some hand-tweaked scaling, to make the lights seem larger from distance (final size is in pixels)
		const float pixrad = Clamp(Graphics::GetScreenHeight() / trans.GetTranslate().Length(), 1.0f, 15.0f);
		const float size = (m_size * Graphics::GetFovFactor()) * pixrad;
		m_bbVA.Add(trans * vector3f(0.0f), vector3f(m_colorUVoffset, size));
	}

} // namespace SceneGraph
