// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "NavLights.h"

#include "FileSystem.h"
#include "GameSaveError.h"
#include "JsonUtils.h"
#include "core/IniConfig.h"
#include "graphics/RenderState.h"
#include "graphics/TextureBuilder.h"
#include "scenegraph/FindNodeVisitor.h"
#include "scenegraph/SceneGraph.h"
#include "utils.h"

const float BILLBOARD_SIZE = 2.5f;

static RefCountedPtr<Graphics::Texture> texHalos4x4;
static RefCountedPtr<Graphics::Material> matHalos4x4;

static bool g_initted = false;
static vector2f m_lightColorsUVoffsets[(NavLights::NAVLIGHT_YELLOW + 1)] = {
	vector2f(0.0f, 0.0f),
	vector2f(0.5f, 0.0f),
	vector2f(0.0f, 0.5f),
	vector2f(0.5f, 0.5f)
};

static vector2f get_color(Uint8 c)
{
	return m_lightColorsUVoffsets[c];
}

static inline vector2f LoadLightColorUVoffset(const std::string &spec)
{
	std::vector<float> v;
	// parse float values in the spec
	SplitString(spec, ",").to_vector(v, [](std::string_view str) {
		return std::atof(std::string(str).c_str());
	});

	// ensure the spec has enough values
	v.resize(2);
	return vector2f(v[0], v[1]);
}

NavLights::LightBulb::LightBulb(Uint8 _group, Uint8 _mask, Uint8 _color, SceneGraph::Billboard *_bb) :
	group(_group),
	mask(_mask),
	color(_color),
	billboard(_bb)
{
}

void NavLights::Init(Graphics::Renderer *renderer)
{
	PROFILE_SCOPED()
	assert(!g_initted);

	IniConfig cfg;
	// set defaults
	cfg.SetString("LeftOrOccupiedUVOffset", "0,0");
	cfg.SetString("RightOrFreeUVOffset", "0.5,0");
	cfg.SetString("StaticUVOffset", "0,0.5");
	cfg.SetString("DockingUVOffset", "0.5,0.5");
	// load
	cfg.Read(FileSystem::gameDataFiles, "textures/NavLights.ini");

	m_lightColorsUVoffsets[NAVLIGHT_RED] = LoadLightColorUVoffset(cfg.String("LeftOrOccupiedUVOffset"));
	m_lightColorsUVoffsets[NAVLIGHT_GREEN] = LoadLightColorUVoffset(cfg.String("RightOrFreeUVOffset"));
	m_lightColorsUVoffsets[NAVLIGHT_BLUE] = LoadLightColorUVoffset(cfg.String("StaticUVOffset"));
	m_lightColorsUVoffsets[NAVLIGHT_YELLOW] = LoadLightColorUVoffset(cfg.String("DockingUVOffset"));

	const std::string texPath = cfg.String("NavLightsTexture");
	texHalos4x4.Reset(Graphics::TextureBuilder::Billboard(texPath).GetOrCreateTexture(renderer, std::string("billboard")));

	Graphics::MaterialDescriptor desc;
	desc.effect = Graphics::EFFECT_BILLBOARD_ATLAS;
	desc.textures = 1;

	Graphics::RenderStateDesc rsd;
	rsd.blendMode = Graphics::BLEND_ADDITIVE;
	rsd.depthWrite = false;
	rsd.primitiveType = Graphics::POINTS;

	matHalos4x4.Reset(renderer->CreateMaterial("billboards", desc, rsd));
	matHalos4x4->SetTexture("texture0"_hash, texHalos4x4.Get());
	matHalos4x4->SetPushConstant("coordDownScale"_hash, 0.5f);

	g_initted = true;
}

void NavLights::Uninit()
{
	assert(g_initted);

	matHalos4x4.Reset();
	texHalos4x4.Reset();

	g_initted = false;
}

