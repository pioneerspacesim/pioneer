// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GASSPHERE_H
#define _GASSPHERE_H

#include <SDL_stdinc.h>

#include "vector3.h"
#include "Random.h"
#include "Camera.h"
#include "galaxy/StarSystem.h"
#include "graphics/Drawables.h"
#include "graphics/Material.h"
#include "terrain/Terrain.h"
#include "GeoPatchID.h"

#include <deque>

namespace Graphics { class Renderer; }
class SystemBody;
class GeoPatch;
class GeoPatchContext;
class GasSphere;
class SQuadSplitRequest;
class SQuadSplitResult;
class SSingleSplitResult;

#define NUM_PATCHES 6

class GasSphere {
public:
	GasSphere(const SystemBody *body);
	~GasSphere();

	void Update();
	void Render(Graphics::Renderer *renderer, const matrix4x4d &modelView, vector3d campos, const float radius, const float scale, const std::vector<Camera::Shadow> &shadows);

	inline double GetHeight(const vector3d &p) const {
		return 0.0;
	}
	static void Init();
	static void Uninit();
	static void UpdateAllGasSpheres();
	static void OnChangeDetailLevel();
	// in sbody radii
	double GetMaxFeatureHeight() const { return 0.0; }

	struct MaterialParameters {
		SystemBody::AtmosphereParameters atmosphere;
		std::vector<Camera::Shadow> shadows;
	};

	void Reset();

private:
	const SystemBody *m_sbody;

	std::unique_ptr<Graphics::Drawables::Sphere3D> m_baseCloudSurface;

	// all variables for GetHeight(), GetColor()
	RefCountedPtr<Terrain> m_terrain;

	bool m_hasTempCampos;
	vector3d m_tempCampos;

	void SetUpMaterials();
	RefCountedPtr<Graphics::Material> m_surfaceMaterial;
	std::unique_ptr<Graphics::Material> m_atmosphereMaterial;
	//special parameters for shaders
	MaterialParameters m_materialParameters;
};

#endif /* _GEOSPHERE_H */
