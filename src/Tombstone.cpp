// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Tombstone.h"
#include "Lang.h"
#include "Pi.h"
#include "graphics/Renderer.h"

Tombstone::Tombstone(Graphics::Renderer *r, int width, int height)
: Cutscene(r, width, height)
{
	m_ambientColor = Color(0.1f, 0.1f, 0.1f, 1.f);

	const Color lc(1.f, 1.f, 1.f, 0.f);
	m_lights.push_back(Graphics::Light(Graphics::Light::LIGHT_DIRECTIONAL, vector3f(0.f, 1.f, 1.f), lc, lc));

	m_model = Pi::FindModel("tombstone");

	// Model parameters
	memset(&m_modelParams, 0, sizeof(LmrObjParams));
	LmrMaterial matA = { { 1.0f, 1.0f, 1.0f, 1.0f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 };
	LmrMaterial matB = { { 0.8f, 0.6f, 0.5f, 1.0f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 };
	LmrMaterial matC = { { 0.5f, 0.5f, 0.5f, 1.0f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 };
	m_modelParams.pMat[0] = matA;
	m_modelParams.pMat[1] = matB;
	m_modelParams.pMat[2] = matC;
	m_modelParams.label = Lang::TOMBSTONE_EPITAPH;
}

void Tombstone::Draw(float _time)
{
	m_renderer->SetPerspectiveProjection(75, m_aspectRatio, 1.f, 10000.f);
	m_renderer->SetTransform(matrix4x4f::Identity());

	glPushAttrib(GL_ALL_ATTRIB_BITS & (~GL_POINT_BIT));

	const Color oldSceneAmbientColor = m_renderer->GetAmbientColor();
	m_renderer->SetAmbientColor(m_ambientColor);
	m_renderer->SetLights(m_lights.size(), &m_lights[0]);

	matrix4x4f rot = matrix4x4f::RotateYMatrix(_time*2);
	rot[14] = -std::max(150.0f - 30.0f*_time, 30.0f);
	m_model->Render(m_renderer, rot, &m_modelParams);
	glPopAttrib();
	m_renderer->SetAmbientColor(oldSceneAmbientColor);
}
