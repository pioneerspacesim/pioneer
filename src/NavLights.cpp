// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "NavLights.h"
#include "graphics/TextureBuilder.h"
#include "scenegraph/FindNodeVisitor.h"
#include "scenegraph/SceneGraph.h"

const float BILLBOARD_SIZE = 5.f;

static bool g_initted = false;

static RefCountedPtr<Graphics::Material> matWhite;
static RefCountedPtr<Graphics::Material> matRed;
static RefCountedPtr<Graphics::Material> matGreen;
static RefCountedPtr<Graphics::Material> matBlue;
static RefCountedPtr<Graphics::Material> matYellow;

typedef std::vector<NavLights::LightBulb>::iterator LightIterator;

static RefCountedPtr<Graphics::Material> get_material(Uint8 c)
{
	if (c == NavLights::NAVLIGHT_RED)
		return matRed;
	else if (c == NavLights::NAVLIGHT_GREEN)
		return matGreen;
	else if (c == NavLights::NAVLIGHT_YELLOW)
		return matYellow;
	else
		return matBlue;
}

NavLights::LightBulb::LightBulb(Uint8 _group, Uint8 _mask, Uint8 _color, SceneGraph::Billboard *_bb)
: group(_group)
, mask(_mask)
, color(_color)
, billboard(_bb)
{
}

void NavLights::Init(Graphics::Renderer *renderer)
{
	assert(!g_initted);
	Graphics::MaterialDescriptor desc;
	desc.textures = 1;
	matWhite.Reset(renderer->CreateMaterial(desc));
	matRed.Reset(renderer->CreateMaterial(desc));
	matGreen.Reset(renderer->CreateMaterial(desc));
	matBlue.Reset(renderer->CreateMaterial(desc));
	matYellow.Reset(renderer->CreateMaterial(desc));

	//not cached because modelviewer clears everything...
	matWhite->texture0  = Graphics::TextureBuilder::Billboard("textures/halo.png").CreateTexture(renderer);
	matRed->texture0    = Graphics::TextureBuilder::Billboard("textures/halo_red.png").CreateTexture(renderer);
	matGreen->texture0  = Graphics::TextureBuilder::Billboard("textures/halo_green.png").CreateTexture(renderer);
	matBlue->texture0   = Graphics::TextureBuilder::Billboard("textures/halo_blue.png").CreateTexture(renderer);
	matYellow->texture0 = Graphics::TextureBuilder::Billboard("textures/halo_yellow.png").CreateTexture(renderer);

	g_initted = true;
}

void NavLights::Uninit()
{
	assert(g_initted);

	delete matWhite->texture0;
	delete matRed->texture0;
	delete matGreen->texture0;
	delete matBlue->texture0;
	delete matYellow->texture0;

	g_initted = false;
}

NavLights::NavLights(SceneGraph::Model *model, float period)
: m_time(0.f)
, m_period(period)
, m_enabled(false)
{
	PROFILE_SCOPED();
	assert(g_initted);

	Graphics::Renderer *renderer = model->GetRenderer();

	using SceneGraph::Node;
	using SceneGraph::MatrixTransform;
	using SceneGraph::Billboard;

	//This will find all matrix transforms meant for navlights.
	SceneGraph::FindNodeVisitor lightFinder(SceneGraph::FindNodeVisitor::MATCH_NAME_STARTSWITH, "navlight_");
	model->GetRoot()->Accept(lightFinder);
	const std::vector<Node*> &results = lightFinder.GetResults();

	//attach light billboards
	for (unsigned int i=0; i < results.size(); i++) {
		MatrixTransform *mt = dynamic_cast<MatrixTransform*>(results.at(i));
		assert(mt);
		Billboard *bblight = new Billboard(renderer, matBlue, vector3f(0.f), BILLBOARD_SIZE);
		Uint32 group = 0;
		Uint8 mask  = 0xff; //always on
		Uint8 color = NAVLIGHT_BLUE;

		if (mt->GetName().substr(9, 3) == "red") {
			bblight->SetMaterial(matRed);
			mask  = 0x0f;
			color = NAVLIGHT_RED;
		} else if (mt->GetName().substr(9, 5) == "green") {
			mask  = 0xf0;
			color = NAVLIGHT_GREEN;
		} else if (mt->GetName().substr(9, 3) == "pad") {
			//group by pad number
			// due to this problem: http://stackoverflow.com/questions/15825254/why-is-scanfhhu-char-overwriting-other-variables-when-they-are-local
			// where MSVC is still using a C89 compiler the format identifer %hhu is not recognised. Therefore I've switched to Uint32 for group.
			PiVerify(1 == sscanf(mt->GetName().c_str(), "navlight_pad%u", &group));
			mask  = 0xf0;
		}
		bblight->SetMaterial(get_material(color));

		m_lights.push_back(LightBulb(group, mask, color, bblight));
		mt->SetNodeMask(SceneGraph::NODE_TRANSPARENT);
		mt->AddChild(bblight);
	}
}

