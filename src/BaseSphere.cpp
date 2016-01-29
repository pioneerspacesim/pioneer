// Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "Pi.h"
#include "BaseSphere.h"
#include "GeoSphere.h"
#include "GasGiant.h"
#include "graphics/Material.h"

BaseSphere::BaseSphere(const SystemBody *body) : m_sbody(body), m_terrain(Terrain::InstanceTerrain(body)) {}

BaseSphere::~BaseSphere() {}

//static 
void BaseSphere::Init()
{
	GeoSphere::Init();
}

//static 
void BaseSphere::Uninit()
{
	GeoSphere::Uninit();
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
	Graphics::RenderState *rs, Graphics::Material *mat)
{
	using namespace Graphics;
	const vector3d yaxis = campos.Normalized();
	const vector3d zaxis = vector3d(1.0, 0.0, 0.0).Cross(yaxis).Normalized();
	const vector3d xaxis = yaxis.Cross(zaxis);
	const matrix4x4d invrot = matrix4x4d::MakeRotMatrix(xaxis, yaxis, zaxis).Inverse();

	renderer->SetTransform(modelView * matrix4x4d::ScaleMatrix(rad, rad, rad) * invrot);

	// what is this? Well, angle to the horizon is:
	// acos(planetRadius/viewerDistFromSphereCentre)
	// and angle from this tangent on to atmosphere is:
	// acos(planetRadius/atmosphereRadius) ie acos(1.0/1.01244blah)
	const double endAng = acos(1.0 / campos.Length()) + acos(1.0 / rad);
	const double latDiff = endAng / double(LAT_SEGS);

	double rot = 0.0;
	float sinTable[LONG_SEGS + 1];
	float cosTable[LONG_SEGS + 1];
	for (int i = 0; i <= LONG_SEGS; i++, rot += 2.0*M_PI / double(LONG_SEGS)) {
		sinTable[i] = float(sin(rot));
		cosTable[i] = float(cos(rot));
	}

	// Tri-fan above viewer
	if(!m_TriFanAbove.Valid())
	{
		// VertexBuffer
		VertexBufferDesc vbd;
		vbd.attrib[0].semantic = ATTRIB_POSITION;
		vbd.attrib[0].format = ATTRIB_FORMAT_FLOAT3;
		vbd.numVertices = LONG_SEGS + 2;
		vbd.usage = BUFFER_USAGE_STATIC;
		m_TriFanAbove.Reset(renderer->CreateVertexBuffer(vbd));
	}
#pragma pack(push, 4)
	struct PosVert {
		vector3f pos;
	};
#pragma pack(pop)
	PosVert* vtxPtr = m_TriFanAbove->Map<PosVert>(Graphics::BUFFER_MAP_WRITE);
	vtxPtr[0].pos = vector3f(0.f, 1.f, 0.f);
	for (int i = 0; i <= LONG_SEGS; i++)
	{
		vtxPtr[i + 1].pos = vector3f(
			sin(latDiff)*sinTable[i],
			cos(latDiff),
			-sin(latDiff)*cosTable[i]);
	}
	m_TriFanAbove->Unmap();
	renderer->DrawBuffer(m_TriFanAbove.Get(), rs, mat, Graphics::TRIANGLE_FAN);

	// and wound latitudinal strips
	if (!m_LatitudinalStrips[0].Valid())
	{
		VertexBufferDesc vbd;
		vbd.attrib[0].semantic = ATTRIB_POSITION;
		vbd.attrib[0].format = ATTRIB_FORMAT_FLOAT3;
		vbd.numVertices = (LONG_SEGS + 1) * 2;
		vbd.usage = BUFFER_USAGE_DYNAMIC;
		for (int j = 0; j < LAT_SEGS; j++) {
			// VertexBuffer
			m_LatitudinalStrips[j].Reset(renderer->CreateVertexBuffer(vbd));
		}
	}
	double lat = latDiff;
	for (int j=1; j<LAT_SEGS; j++, lat += latDiff) {
		const float cosLat = cos(lat);
		const float sinLat = sin(lat);
		const float cosLat2 = cos(lat+latDiff);
		const float sinLat2 = sin(lat+latDiff);
		vtxPtr = m_LatitudinalStrips[j]->Map<PosVert>(Graphics::BUFFER_MAP_WRITE);
		vtxPtr[0].pos = vector3f(0.f, 1.f, 0.f);
		for (int i=0; i<=LONG_SEGS; i++) {
			vtxPtr[(i*2)+0].pos = vector3f(sinLat*sinTable[i], cosLat, -sinLat*cosTable[i]);
			vtxPtr[(i*2)+1].pos = vector3f(sinLat2*sinTable[i], cosLat2, -sinLat2*cosTable[i]);
		}
		m_LatitudinalStrips[j]->Unmap();
		renderer->DrawBuffer(m_LatitudinalStrips[j].Get(), rs, mat, Graphics::TRIANGLE_STRIP);
	}

	renderer->GetStats().AddToStatCount(Graphics::Stats::STAT_ATMOSPHERES, 1);
}
