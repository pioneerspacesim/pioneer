// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "FullscreenGenerator.h"

#include "graphics/Material.h"
#include "graphics/RenderState.h"
#include "graphics/RenderTarget.h"
#include "graphics/Renderer.h"

#include "graphics/Types.h"
#include "graphics/VertexArray.h"
#include "render/RenderSetup.h"
#include "render/ResourceManager.h"
#include "render/SceneRenderer.h"

using namespace Render;

static void draw_fullscreen_quad(Graphics::Renderer *r, Graphics::Material *m)
{
	Graphics::VertexArray quad = Graphics::VertexArray(Graphics::ATTRIB_POSITION, 6);

	quad.Add(vector3f(-1, -1, 0));
	quad.Add(vector3f( 1, -1, 0));
	quad.Add(vector3f( 1,  1, 0));
	quad.Add(vector3f(-1, -1, 0));
	quad.Add(vector3f( 1,  1, 0));
	quad.Add(vector3f(-1,  1, 0));

	r->DrawBuffer(&quad, m);
}

void FullscreenGenerator::Run(RenderPass *pass, Space *space, const CameraContext *camera)
{
	Graphics::Material *mat = m_sceneRenderer->GetShaderForPass(pass, pass->shader);
	Graphics::RenderTarget *input = m_sceneRenderer->GetResourceManager()->GetRenderTarget(pass->input);

	mat->SetTexture("texture0"_hash, input->GetColorTexture());
	draw_fullscreen_quad(m_sceneRenderer->GetRenderer(), mat);
}

void FullscreenResolveGenerator::Run(RenderPass *pass, Space *space, const CameraContext *camera)
{
	Graphics::RenderTarget *input = m_sceneRenderer->GetResourceManager()->GetRenderTarget(pass->input);
	Graphics::RenderTarget *output = m_sceneRenderer->GetResourceManager()->GetRenderTarget(pass->render_target);

	Graphics::ViewportExtents ext = { 0, 0, input->GetDesc().width, input->GetDesc().height };

	if (input->GetDesc().numSamples != 0) {
		m_sceneRenderer->GetRenderer()->ResolveRenderTarget(input, output, ext);
	} else {
		m_sceneRenderer->GetRenderer()->CopyRenderTarget(input, output, ext, ext, false);
	}
}

void FullscreenDownsampleGenerator::CacheShadersForPass(RenderPass *pass)
{
	Graphics::Renderer *r = m_sceneRenderer->GetRenderer();

	Graphics::RenderStateDesc rsd = {};
	rsd.depthTest = false;
	rsd.depthWrite = false;

	Graphics::Material *downsample = r->CreateMaterial(pass->shader + "_downsample", {}, rsd);
	Graphics::Material *upsample = r->CreateMaterial(pass->shader + "_upsample", {}, rsd);

	m_sceneRenderer->CacheShaderForPass(pass, "downsample", downsample);
	m_sceneRenderer->CacheShaderForPass(pass, "upsample", upsample);
}

void FullscreenDownsampleGenerator::Run(RenderPass *pass, Space *space, const CameraContext *camera)
{
	Graphics::Renderer *r = m_sceneRenderer->GetRenderer();
	ResourceManager *rm = m_sceneRenderer->GetResourceManager();

	Graphics::Material *mat_downsample = m_sceneRenderer->GetShaderForPass(pass, "downsample");
	Graphics::Material *mat_upsample = m_sceneRenderer->GetShaderForPass(pass, "downsample");

	Graphics::RenderTarget *input = rm->GetRenderTarget(pass->input);
	Graphics::RenderTarget *output = rm->GetRenderTarget(pass->render_target);

	uint32_t width = input->GetDesc().width;
	uint32_t height = input->GetDesc().height;
	Graphics::TextureFormat outputFormat = output->GetDesc().colorFormat;

	uint32_t numDownsamples = pass->steps;
	std::vector<Graphics::RenderTarget *> tempRTs;

	// Downsample chain
	for (size_t i = 0; i < numDownsamples; i++) {
		// Allocate a temporary render target for this downsample step
		width /= 2;
		height /= 2;
		tempRTs.push_back(rm->GetTempRenderTarget(width, height, outputFormat, false));

		// Draw the downsample
		r->SetRenderTarget(tempRTs.back());
		r->SetViewport(Graphics::ViewportExtents(0, 0, width, height));
		mat_downsample->SetTexture("texture0"_hash, (i == 0 ? input : tempRTs[i - 1])->GetColorTexture());
		draw_fullscreen_quad(r, mat_downsample);
	}

	mat_downsample->SetTexture("texture0"_hash, nullptr);

	// Upsample chain
	for (int i = numDownsamples - 1; i >= 0; i--) {
		input = tempRTs.back();
		tempRTs.pop_back();

		Graphics::RenderTarget *rt = i == 0 ? output : tempRTs.back();
		r->SetRenderTarget(rt);
		r->SetViewport(Graphics::ViewportExtents(0, 0, rt->GetDesc().width, rt->GetDesc().height));

		mat_upsample->SetTexture("texture0"_hash, input->GetColorTexture());
		draw_fullscreen_quad(r, mat_upsample);

		// Release it back to the wild once we're done drawing from it
		rm->ReleaseTempRenderTarget(input);
	}

	mat_upsample->SetTexture("texture0"_hash, nullptr);

	// At this point we've fully blitted to the output texture and are done
}
