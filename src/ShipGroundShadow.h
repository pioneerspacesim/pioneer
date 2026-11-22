// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SHIP_GROUND_SHADOW_H
#define _SHIP_GROUND_SHADOW_H

#include "vector3.h"

#include <vector>

class Camera;
class Planet;
class Ship;

namespace Graphics {
	class RenderTarget;
	class Renderer;
} // namespace Graphics

struct ShipGroundShadowPlane {
	vector3d point;
	vector3d normal;
	vector3d projectDir;
	float strength = 1.f; // angle-based fade, 0-1
};

class ShipGroundShadow {
public:
	static void Init(Graphics::Renderer *renderer);
	static void Uninit();

	static bool ShouldCastGroundShadow(const Ship *ship, const Planet *planet);

	static bool TryBuildShadowPlane(const Camera *camera, const Ship *ship, const Planet *planet, ShipGroundShadowPlane &planeOut);

	static void RenderPass(
		Graphics::Renderer *renderer,
		const Camera *camera,
		Graphics::RenderTarget *mainRenderTarget,
		const std::vector<const Ship *> &ships);

private:
	static void EnsureResources(Graphics::Renderer *renderer, int width, int height);
};

#endif
