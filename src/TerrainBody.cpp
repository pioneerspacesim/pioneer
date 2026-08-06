// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "TerrainBody.h"

#include <algorithm>

#include "Frame.h"
#include "GameSaveError.h"
#include "GasGiant.h"
#include "GeoSphere.h"
#include "Json.h"
#include "Space.h"
#include "galaxy/SystemBody.h"
#include "graphics/Renderer.h"

TerrainBody::TerrainBody(SystemBody *sbody) :
	Body(),
	m_sbody(sbody),
	m_mass(0)
{
	InitTerrainBody();
}

TerrainBody::~TerrainBody()
{
	m_baseSphere.reset();
}

void TerrainBody::InitTerrainBody()
{
	PROFILE_SCOPED()
	assert(m_sbody);
	m_mass = m_sbody->GetMass();
	if (!m_baseSphere) {
		if (SystemBody::SUPERTYPE_GAS_GIANT == m_sbody->GetSuperType()) {
			m_baseSphere.reset(new GasGiant(m_sbody));
		} else {
			m_baseSphere.reset(new GeoSphere(m_sbody));
		}
	}
	m_maxFeatureHeight = (m_baseSphere->GetMaxFeatureHeight() + 1.0) * m_sbody->GetRadius();
}

void TerrainBody::SaveToJson(Json &jsonObj, Space *space)
{
	Body::SaveToJson(jsonObj, space);

	Json terrainBodyObj({}); // Create JSON object to contain terrain body data.

	terrainBodyObj["index_for_system_body"] = space->GetIndexForSystemBody(m_sbody);

	jsonObj["terrain_body"] = terrainBodyObj; // Add terrain body object to supplied object.
}

TerrainBody::TerrainBody(const Json &jsonObj, Space *space) :
	Body(jsonObj, space)
{

	try {
		Json terrainBodyObj = jsonObj["terrain_body"];

		m_sbody = space->GetSystemBodyByIndex(terrainBodyObj["index_for_system_body"]);
	} catch (Json::type_error &) {
		throw SavedGameCorruptException();
	}

	InitTerrainBody();
}

void TerrainBody::Render(Graphics::Renderer *renderer, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform)
{
	matrix4x4d ftran = viewTransform;
	vector3d fpos = viewCoords;
	double rad = m_sbody->GetRadius();

	float znear, zfar;
	renderer->GetNearFarRange(znear, zfar);

	//stars very far away are downscaled, because they cannot be
	//accurately drawn using actual distances
	int shrink = 0;
	if (m_sbody->GetSuperType() == SystemBody::SUPERTYPE_STAR) {
		double len = fpos.Length();
		double dist_to_horizon;
		for (;;) {
			if (len < rad) // player inside radius case
				break;

			dist_to_horizon = sqrt(len * len - rad * rad);

			if (dist_to_horizon < zfar * 0.5)
				break;

			rad *= 0.25;
			fpos = 0.25 * fpos;
			len *= 0.25;
			++shrink;
		}
	}

	vector3d campos = fpos;
	ftran.ClearToRotOnly();
	campos = campos * ftran;

	campos = campos * (1.0 / rad); // position of camera relative to planet "model"

	std::vector<Camera::Shadow> shadows;
	if (camera) {
		camera->PrincipalShadows(this, 3, shadows);
		for (std::vector<Camera::Shadow>::iterator it = shadows.begin(), itEnd = shadows.end(); it != itEnd; ++it) {
			it->centre = ftran * it->centre;
		}
	}

	ftran.Scale(rad, rad, rad);

	// translation not applied until patch render to fix jitter
	m_baseSphere->Render(renderer, ftran, -campos, m_sbody->GetRadius(), shadows);

	ftran.Translate(campos.x, campos.y, campos.z);
	SubRender(renderer, ftran, campos);

	//clear depth buffer, shrunken objects should not interact with foreground
	if (shrink)
		renderer->ClearDepthBuffer();
}

void TerrainBody::SetFrame(FrameId fId)
{
	Frame *f = Frame::GetFrame(GetFrame());

	if (f) {
		f->SetPlanetGeom(0, nullptr);
	}
	Body::SetFrame(fId);

	f = Frame::GetFrame(fId);
	if (f) {
		f->SetPlanetGeom(0, nullptr);
	}
}

double TerrainBody::GetTerrainHeight(const vector3d &pos_) const
{
	double radius = m_sbody->GetRadius();
	if (m_baseSphere) {
		return radius * (1.0 + m_baseSphere->GetTerrainHeight(pos_));
	} else {
		assert(0);
		return radius;
	}
}

vector3d TerrainBody::GetTerrainSurfacePoint(const vector3d &pos) const
{
	const vector3d radial = pos.NormalizedSafe();
	return radial * GetTerrainHeight(radial);
}

vector3d TerrainBody::GetTerrainSurfaceNormal(const vector3d &pos, double sampleSizeMeters) const
{
	// Sample terrain heights at half the sample diameter in each tangent direction,
	// then derive the normal from the east-west and north-south surface vectors.

	const vector3d n = pos.NormalizedSafe();
	if (n.LengthSqr() < 1e-18)
		return n;

	vector3d east = n.Cross(vector3d(0, 0, 1));
	if (east.LengthSqr() < 1e-12)
		east = n.Cross(vector3d(1, 0, 0));
	east = east.Normalized();
	const vector3d north = east.Cross(n).Normalized();

	const double radius = m_sbody->GetRadius();
	const double halfSample = 0.5 * std::max(sampleSizeMeters, 1.0);
	const double ang = halfSample / radius;

	const vector3d dirE = (n + east * ang).Normalized();
	const vector3d dirW = (n - east * ang).Normalized();
	const vector3d dirN = (n + north * ang).Normalized();
	const vector3d dirS = (n - north * ang).Normalized();

	const vector3d pE = GetTerrainSurfacePoint(dirE);
	const vector3d pW = GetTerrainSurfacePoint(dirW);
	const vector3d pN = GetTerrainSurfacePoint(dirN);
	const vector3d pS = GetTerrainSurfacePoint(dirS);

	vector3d normal = (pE - pW).Cross(pN - pS).NormalizedSafe();
	if (normal.Dot(n) < 0.0)
		normal = -normal;
	return normal;
}

//static
void TerrainBody::OnChangeDetailLevel(Graphics::Renderer *r)
{
	BaseSphere::OnChangeDetailLevel(r);
}

const char *TerrainBody::GetHeightFractalName() const
{
	if (Terrain* terrain = m_baseSphere->GetTerrain()) {
		return terrain->GetHeightFractalName();
	}
	return nullptr;
}

const char* TerrainBody::GetColorFractalName() const
{
	if (Terrain *terrain = m_baseSphere->GetTerrain()) {
		return terrain->GetColorFractalName();
	}
	return nullptr;
}
