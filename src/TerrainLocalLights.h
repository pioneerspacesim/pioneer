// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _TERRAINLOCALLIGHTS_H
#define _TERRAINLOCALLIGHTS_H

class Camera;
class Planet;

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
};

#endif
