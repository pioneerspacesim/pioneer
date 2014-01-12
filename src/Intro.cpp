// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Intro.h"
#include "Pi.h"
#include "Lang.h"
#include "Easing.h"
#include "graphics/Renderer.h"
#include "graphics/TextureBuilder.h"
#include "graphics/Graphics.h"
#include "scenegraph/SceneGraph.h"
#include "scenegraph/ModelSkin.h"
#include <algorithm>

struct PiRngWrapper {
	unsigned int operator()(unsigned int n) {
		return Pi::rng.Int32(n);
	}
};

Intro::Intro(Graphics::Renderer *r, int width, int height)
: Cutscene(r, width, height)
{
	using Graphics::Light;

	m_background.reset(new Background::Container(r, UNIVERSE_SEED));
	m_ambientColor = Color(0);

	const Color one = Color::WHITE;
	const Color two = Color(77, 77, 204, 0);
	m_lights.push_back(Light(Graphics::Light::LIGHT_DIRECTIONAL, vector3f(0.f, 0.3f, 1.f), one, one));
	m_lights.push_back(Light(Graphics::Light::LIGHT_DIRECTIONAL, vector3f(0.f, -1.f, 0.f), two, Color::BLACK));

	SceneGraph::ModelSkin skin;
	skin.SetDecal("pioneer");
	skin.SetLabel(Lang::PIONEER);

	for (std::vector<ShipType::Id>::const_iterator i = ShipType::player_ships.begin(); i != ShipType::player_ships.end(); ++i) {
		SceneGraph::Model *model = Pi::FindModel(ShipType::types[*i].modelName)->MakeInstance();
		skin.SetRandomColors(Pi::rng);
		skin.Apply(model);
		model->SetThrust(vector3f(0.f, 0.f, -0.6f), vector3f(0.f));
		const Uint32 numMats = model->GetNumMaterials();
		for( Uint32 m=0; m<numMats; m++ ) {
			RefCountedPtr<Graphics::Material> mat = model->GetMaterialByIndex(m);
			mat->specialParameter0 = nullptr;
		}
		m_models.push_back(model);
	}

	PiRngWrapper rng;
	std::random_shuffle(m_models.begin(), m_models.end(), rng);

	m_state = STATE_SELECT;
	m_modelIndex = 0;
}

Intro::~Intro()
{
	for (std::vector<SceneGraph::Model*>::iterator i = m_models.begin(); i != m_models.end(); ++i)
		delete (*i);
}

void Intro::Draw(float _time)
{
	switch (m_state) {
		case STATE_SELECT:
			m_model = m_models[m_modelIndex++];
			if (m_modelIndex == m_models.size()) m_modelIndex = 0;
			m_zoomBegin = -10000.0f;
			m_zoomEnd = -m_model->GetDrawClipRadius()*1.7f;
			m_dist = m_zoomBegin;
			m_state = STATE_ZOOM_IN;
			m_startTime = _time;
			break;

		case STATE_ZOOM_IN:
			m_dist = Clamp(Easing::Quad::EaseOut(_time-m_startTime, m_zoomBegin, m_zoomEnd-m_zoomBegin, 2.0f), m_zoomBegin, m_zoomEnd);
			if (_time-m_startTime > 2.0f) {
				m_state = STATE_WAIT;
				m_startTime = _time;
			}
			break;

		case STATE_WAIT:
			if (_time - m_startTime > 10.0f) {
				m_state = STATE_ZOOM_OUT;
				m_startTime = _time;
			}
			break;

		case STATE_ZOOM_OUT:
			m_dist = Clamp(Easing::Quad::EaseIn(_time-m_startTime, m_zoomEnd, m_zoomBegin-m_zoomEnd, 2.0f), m_zoomBegin, m_zoomEnd);
			if (_time-m_startTime > 2.0f) {
				m_state = STATE_SELECT;
				m_startTime = _time;
			}
			break;
	}

	m_renderer->SetPerspectiveProjection(75, m_aspectRatio, 1.f, 10000.f);
	m_renderer->SetTransform(matrix4x4f::Identity());

	m_renderer->SetDepthTest(true);
	m_renderer->SetDepthWrite(true);

	glPushAttrib(GL_ALL_ATTRIB_BITS & (~GL_POINT_BIT));

	const Color oldSceneAmbientColor = m_renderer->GetAmbientColor();
	m_renderer->SetAmbientColor(m_ambientColor);
	m_renderer->SetLights(m_lights.size(), &m_lights[0]);

	// XXX all this stuff will be gone when intro uses a Camera
	// rotate background by time, and a bit extra Z so it's not so flat
	matrix4x4d brot = matrix4x4d::RotateXMatrix(-0.25*_time) * matrix4x4d::RotateZMatrix(0.6);
	m_background->Draw(m_renderer, brot);

	const float w = Graphics::GetScreenWidth()*(2.0/3.0);
	const float h = Graphics::GetScreenHeight();
	const float ratio = w/h;
	m_renderer->SetViewport(0, 0, w, h);
	m_renderer->SetPerspectiveProjection(75, ratio, 1.f, 10000.f);

	matrix4x4f trans =
		matrix4x4f::Translation(0, 0, m_dist) *
		matrix4x4f::RotateYMatrix(_time) *
		matrix4x4f::RotateZMatrix(0.6f*_time) *
		matrix4x4f::RotateXMatrix(_time*0.7f);
	m_model->Render(trans);

	m_renderer->SetAmbientColor(oldSceneAmbientColor);

	m_renderer->SetViewport(0, 0, Graphics::GetScreenWidth(), Graphics::GetScreenHeight());

	glPopAttrib();
}
