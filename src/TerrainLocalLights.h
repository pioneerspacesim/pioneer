// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _TERRAINLOCALLIGHTS_H
#define _TERRAINLOCALLIGHTS_H

#include "Color.h"

class Camera;
class Planet;
class Ship;

namespace Graphics {
	class Material;
}

class TerrainLocalLights {
public:
	static void UploadToMaterial(
		Graphics::Material *material,
		const Camera *camera,
		const Planet *planet,
		double planetRadius);

	// Additive RGB from this ship's nav/thruster lights, night-gated like terrain local lights.
	// Used to brighten the ship's own exhaust/dust particles.
	static Color4f CalcShipSelfIllumination(const Camera *camera, const Ship *ship);
};

#endif