NavLights::NavLights(SceneGraph::Model *model, float period) :
	m_time(0.f),
	m_period(period),
	m_enabled(false),
	// NB - we're (ab)using the normal type to hold (uv coordinate offset value + point size)
	m_billboardTris(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_NORMAL)
{
	PROFILE_SCOPED();
	assert(g_initted);

	Graphics::Renderer *renderer = model->GetRenderer();

	using SceneGraph::Billboard;
	using SceneGraph::MatrixTransform;
	using SceneGraph::Node;

	//This will find all matrix transforms meant for navlights.
	SceneGraph::FindNodeVisitor lightFinder(SceneGraph::FindNodeVisitor::MATCH_NAME_STARTSWITH, "navlight_");
	model->GetRoot()->Accept(lightFinder);
	const std::vector<Node *> &results = lightFinder.GetResults();

	//attach light billboards
	for (unsigned int i = 0; i < results.size(); i++) {
		MatrixTransform *mt = dynamic_cast<MatrixTransform *>(results.at(i));
		assert(mt);
		Billboard *bblight = new Billboard(m_billboardTris, renderer, BILLBOARD_SIZE);
		Uint32 group = 0;
		Uint8 mask = 0xff; //always on
		Uint8 color = NAVLIGHT_BLUE;

		if (mt->GetName().substr(9, 3) == "red") {
			mask = 0x0f;
			color = NAVLIGHT_RED;
		} else if (mt->GetName().substr(9, 5) == "green") {
			mask = 0xf0;
			color = NAVLIGHT_GREEN;
		} else if (mt->GetName().substr(9, 3) == "pad") {
			//group by pad number
			// due to this problem: http://stackoverflow.com/questions/15825254/why-is-scanfhhu-char-overwriting-other-variables-when-they-are-local
			// where MSVC is still using a C89 compiler the format identifier %hhu is not recognised. Therefore I've switched to Uint32 for group.
			PiVerify(1 == sscanf(mt->GetName().c_str(), "navlight_pad%u", &group));
			mask = 0xf8;
		}
		bblight->SetColorUVoffset(get_color(color));

		// automagically create a new group if one doesn't exist.
		m_groupLights[group].push_back(LightBulb(group, mask, color, bblight));
		mt->SetNodeMask(SceneGraph::NODE_TRANSPARENT);
		mt->AddChild(bblight);
	}
}

NavLights::~NavLights()
{
}

void NavLights::SaveToJson(Json &jsonObj)
{
	Json navLightsObj = Json::object(); // Create JSON object to contain nav lights data.

	navLightsObj["time"] = m_time;
	navLightsObj["enabled"] = m_enabled;

	jsonObj["nav_lights"] = navLightsObj; // Add nav lights object to supplied object.
}

void NavLights::LoadFromJson(const Json &jsonObj)
{
	try {
		Json navLightsObj = jsonObj["nav_lights"];

		m_time = navLightsObj["time"];
		m_enabled = navLightsObj["enabled"];
	} catch (Json::type_error &) {
		throw SavedGameCorruptException();
	}
}

void NavLights::Update(float time)
{
	PROFILE_SCOPED();
	if (!m_enabled) {
		for (const auto &group : m_groupLights)
			for (const LightBulb &light : group.second)
				light.billboard->SetNodeMask(0x0);
		return;
	}

	m_time += time;

	const int phase((fmod(m_time, m_period) / m_period) * 8);
	const Uint8 mask = 1 << phase;

	for (const auto &pair : m_groupLights) {
		for (const LightBulb &light : pair.second) {
			if (light.mask & mask && light.color != LightColor::NAVLIGHT_OFF)
				light.billboard->SetNodeMask(SceneGraph::NODE_TRANSPARENT);
			else
				light.billboard->SetNodeMask(0x0);
		}
	}
}

void NavLights::Render(Graphics::Renderer *renderer)
{
	if (!m_billboardTris.IsEmpty()) {
		renderer->SetTransform(matrix4x4f::Identity());
		renderer->DrawBuffer(&m_billboardTris, matHalos4x4.Get());
		renderer->GetStats().AddToStatCount(Graphics::Stats::STAT_BILLBOARD, m_billboardTris.GetNumVerts());

		m_billboardTris.Clear();
	}
}

void NavLights::SetColor(unsigned int group, LightColor c)
{
	if (!m_groupLights.count(group)) return;
	for (LightBulb &light : m_groupLights[group]) {
		if (light.group != group || light.color == c) continue;
		if (c != LightColor::NAVLIGHT_OFF)
			light.billboard->SetColorUVoffset(get_color(c));

		light.color = c;
	}
}

void NavLights::SetMask(unsigned int group, uint8_t mask)
{
	if (!m_groupLights.count(group)) return;
	for (LightBulb &light : m_groupLights[group]) {
		light.mask = mask;
	}
}
