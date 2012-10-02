// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Intro.h"
#include "Lang.h"
#include "Ship.h"
#include "graphics/Renderer.h"

Intro::Intro(Graphics::Renderer *r, int width, int height)
: Cutscene(r, width, height)
{
	m_background.Reset(new Background::Container(r, UNIVERSE_SEED));
	m_ambientColor = Color(0.1f, 0.1f, 0.1f, 1.f);

	const Color lc(1.f, 1.f, 1.f, 0.f);
	m_lights.push_back(Graphics::Light(Graphics::Light::LIGHT_DIRECTIONAL, vector3f(0.f, 1.f, 1.f), lc, lc, lc));

	m_model = LmrLookupModelByName("lanner_ub");

	// Model parameters
	LmrObjParams params = {
		"ShipAnimation", // animation namespace
		0.0, // time
		{ }, // animation stages
		{ 0.0, 1.0 }, // animation positions
		Lang::PIONEER, // label
		0, // equipment
		Ship::FLYING, // flightState
		{ 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f, 0.0f }, // rear thrusters active
		{	// pColor[3]
		{ { .2f, .2f, .5f, 1.0f }, { 1, 1, 1 }, { 0, 0, 0 }, 100.0 },
		{ { 0.5f, 0.5f, 0.5f, 1.0f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 },
		{ { 0.8f, 0.8f, 0.8f, 1.0f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 } },
	};
	m_modelParams = params;

	// Some equipment (in case the model can show them)
	m_equipment.Add(Equip::ECM_ADVANCED, 1);
	m_equipment.Add(Equip::HYPERCLOUD_ANALYZER, 1);
	m_equipment.Add(Equip::ATMOSPHERIC_SHIELDING, 1);
	m_equipment.Add(Equip::FUEL_SCOOP, 1);
	m_equipment.Add(Equip::SCANNER, 1);
	m_equipment.Add(Equip::RADAR_MAPPER, 1);
	m_equipment.Add(Equip::MISSILE_NAVAL, 4);
	m_modelParams.equipment = &m_equipment;
}

void Intro::Draw(float _time)
{
	m_renderer->SetPerspectiveProjection(75, m_aspectRatio, 1.f, 10000.f);
	m_renderer->SetTransform(matrix4x4f::Identity());

	// XXX all this stuff will be gone when intro uses a Camera
	// rotate background by time, and a bit extra Z so it's not so flat
	matrix4x4d brot = matrix4x4d::RotateXMatrix(-0.25*_time) * matrix4x4d::RotateZMatrix(0.6);
	m_background->Draw(m_renderer, brot);

	glPushAttrib(GL_ALL_ATTRIB_BITS);

	const Color oldSceneAmbientColor = m_renderer->GetAmbientColor();
	m_renderer->SetAmbientColor(m_ambientColor);
	m_renderer->SetLights(m_lights.size(), &m_lights[0]);

	matrix4x4f rot = matrix4x4f::RotateYMatrix(_time) * matrix4x4f::RotateZMatrix(0.6f*_time) *
			matrix4x4f::RotateXMatrix(_time*0.7f);
	rot[14] = -80.0;
	m_model->Render(rot, &m_modelParams);
	glPopAttrib();
	m_renderer->SetAmbientColor(oldSceneAmbientColor);
}
