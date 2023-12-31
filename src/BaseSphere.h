// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _BASESPHERE_H
#define _BASESPHERE_H

#include "Camera.h"
#include "galaxy/AtmosphereParameters.h"
#include "graphics/Material.h"
#include "terrain/Terrain.h"
#include "vector3.h"

namespace Graphics {
	class Renderer;
	class RenderState;
	namespace Drawables {
		class Sphere3D;
	}
} // namespace Graphics

class BaseSphere {
public:
	BaseSphere(const SystemBody *body);
	virtual ~BaseSphere();

	virtual void Update() = 0;
	virtual void Render(Graphics::Renderer *renderer, const matrix4x4d &modelView, vector3d campos, const float radius, const std::vector<Camera::Shadow> &shadows) = 0;

	virtual double GetHeight(const vector3d &p) const { return 0.0; }

	static void Init();
	static void Uninit();
	static void UpdateAllBaseSphereDerivatives();
	static void OnChangeDetailLevel();

	void DrawAtmosphereSurface(Graphics::Renderer *renderer,
		const matrix4x4d &modelView, const vector3d &campos, float rad,
		RefCountedPtr<Graphics::Material> mat);

	// in sbody radii
	virtual double GetMaxFeatureHeight() const = 0;

	virtual void Reset() = 0;

	const SystemBody *GetSystemBody() const { return m_sbody; }
	Terrain *GetTerrain() const { return m_terrain.Get(); }

	RefCountedPtr<Graphics::Material> GetSurfaceMaterial() const { return m_surfaceMaterial; }

protected:
	const SystemBody *m_sbody;

	// all variables for GetHeight(), GetColor()
	RefCountedPtr<Terrain> m_terrain;

	virtual void SetUpMaterials() = 0;

	RefCountedPtr<Graphics::Material> m_surfaceMaterial;
	RefCountedPtr<Graphics::Material> m_atmosphereMaterial;

	// set up shader data for this geosphere's atmosphere
	void SetMaterialParameters(const matrix4x4d &t, const float r, const std::vector<Camera::Shadow> &s, const AtmosphereParameters &ap);

	// atmosphere geometry
	std::unique_ptr<Graphics::Drawables::Sphere3D> m_atmos;
};

#endif /* _GEOSPHERE_H */