NavLights::~NavLights()
{
}

void NavLights::SaveToJson(Json::Value &jsonObj)
{
	Json::Value navLightsObj(Json::objectValue); // Create JSON object to contain nav lights data.

	navLightsObj["time"] = FloatToStr(m_time);
	navLightsObj["enabled"] = m_enabled;

	Json::Value lightsArray(Json::arrayValue); // Create JSON array to contain lights data.
	for (LightIterator it = m_lights.begin(); it != m_lights.end(); ++it)
		lightsArray.append(it->color);
	navLightsObj["lights"] = lightsArray; // Add lights array to nav lights object.

	jsonObj["nav_lights"] = navLightsObj; // Add nav lights object to supplied object.
}

void NavLights::LoadFromJson(const Json::Value &jsonObj)
{
	if (!jsonObj.isMember("nav_lights")) throw SavedGameCorruptException();
	Json::Value navLightsObj = jsonObj["nav_lights"];

	if (!navLightsObj.isMember("time")) throw SavedGameCorruptException();
	if (!navLightsObj.isMember("enabled")) throw SavedGameCorruptException();
	if (!navLightsObj.isMember("lights")) throw SavedGameCorruptException();

	m_time = StrToFloat(navLightsObj["time"].asString());
	m_enabled = navLightsObj["enabled"].asBool();

	Json::Value lightsArray = navLightsObj["lights"];
	if (!lightsArray.isArray()) throw SavedGameCorruptException();
	RefCountedPtr<Graphics::Material> mat;
	assert(m_lights.size() == lightsArray.size());
	unsigned int arrayIndex = 0;
	for (LightIterator it = m_lights.begin(); it != m_lights.end(); ++it)
	{
		Uint8 c = lightsArray[arrayIndex++].asUInt();
		it->billboard->SetMaterial(get_material(c));
	}
}

void NavLights::Update(float time)
{
	PROFILE_SCOPED();
	if (!m_enabled) {
		for (LightIterator it = m_lights.begin(); it != m_lights.end(); ++it)
			it->billboard->SetNodeMask(0x0);
		return;
	}

	m_time += time;

	int phase((fmod(m_time, m_period) / m_period) * 8);
	Uint8 mask = 1 << phase;

	for (LightIterator it = m_lights.begin(); it != m_lights.end(); ++it) {
		if (it->mask & mask)
			it->billboard->SetNodeMask(SceneGraph::NODE_TRANSPARENT);
		else
			it->billboard->SetNodeMask(0x0);
	}
}

void NavLights::SetColor(unsigned int group, LightColor c)
{
	PROFILE_SCOPED();
	for (LightIterator it = m_lights.begin(); it != m_lights.end(); ++it) {
		if (it->group != group || it->color == c) continue;
		it->billboard->SetMaterial(get_material(c));
		it->color = c;
	}
}
