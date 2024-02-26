// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Intro.h"
#include "Easing.h"
#include "Lang.h"
#include "Pi.h"
#include "galaxy/Galaxy.h"
#include "galaxy/GalaxyGenerator.h"
#include "graphics/Graphics.h"
#include "graphics/Renderer.h"
#include "graphics/TextureBuilder.h"
#include "scenegraph/ModelSkin.h"
#include "scenegraph/SceneGraph.h"
#include <algorithm>

class PiRngWrapper {
public:
	PiRngWrapper(size_t maxValue) :
		maxVal(maxValue) {}
	typedef unsigned int result_type;
	static constexpr unsigned int min() { return 0; }
	static constexpr unsigned int max() { return std::numeric_limits<uint32_t>::max(); }
	unsigned int operator()()
	{
		return Pi::rng.Int32(maxVal);
	}

private:
	const int32_t maxVal;
};

Intro::Intro(Graphics::Renderer *r, int width, int height) :
	Cutscene(r, width, height)
{
	using Graphics::Light;

	RefreshBackground(r);
	m_ambientColor = Color::BLANK;

	const Color one = Color::WHITE;
	const Color two = Color(77, 77, 204, 0);
	m_lights.push_back(Light(Graphics::Light::LIGHT_DIRECTIONAL, vector3f(0.f, 0.3f, 1.f), one, one));
	m_lights.push_back(Light(Graphics::Light::LIGHT_DIRECTIONAL, vector3f(0.f, -1.f, 0.f), two, Color::BLACK));

	m_skin.SetDecal("pioneer");
	m_skin.SetLabel(Lang::PIONEER);

	for (auto i : ShipType::player_ships) {
		SceneGraph::Model *model = Pi::FindModel(ShipType::types[i].modelName)->MakeInstance();
		model->SetThrust(vector3f(0.f, 0.f, -0.6f), vector3f(0.f));
		if (ShipType::types[i].isGlobalColorDefined) model->SetThrusterColor(ShipType::types[i].globalThrusterColor);
		for (int j = 0; j < THRUSTER_MAX; j++) {
			if (!ShipType::types[i].isDirectionColorDefined[j]) continue;
			vector3f dir;
			switch (j) {
			case THRUSTER_FORWARD: dir = vector3f(0.0, 0.0, 1.0); break;
			case THRUSTER_REVERSE: dir = vector3f(0.0, 0.0, -1.0); break;
			case THRUSTER_LEFT: dir = vector3f(1.0, 0.0, 0.0); break;
			case THRUSTER_RIGHT: dir = vector3f(-1.0, 0.0, 0.0); break;
			case THRUSTER_UP: dir = vector3f(1.0, 0.0, 0.0); break;
			case THRUSTER_DOWN: dir = vector3f(-1.0, 0.0, 0.0); break;
			}
			model->SetThrusterColor(dir, ShipType::types[i].directionThrusterColor[j]);
		}
		const Uint32 numMats = model->GetNumMaterials();
		for (Uint32 m = 0; m < numMats; m++) {
			RefCountedPtr<Graphics::Material> mat = model->GetMaterialByIndex(m);
		}
		m_models.push_back(model);
	}

	std::shuffle(m_models.begin(), m_models.end(), PiRngWrapper(m_models.size()));

	m_modelIndex = 0;

	const int w = r->GetWindowWidth();
	const int h = r->GetWindowHeight();

	// double-width viewport, centred, then offset 1/6th to centre on the left
	// 2/3rds of the screen, to the left of the menu
	m_spinnerLeft = int(float(w) * -.5f - float(w) / 6.f);
	m_spinnerWidth = w * 2;
	m_spinnerRatio = w * 2.f / h;

	m_needReset = true;
}

Intro::~Intro()
{
	for (std::vector<SceneGraph::Model *>::iterator i = m_models.begin(); i != m_models.end(); ++i)
		delete (*i);
}

void Intro::RefreshBackground(Graphics::Renderer *r)
{
	const SystemPath s(0, 0, 0);
	RefCountedPtr<Galaxy> galaxy(GalaxyGenerator::Create());
	m_background.reset(new Background::Container(r, Pi::rng));
	m_background->GetStarfield()->Fill(Pi::rng, &s, galaxy);
}

void Intro::Reset()
{
	m_model = m_models[m_modelIndex++];
	if (m_modelIndex == m_models.size()) m_modelIndex = 0;
	m_skin.SetRandomColors(Pi::rng);
	m_skin.Apply(m_model);
	if (m_model->SupportsPatterns())
		m_model->SetPattern(Pi::rng.Int32(0, m_model->GetNumPatterns() - 1));
	m_zoomBegin = -10000.0f;
	m_zoomEnd = -m_model->GetDrawClipRadius() * 1.7f;
	m_dist = m_zoomBegin;
	m_startTime = Pi::GetApp()->GetTime();
	m_needReset = false;
}

// stage end times
static const float ZOOM_IN_END = 2.0f;
static const float WAIT_END = 12.0f;
static const float ZOOM_OUT_END = 14.0f;

void Intro::Draw(float deltaTime)
{
	if (m_needReset)
		Reset();

	// FIXME: add a better interface for retrieving a global time source
	float duration = Pi::GetApp()->GetTime() - m_startTime;

	// zoom in
	if (duration < ZOOM_IN_END)
		m_dist = Clamp(Easing::Quad::EaseOut(duration, m_zoomBegin, m_zoomEnd - m_zoomBegin, 2.0f), m_zoomBegin, m_zoomEnd);

	// wait
	else if (duration < WAIT_END) {
		m_dist = m_zoomEnd;
	}

	// zoom out
	else if (duration < ZOOM_OUT_END)
		m_dist = Clamp(Easing::Quad::EaseIn(duration - WAIT_END, m_zoomEnd, m_zoomBegin - m_zoomEnd, 2.0f), m_zoomBegin, m_zoomEnd);

	// done
	else
		m_needReset = true;

	Graphics::Renderer::StateTicket ticket(m_renderer);

	m_renderer->SetPerspectiveProjection(75, m_aspectRatio, 1.f, 10000.f);
	m_renderer->SetTransform(matrix4x4f::Identity());

	m_renderer->SetAmbientColor(m_ambientColor);

	float intensity[4] = { 1.f, 1.f, 1.f, 1.f };
	m_renderer->SetLights(m_lights.size(), &m_lights[0]);
	m_renderer->SetLightIntensity(4, intensity);

	// XXX all this stuff will be gone when intro uses a Camera
	// rotate background by time, and a bit extra Z so it's not so flat
	matrix4x4d brot = matrix4x4d::RotateXMatrix(-0.25 * Pi::GetApp()->GetTime()) * matrix4x4d::RotateZMatrix(0.6);
	m_renderer->ClearDepthBuffer();
	m_background->Draw(brot);

	m_renderer->SetViewport({ m_spinnerLeft, 0, m_spinnerWidth, m_renderer->GetWindowHeight() });
	m_renderer->SetPerspectiveProjection(75, m_spinnerRatio, 1.f, 10000.f);

	matrix4x4f trans =
		matrix4x4f::Translation(0, 0, m_dist) *
		matrix4x4f::RotateXMatrix(DEG2RAD(-15.0f)) *
		matrix4x4f::RotateYMatrix(duration);
	m_model->Render(trans);
}
