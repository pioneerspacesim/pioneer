// Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "BaseSphere.h"

#include "GasGiant.h"
#include "GeoSphere.h"

#include "graphics/Drawables.h"
#include "graphics/Renderer.h"

BaseSphere::BaseSphere(const SystemBody *body) :
	m_sbody(body),
	m_terrain(Terrain::InstanceTerrain(body)) {}

BaseSphere::~BaseSphere() {}

//static
void BaseSphere::Init()
{
	PROFILE_SCOPED()
	GeoSphere::Init();
	GasGiant::Init();
}

//static
void BaseSphere::Uninit()
{
	GeoSphere::Uninit();
	GasGiant::Uninit();
}

//static
void BaseSphere::UpdateAllBaseSphereDerivatives()
{
	GeoSphere::UpdateAllGeoSpheres();
	GasGiant::UpdateAllGasGiants();
}

//static
void BaseSphere::OnChangeDetailLevel()
{
	GeoSphere::OnChangeDetailLevel();
}

void BaseSphere::DrawAtmosphereSurface(Graphics::Renderer *renderer,
	const matrix4x4d &modelView, const vector3d &campos, float rad,
	RefCountedPtr<Graphics::Material> mat)
{
	PROFILE_SCOPED()
	using namespace Graphics;
	const vector3d yaxis = campos.Normalized();
	const vector3d zaxis = vector3d(1.0, 0.0, 0.0).Cross(yaxis).Normalized();
	const vector3d xaxis = yaxis.Cross(zaxis);
	const matrix4x4d invrot = matrix4x4d::MakeRotMatrix(xaxis, yaxis, zaxis).Inverse();

	renderer->SetTransform(matrix4x4f(modelView * matrix4x4d::ScaleMatrix(rad) * invrot));

	if (!m_atmos)
		m_atmos.reset(new Drawables::Sphere3D(renderer, mat, 4, 1.0f, ATTRIB_POSITION));
	m_atmos->Draw(renderer);

	renderer->GetStats().AddToStatCount(Graphics::Stats::STAT_ATMOSPHERES, 1);
}
