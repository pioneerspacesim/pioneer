#include "Intro.h"
#include "libs.h"
#include "Background.h"
#include "Pi.h" //for modelcache
#include "ModelCache.h"
#include "graphics/Renderer.h"
#include "graphics/Texture.h"
#include "graphics/TextureBuilder.h"
#include "newmodel/NModel.h"

static Color4ub random_color()
{
	return Color4ub(Pi::rng.Int32(255), Pi::rng.Int32(255), Pi::rng.Int32(255), 255);
}

Intro::Intro(Graphics::Renderer *r, const vector2f &size)
: m_renderer(r)
, m_aspectRatio(size.x/size.y)
, m_ambientColor(Color4f::BLACK)
{
	m_background = new Background::Container(r, UNIVERSE_SEED);
	Newmodel::NModel *model = Pi::modelCache->FindModel("test_cobra");
	std::vector<Color4ub> shipColors;
	shipColors.push_back(random_color());
	shipColors.push_back(random_color());
	shipColors.push_back(random_color());
	model->SetColors(r, shipColors);
	assert(model->GetPatterns().size() > 1);
	model->SetPattern(1);
	Graphics::Texture *decal = Graphics::TextureBuilder(
        "icons/badge.png",
        Graphics::LINEAR_CLAMP,
        true, true, false).GetOrCreateTexture(r, "model");
	model->SetDecalTexture(decal);
	model->SetLabel("PIONEER");
	m_model = model;

	const Color lc(1.f, 1.f, 1.f, 0.f);
	Graphics::Light light(Graphics::Light::LIGHT_DIRECTIONAL, vector3f(0.f, 1.f, 1.f), lc, lc, lc);
	m_lights.push_back(light);
}

Intro::~Intro()
{
	delete m_background;
}

void Intro::Render(float time)
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	m_renderer->BeginFrame();
	m_renderer->SetPerspectiveProjection(35, m_aspectRatio, 1.f, 10000.f);
	m_renderer->SetTransform(matrix4x4f::Identity());

	//Old junk from Pi::draw_intro
#if 0
	LmrMaterial m0 = { { .2f, .2f, .5f, 1.0f }, { 1, 1, 1 }, { 0, 0, 0 }, 100.0 };
	LmrMaterial m1 = { { 0.5f, 0.5f, 0.5f, 1.0f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 };
	LmrMaterial m2 = { { 0.8f, 0.8f, 0.8f, 1.0f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 };

	LmrObjParams params;
	params.animationNamespace = "ShipAnimation";
	params.time = 0.0;
	params.animValues[1] = 0.0;
	params.label = Lang::PIONEER;
	params.equipment = 0;
	params.flightState = Ship::FLYING;
	params.linthrust[0] = 0.0f; params.linthrust[1] = 0.0f; params.linthrust[2] = -1.0f;
	params.angthrust[0] = 0.0f; params.angthrust[1] = 0.0f; params.angthrust[2] = 0.0f;
	params.pMat[0] = m0;
	params.pMat[1] = m1;
	params.pMat[2] = m2;

	EquipSet equipment;
	// The finest parts that money can buy!
	params.equipment = &equipment;
	equipment.Add(Equip::ECM_ADVANCED, 1);
	equipment.Add(Equip::HYPERCLOUD_ANALYZER, 1);
	equipment.Add(Equip::ATMOSPHERIC_SHIELDING, 1);
	equipment.Add(Equip::FUEL_SCOOP, 1);
	equipment.Add(Equip::SCANNER, 1);
	equipment.Add(Equip::RADAR_MAPPER, 1);
	equipment.Add(Equip::MISSILE_NAVAL, 4);
#endif
	LmrObjParams params;
	// XXX all this stuff will be gone when intro uses a Camera
	// rotate background by time, and a bit extra Z so it's not so flat
	matrix4x4d brot = matrix4x4d::RotateXMatrix(-0.25 * time) * matrix4x4d::RotateZMatrix(0.6);
	m_background->Draw(m_renderer, brot);

	m_renderer->SetAmbientColor(m_ambientColor);
	Pi::renderer->SetLights(m_lights.size(), &m_lights[0]);

	matrix4x4f rot = matrix4x4f::RotateYMatrix(time) * matrix4x4f::RotateZMatrix(0.6f * time) *
			matrix4x4f::RotateXMatrix(time * 0.7f);
	rot[14] = -80.0;

	m_model->Render(m_renderer, rot, &params);

	m_renderer->EndFrame();
	glPopAttrib();
}