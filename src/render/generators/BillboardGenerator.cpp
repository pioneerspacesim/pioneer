// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "BillboardGenerator.h"
#include "RenderSetup.h"
#include "SceneRenderer.h"

#include "graphics/Material.h"
#include "graphics/RenderState.h"
#include "graphics/Renderer.h"
#include "graphics/TextureBuilder.h"
#include "graphics/Types.h"
#include "graphics/VertexArray.h"

using namespace Render;

void BillboardGenerator::CacheShadersForPass(RenderPass *pass)
{
	Graphics::Renderer *r = m_sceneRenderer->GetRenderer();

	// TODO: material descriptor should be deprecated
	Graphics::MaterialDescriptor desc;
	desc.textures = 1;

	// TODO: RenderStateDesc should be specified in the shaderdef (potentially with multiple "techniques" per shaderdef)
	Graphics::RenderStateDesc rsd;
	rsd.blendMode = Graphics::BLEND_ALPHA_ONE;
	rsd.depthWrite = false;
	rsd.primitiveType = Graphics::POINTS;

	Graphics::Material *mat = r->CreateMaterial(pass->shader, desc, rsd);

	// TODO: allow this to be set from the render_setup file via a parameters block
	mat->SetTexture("texture0"_hash,
		Graphics::TextureBuilder::Billboard("textures/planet_billboard.dds").GetOrCreateTexture(r, "billboard"));

	m_sceneRenderer->CacheShaderForPass(pass, pass->shader, mat);
}

void BillboardGenerator::Run(RenderPass *pass, Space *space, const CameraContext *camera)
{
	Graphics::VertexArray billboards(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_NORMAL);

	for (const auto &attrs : m_sceneRenderer->GetSortedBodies()) {
		if (attrs.billboard) {
			billboards.Add(vector3f(attrs.viewCoords), vector3f(0.f, 0.f, attrs.billboardSize));
		}
	}

	if (!billboards.IsEmpty()) {
		Graphics::Renderer *r = m_sceneRenderer->GetRenderer();
		Graphics::Material *mat = m_sceneRenderer->GetShaderForPass(pass, pass->shader);

		r->SetTransform(matrix4x4f::Identity());
		r->DrawBuffer(&billboards, mat);
	}
}
