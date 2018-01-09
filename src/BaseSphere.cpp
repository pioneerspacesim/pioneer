// Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "Pi.h"
#include "BaseSphere.h"
#include "GeoSphere.h"
#include "GasGiant.h"
#include "graphics/Material.h"
#include "CityOnPlanet.h"

BaseSphere::BaseSphere(const SystemBody *body) : m_sbody(body), m_terrain(Terrain::InstanceTerrain(body))
{
	std::vector<vector3d> m_positions;
	std::vector<RegionType> m_regionTypes;
	const double planetRadius = body->GetRadius();
	const double invPlanetRadius = 1.0 / planetRadius;

	// step through the planet's sbody's children and set up regions for surface starports
	for (SystemBody* kid : body->GetChildren()) {
		if (kid->GetType() == SystemBody::TYPE_STARPORT_SURFACE) {
			// calculate position of starport
			const vector3d pos = (kid->GetOrbit().GetPlane() * vector3d(0, 1, 0));

			// set up regions which contain the details for region implementation
			RegionType rt;

			rt.height = m_terrain->GetHeight(pos); // height in planet radii

			// Calculate average variation of four points about star port
			// points do not need to be on the planet surface
			const double delta = (0.75 * CITY_ON_PLANET_RADIUS / planetRadius);
			double avgVariation = fabs(m_terrain->GetHeight(vector3d(pos.x + delta, pos.y, pos.z)) - rt.height);
			avgVariation += fabs(m_terrain->GetHeight(vector3d(pos.x - delta, pos.y, pos.z)) - rt.height);
			avgVariation += fabs(m_terrain->GetHeight(vector3d(pos.x, pos.y, pos.z + delta)) - rt.height);
			avgVariation += fabs(m_terrain->GetHeight(vector3d(pos.x, pos.y, pos.z - delta)) - rt.height);
			avgVariation *= 0.25;
			rt.heightVariation = invPlanetRadius + 0.625 * avgVariation;

			const double citySize_m = CITY_ON_PLANET_RADIUS * 1.1;
			double size = fabs(cos(std::min(citySize_m, 0.2 * planetRadius) / planetRadius)); // angle between city center/boundary = 2pi*city size/(perimeter great circle = 2pi r)
			rt.outer = size; // city center pos and current point will be dotted, and compared against size
			rt.inner = (1.0 - size)*0.5 + size;
			rt.Type = 1;
			rt.Valid = true;

			m_positions.push_back(pos);
			m_regionTypes.push_back(rt);
		}
	}

	m_terrain->SetCityRegions(m_positions, m_regionTypes);
}

BaseSphere::~BaseSphere() {}

//static
void BaseSphere::Init()
{
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
	Graphics::RenderState *rs, RefCountedPtr<Graphics::Material> mat)
{
	PROFILE_SCOPED()
	using namespace Graphics;
	const vector3d yaxis = campos.Normalized();
	const vector3d zaxis = vector3d(1.0, 0.0, 0.0).Cross(yaxis).Normalized();
	const vector3d xaxis = yaxis.Cross(zaxis);
	const matrix4x4d invrot = matrix4x4d::MakeRotMatrix(xaxis, yaxis, zaxis).Inverse();

	renderer->SetTransform(modelView * matrix4x4d::ScaleMatrix(rad, rad, rad) * invrot);

	if(!m_atmos)
		m_atmos.reset( new Drawables::Sphere3D(renderer, mat, rs, 4, 1.0f, ATTRIB_POSITION));
	m_atmos->Draw(renderer);

	renderer->GetStats().AddToStatCount(Graphics::Stats::STAT_ATMOSPHERES, 1);
}
