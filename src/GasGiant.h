// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GASGIANT_H
#define _GASGIANT_H

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
class GasGiant;
class GasPatch;
class GasPatchContext;

#define NUM_PATCHES 6

class GasGiant : public BaseSphere {
public:
	GasGiant(const SystemBody *body);
	virtual ~GasGiant();

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
	void GenerateTexture();

	static RefCountedPtr<GasPatchContext> GasGiant::s_patchContext;

	// all variables for GetHeight(), GetColor()
	RefCountedPtr<Terrain> m_terrain;

	//std::unique_ptr<Graphics::Drawables::Sphere3D> m_baseCloudSurface;
	std::unique_ptr<GasPatch> m_patches[NUM_PATCHES];

	bool m_hasTempCampos;
	vector3d m_tempCampos;

	virtual void SetUpMaterials();
	Graphics::RenderState *m_surfRenderState;
	Graphics::RenderState *m_atmosRenderState;
	RefCountedPtr<Graphics::Texture> m_surfaceTexture;
	RefCountedPtr<Graphics::Material> m_surfaceMaterial;
	std::unique_ptr<Graphics::Material> m_atmosphereMaterial;
	//special parameters for shaders
	MaterialParameters m_materialParameters;
};

#endif /* _GASGIANT_H */
