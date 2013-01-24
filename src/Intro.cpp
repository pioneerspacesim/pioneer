// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Intro.h"
#include "Pi.h"
#include "Lang.h"
#include "Ship.h"
#include "graphics/Renderer.h"

Intro::Intro(Graphics::Renderer *r, int width, int height)
: Cutscene(r, width, height)
{
	using Graphics::Light;

	m_background.Reset(new Background::Container(r, UNIVERSE_SEED));
	m_ambientColor = Color(0.f);

	const Color one = Color::WHITE;
	const Color two = Color(0.1f, 0.1f, 0.5f, 0.f);
	m_lights.push_back(Light(Graphics::Light::LIGHT_DIRECTIONAL, vector3f(0.f, 0.3f, 1.f), one, one));
	m_lights.push_back(Light(Graphics::Light::LIGHT_DIRECTIONAL, vector3f(0.f, -1.f, 0.f), two, Color::BLACK));

	m_model = Pi::FindModel("lanner");
	m_model->SetLabel(Lang::PIONEER);

	// Model parameters
	memset(&m_modelParams, 0, sizeof(LmrObjParams));
	m_modelParams.animationNamespace = "ShipAnimation";
	m_modelParams.flightState = Ship::FLYING;
	m_modelParams.linthrust[2] = -1.f;
	m_modelParams.animValues[1] = 1.f;

	LmrMaterial matA = { { .2f, .2f, .5f, 1.0f }, { 1, 1, 1 }, { 0, 0, 0 }, 100.0 };
	LmrMaterial matB = { { 0.5f, 0.5f, 0.5f, 1.0f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 };
	LmrMaterial matC = { { 0.8f, 0.8f, 0.8f, 1.0f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 };
	m_modelParams.pMat[0] = matA;
	m_modelParams.pMat[1] = matB;
	m_modelParams.pMat[2] = matC;

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

	glPushAttrib(GL_ALL_ATTRIB_BITS & (~GL_POINT_BIT));

	const Color oldSceneAmbientColor = m_renderer->GetAmbientColor();
	m_renderer->SetAmbientColor(m_ambientColor);
	m_renderer->SetLights(m_lights.size(), &m_lights[0]);

	matrix4x4f trans =
		matrix4x4f::Translation(-25.0f, 0, -80.0f) *
		matrix4x4f::RotateYMatrix(_time) *
		matrix4x4f::RotateZMatrix(0.6f*_time) *
		matrix4x4f::RotateXMatrix(_time*0.7f);
	m_model->Render(m_renderer, trans, &m_modelParams);
	glPopAttrib();
	m_renderer->SetAmbientColor(oldSceneAmbientColor);
}
