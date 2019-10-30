// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "TerrainBody.h"

#include "Frame.h"
#include "GameSaveError.h"
#include "GasGiant.h"
#include "GeoSphere.h"
#include "Json.h"
#include "Space.h"
#include "graphics/Renderer.h"

TerrainBody::TerrainBody(SystemBody *sbody) :
	Body(),
	SystemBodyWrapper(sbody),
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
	m_mass = SystemBodyWrapper::GetSystemBodyMass();
	if (!m_baseSphere) {
		if (SystemBodyWrapper::IsSuperType(GalaxyEnums::BodySuperType::SUPERTYPE_GAS_GIANT)) {
			m_baseSphere.reset(new GasGiant(SystemBodyWrapper::GetSystemBody()));
		} else {
			m_baseSphere.reset(new GeoSphere(SystemBodyWrapper::GetSystemBody()));
		}
	}
	m_maxFeatureHeight = (m_baseSphere->GetMaxFeatureHeight() + 1.0) * SystemBodyWrapper::GetSystemBodyRadius();
}

void TerrainBody::SaveToJson(Json &jsonObj, Space *space)
{
	Body::SaveToJson(jsonObj, space);

	Json terrainBodyObj({}); // Create JSON object to contain terrain body data.

	terrainBodyObj["index_for_system_body"] = space->GetIndexForSystemBody(SystemBodyWrapper::GetSystemBody());

	jsonObj["terrain_body"] = terrainBodyObj; // Add terrain body object to supplied object.
}

TerrainBody::TerrainBody(const Json &jsonObj, Space *space) :
	Body(jsonObj, space),
	SystemBodyWrapper(space->GetSystemBodyByIndex(jsonObj["terrain_body"]["index_for_system_body"]))
{

	try {
		// Left here because this will throw an exception if
		// the initialization above will not work properly
		Json terrainBodyObj = jsonObj["terrain_body"];

	} catch (Json::type_error &) {
		throw SavedGameCorruptException();
	}

	InitTerrainBody();
}

void TerrainBody::Render(Graphics::Renderer *renderer, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform)
{
	matrix4x4d ftran = viewTransform;
	vector3d fpos = viewCoords;
	double rad = SystemBodyWrapper::GetSystemBodyRadius();

	float znear, zfar;
	renderer->GetNearFarRange(znear, zfar);

	//stars very far away are downscaled, because they cannot be
	//accurately drawn using actual distances
	int shrink = 0;
	if (SystemBodyWrapper::IsSuperType(GalaxyEnums::BodySuperType::SUPERTYPE_STAR)) {
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

	ftran.Scale(rad);

	// translation not applied until patch render to fix jitter
	m_baseSphere->Render(renderer, ftran, -campos, SystemBodyWrapper::GetSystemBodyRadius(), shadows);

	ftran.Translate(campos.x, campos.y, campos.z);
	SubRender(renderer, ftran, campos);

	//clear depth buffer, shrunken objects should not interact with foreground
	if (shrink)
		renderer->ClearDepthBuffer();
}

void TerrainBody::SetFrame(Frame *f)
{
	if (GetFrame()) {
		GetFrame()->SetPlanetGeom(0, 0);
	}
	Body::SetFrame(f);
	if (f) {
		GetFrame()->SetPlanetGeom(0, 0);
	}
}

double TerrainBody::GetTerrainHeight(const vector3d &pos_) const
{
	double radius = SystemBodyWrapper::GetSystemBodyRadius();
	if (m_baseSphere) {
		return radius * (1.0 + m_baseSphere->GetHeight(pos_));
	} else {
		assert(0);
		return radius;
	}
}

//static
void TerrainBody::OnChangeDetailLevel()
{
	GeoSphere::OnChangeDetailLevel();
	GasGiant::OnChangeDetailLevel();
}
