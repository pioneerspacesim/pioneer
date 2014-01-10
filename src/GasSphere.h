// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GASSPHERE_H
#define _GASSPHERE_H

#include <SDL_stdinc.h>

#include "vector3.h"
#include "Random.h"
#include "Camera.h"
#include "graphics/Drawables.h"
#include "graphics/Material.h"
#include "terrain/Terrain.h"
#include "BaseSphere.h"

#include <deque>

namespace Graphics { class Renderer; }
class SystemBody;
class GasSphere;
class GasPatch;
class GasPatchContext;

#define NUM_PATCHES 6

class GasSphere : public BaseSphere {
public:
	GasSphere(const SystemBody *body);
	virtual ~GasSphere();

	virtual void Update() {};
	virtual void Render(Graphics::Renderer *renderer, const matrix4x4d &modelView, vector3d campos, const float radius, const float scale, const std::vector<Camera::Shadow> &shadows);

	virtual double GetHeight(const vector3d &p) const { return 0.0; }

	// in sbody radii
	virtual double GetMaxFeatureHeight() const { return 0.0; }

	struct MaterialParameters {
		SystemBody::AtmosphereParameters atmosphere;
		std::vector<Camera::Shadow> shadows;
	};

	virtual void Reset() {};

private:
	void BuildFirstPatches();

	static RefCountedPtr<GasPatchContext> GasSphere::s_patchContext;

	//std::unique_ptr<Graphics::Drawables::Sphere3D> m_baseCloudSurface;
	std::unique_ptr<GasPatch> m_patches[NUM_PATCHES];

	bool m_hasTempCampos;
	vector3d m_tempCampos;

	virtual void SetUpMaterials();
	RefCountedPtr<Graphics::Material> m_surfaceMaterial;
	std::unique_ptr<Graphics::Material> m_atmosphereMaterial;
	//special parameters for shaders
	MaterialParameters m_materialParameters;
};

#endif /* _GEOSPHERE_H */
