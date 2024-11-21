// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LightingGenerator.h"

#include "Camera.h"
#include "Frame.h"
#include "Space.h"

#include "graphics/Renderer.h"
#include "render/SceneRenderer.h"

#include "profiler/Profiler.h"

using namespace Render;

static void position_system_lights(Frame *camFrame, Frame *frame, std::vector<SceneRenderer::Light> &lights)
{
	PROFILE_SCOPED()
	if (lights.size() > 3) return;

	SystemBody *body = frame->GetSystemBody();
	// IsRotFrame check prevents double counting
	if (body && !frame->IsRotFrame() && (body->GetSuperType() == SystemBody::SUPERTYPE_STAR)) {
		vector3d lpos = frame->GetPositionRelTo(camFrame->GetId());

		const Color &col = StarSystem::starRealColors[body->GetType()];

		const Color lightCol(col[0], col[1], col[2], 0);
		vector3f lightpos(lpos.x, lpos.y, lpos.z);
		Graphics::Light light(Graphics::Light::LIGHT_DIRECTIONAL, lightpos, lightCol, lightCol);
		lights.push_back({ light, frame->GetBody() });
	}

	for (FrameId kid : frame->GetChildren()) {
		Frame *kid_f = Frame::GetFrame(kid);
		position_system_lights(camFrame, kid_f, lights);
	}
}

void LightingGenerator::Run(RenderPass *pass, Space *space, const CameraContext *camera)
{
	Frame *camFrame = Frame::GetFrame(camera->GetTempFrame());
	Frame *rootFrame = Frame::GetFrame(space->GetRootFrame());

	std::vector<SceneRenderer::Light> &lightSources = m_sceneRenderer->GetLights();

	// Pick up to four suitable system light sources (stars)
	lightSources.clear();
	lightSources.reserve(4);
	position_system_lights(camFrame, rootFrame, lightSources);

	if (lightSources.empty()) {
		// no lights means we're somewhere weird (eg hyperspace). fake one
		Graphics::Light light(Graphics::Light::LIGHT_DIRECTIONAL, vector3f(0.f), Color::WHITE, Color::WHITE);
		lightSources.push_back({ light, 0 });
	}

	// Setup lighting data for further render passes
	// TODO: define the lighting buffer as an output in the render_setup file

	{
		std::vector<Graphics::Light> rendererLights;
		rendererLights.reserve(lightSources.size());
		for (size_t i = 0; i < lightSources.size(); i++)
			rendererLights.push_back(lightSources[i].light);

		m_sceneRenderer->GetRenderer()->SetLights(rendererLights.size(), &rendererLights[0]);
	}
}
