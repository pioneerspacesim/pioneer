// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "BaseSphere.h"

#include "GasGiant.h"
#include "GeoSphere.h"

#include "Pi.h"

#include "galaxy/AtmosphereParameters.h"
#include "galaxy/SystemBody.h"
#include "graphics/Drawables.h"
#include "graphics/Renderer.h"

struct BaseSphereDataBlock {
	vector3f geosphereCenter;
	float geosphereRadius;
	float geosphereInvRadius;
	float geosphereAtmosTopRad;
	float geosphereAtmosFogDensity;
	float geosphereAtmosInvScaleHeight;
	Color4f atmosColor;
	alignas(16) vector3f coefficientsR;
	alignas(16) vector3f coefficientsM;
	alignas(16) vector2f scaleHeight;

	// Eclipse struct data
	alignas(16) vector3f shadowCentreX;
	alignas(16) vector3f shadowCentreY;
	alignas(16) vector3f shadowCentreZ;
	alignas(16) vector3f srad;
	alignas(16) vector3f lrad;
	alignas(16) vector3f sdivlrad;

	alignas(16) vector2f tropoHeight;
};
static_assert(sizeof(BaseSphereDataBlock) == 208, "");

std::unique_ptr<Graphics::Drawables::Sphere3D> BaseSphere::m_atmos;

BaseSphere::BaseSphere(const SystemBody *body) :
	m_sbody(body),
	m_terrain(Terrain::InstanceTerrain(body)) {}

BaseSphere::~BaseSphere() {}

//static
void BaseSphere::Init(Graphics::Renderer *renderer)
{
	PROFILE_SCOPED()
	GeoSphere::InitGeoSphere();
	GasGiant::InitGasGiant();
	ResetAtmosphereGeometry(renderer);
}

//static
void BaseSphere::Uninit()
{
	GeoSphere::UninitGeoSphere();
	GasGiant::UninitGasGiant();
}

//static
void BaseSphere::UpdateAllBaseSphereDerivatives()
{
	GeoSphere::UpdateAllGeoSpheres();
	GasGiant::UpdateAllGasGiants();
}

//static
void BaseSphere::OnChangeDetailLevel(Graphics::Renderer *renderer)
{
	GeoSphere::OnChangeGeoSphereDetailLevel();
	GasGiant::OnChangeGasGiantsDetailLevel();
	ResetAtmosphereGeometry(renderer);
}

void BaseSphere::DrawAtmosphereSurface(Graphics::Renderer *renderer,
	const matrix4x4d &modelView, const vector3d &campos, float rad,
	RefCountedPtr<Graphics::Material> mat)
{
	PROFILE_SCOPED()
	assert(m_atmos != nullptr);
	const vector3d yaxis = campos.Normalized();
	const vector3d zaxis = vector3d(1.0, 0.0, 0.0).Cross(yaxis).Normalized();
	const vector3d xaxis = yaxis.Cross(zaxis);
	const matrix4x4d invrot = matrix4x4d::MakeRotMatrix(xaxis, yaxis, zaxis).Inverse();

	renderer->SetTransform(matrix4x4f(modelView * matrix4x4d::ScaleMatrix(rad) * invrot));

	m_atmos->Draw(renderer, mat.Get());

	renderer->GetStats().AddToStatCount(Graphics::Stats::STAT_ATMOSPHERES, 1);
}

static size_t s_baseSphereData = "BaseSphereData"_hash;
static size_t s_numShadows = "NumShadows"_hash;
void BaseSphere::SetMaterialParameters(const matrix4x4d &trans, const float radius, const std::vector<Camera::Shadow> &shadows, const AtmosphereParameters &ap)
{
	BaseSphereDataBlock matData{};

	matData.geosphereCenter = vector3f(trans * vector3d(0.0)) / radius;
	matData.geosphereRadius = ap.planetRadius;
	matData.geosphereInvRadius = 1.0f / radius;
	matData.geosphereAtmosTopRad = ap.atmosRadius;
	matData.geosphereAtmosFogDensity = ap.atmosDensity;
	matData.geosphereAtmosInvScaleHeight = ap.atmosInvScaleHeight;
	matData.atmosColor = ap.atmosCol.ToColor4f();
	matData.coefficientsR = ap.rayleighCoefficients;
	matData.coefficientsM = ap.mieCoefficients;
	matData.scaleHeight = ap.scaleHeight;

	// we handle up to three shadows at a time
	auto it = shadows.cbegin(), itEnd = shadows.cend();
	int j = 0;
	while (j < 3 && it != itEnd) {
		matData.shadowCentreX[j] = it->centre[0];
		matData.shadowCentreY[j] = it->centre[1];
		matData.shadowCentreZ[j] = it->centre[2];
		matData.srad[j] = it->srad;
		matData.lrad[j] = it->lrad;
		matData.sdivlrad[j] = it->srad / it->lrad;
		++it;
		++j;
	}

	matData.tropoHeight = ap.tropoHeight;

	// FIXME: these two should share the same buffer data instead of making two separate allocs
	m_surfaceMaterial->SetBufferDynamic(s_baseSphereData, &matData);
	m_surfaceMaterial->SetPushConstant(s_numShadows, int(shadows.size()));

	if (m_atmosphereMaterial.Valid() && ap.atmosDensity > 0.0) {
		m_atmosphereMaterial->SetBufferDynamic(s_baseSphereData, &matData);
		m_atmosphereMaterial->SetPushConstant(s_numShadows, int(shadows.size()));
	}
}

void BaseSphere::ResetAtmosphereGeometry(Graphics::Renderer *renderer)
{
	if (renderer) {
		if (m_atmos) {
			m_atmos.reset();
		}

		// 4 subdivision = 5112 verts, 15360 indices
		// 5 subdivision = 20472 verts, 61440 indices
		// Pi::detail.planets == 3 if High detail, 4 is Very high
		int subdivisions = Pi::detail.planets >= 3 ? 5 : 4;
		m_atmos.reset(new Graphics::Drawables::Sphere3D(renderer, subdivisions, 1.0f, Graphics::ATTRIB_POSITION));
	}
}
