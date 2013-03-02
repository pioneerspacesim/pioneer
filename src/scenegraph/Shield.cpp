// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Shield.h"
#include "NodeVisitor.h"
#include "graphics/Renderer.h"
#include "graphics/VertexArray.h"
#include "graphics/Material.h"
#include "graphics/TextureBuilder.h"

namespace SceneGraph {

Shield::Shield(Graphics::Renderer *r)
: StaticGeometry(r)
, visible(false)
{
}

Shield::Shield(const Shield &shield, NodeCopyCache *cache)
: StaticGeometry(shield, cache)
, visible(shield.visible)
{
}

Node* Shield::Clone(NodeCopyCache *cache)
{
	return this; //shields are shared
}

void Shield::Accept(NodeVisitor &nv)
{
	nv.ApplyShield(*this);
}

void Shield::Render(const matrix4x4f &trans, RenderData *rd)
{
	if (!rd->shieldVisible) return;

	m_params.strength = rd->shieldStrength;
	if (m_mat.Valid()) {
		m_mat->specialParameter0 = &m_params;
	} else {
		m_mat.Reset((*m_meshes.begin())->GetSurface(0)->GetMaterial().Get());
		assert(m_mat.Valid());
		m_mat->specialParameter0 = &m_params;
	}

	Graphics::Renderer *r = GetRenderer();
	r->SetTransform(trans);
	for (MeshContainer::iterator it = m_meshes.begin(), endIt = m_meshes.end(); it != endIt; ++it) {
		r->DrawStaticMesh(it->Get());
	}
}

}
