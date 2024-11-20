// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "BackgroundGenerator.h"

#include "Camera.h"
#include "Frame.h"
#include "MathUtil.h"
#include "Planet.h"
#include "Space.h"

// FIXME: camera should have a reference to its owning body
#include "Game.h"
#include "Pi.h"
#include "Player.h"
#include "SceneRenderer.h"

using namespace Render;

void BackgroundGenerator::Run(RenderPass *pass, Space *space, const CameraContext *camera)
{
	Frame *camFrame = Frame::GetFrame(camera->GetTempFrame());
	Frame *camParent = Frame::GetFrame(camFrame->GetParent());

	const std::vector<SceneRenderer::Light> &lightSources = m_sceneRenderer->GetLights();

	//fade space background based on atmosphere thickness and light angle
	float bgIntensity = 1.f;

	if (camParent && camParent->IsRotFrame()) {
		//check if camera is near a planet
		Body *camParentBody = camParent->GetBody();

		if (camParentBody && camParentBody->IsType(ObjectType::PLANET)) {
			Planet *planet = static_cast<Planet *>(camParentBody);

			const vector3f relpos(planet->GetInterpPositionRelTo(camera->GetTempFrame()));
			double altitude(relpos.Length());
			double pressure, density;

			planet->GetAtmosphericState(altitude, &pressure, &density);

			if (pressure >= 0.001) {
				//go through all lights to calculate something resembling light intensity
				float intensity = 0.f;
				// FIXME: camera should have a reference to its owning body
				const Body *pBody = Pi::game->GetPlayer();
				for (Uint32 i = 0; i < lightSources.size(); i++) {
					// Set up data for eclipses. All bodies are assumed to be spheres.
					const SceneRenderer::Light &ls = lightSources[i];
					const vector3f lightDir(ls.light.GetPosition().Normalized());
					intensity += Camera::ShadowedIntensity(ls.body, pBody) * std::max(0.f, lightDir.Dot(-relpos.Normalized())) * (ls.light.GetDiffuse().GetLuminance() / 255.0f);
				}
				intensity = Clamp(intensity, 0.0f, 1.0f);

				//calculate background intensity with some hand-tweaked fuzz applied
				bgIntensity = Clamp(1.f - std::min(1.f, powf(density, 0.25f)) * (0.3f + powf(intensity, 0.25f)), 0.f, 1.f);
			}
		}
	}

	matrix4x4d trans2bg;
	Frame::GetFrameTransform(space->GetRootFrame(), camera->GetTempFrame(), trans2bg);
	trans2bg.ClearToRotOnly();

	space->GetBackground()->SetIntensity(bgIntensity);
	space->GetBackground()->Draw(trans2bg);
}
