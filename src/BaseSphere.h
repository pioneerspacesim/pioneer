// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _BASESPHERE_H
#define _BASESPHERE_H

#include <SDL_stdinc.h>

#include "vector3.h"
#include "Camera.h"
#include "galaxy/StarSystem.h"
#include "terrain/Terrain.h"

#include <deque>

namespace Graphics { 
	class Renderer; 
	class RenderState;
	class Material;
}
class SystemBody;

class BaseSphere {
public:
	BaseSphere(const SystemBody *body);
	virtual ~BaseSphere();

	virtual void Update()=0;
	virtual void Render(Graphics::Renderer *renderer, const matrix4x4d &modelView, vector3d campos, const float radius, const float scale, const std::vector<Camera::Shadow> &shadows)=0;

	virtual double GetHeight(const vector3d &p) const { return 0.0; }

	static void Init();
	static void Uninit();
	static void UpdateAllBaseSphereDerivatives();
	static void OnChangeDetailLevel();
	static void DrawAtmosphereSurface(Graphics::Renderer *renderer,
		const matrix4x4d &modelView, const vector3d &campos, float rad,
		Graphics::RenderState *rs, Graphics::Material *mat);

	// in sbody radii
	virtual double GetMaxFeatureHeight() const { return 0.0; }

	struct MaterialParameters {
		SystemBody::AtmosphereParameters atmosphere;
		std::vector<Camera::Shadow> shadows;
	};

	virtual void Reset()=0;

	const SystemBody *GetSystemBody() const { return m_sbody; }
	Terrain* GetTerrain() const { return m_terrain.Get(); }

	Graphics::RenderState* GetSurfRenderState() const { return m_surfRenderState; }
	Graphics::Material* GetSurfaceMaterial() const { return m_surfaceMaterial.get(); }

protected:
	const SystemBody *m_sbody;

	// all variables for GetHeight(), GetColor()
	RefCountedPtr<Terrain> m_terrain;

	virtual void SetUpMaterials()=0;

	Graphics::RenderState *m_surfRenderState;
	Graphics::RenderState *m_atmosRenderState;
	std::unique_ptr<Graphics::Material> m_surfaceMaterial;
	std::unique_ptr<Graphics::Material> m_atmosphereMaterial;

	//special parameters for shaders
	MaterialParameters m_materialParameters;
};

#endif /* _GEOSPHERE_H */
