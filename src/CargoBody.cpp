// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "CargoBody.h"

#include "Game.h"
#include "GameSaveError.h"
#include "JsonUtils.h"
#include "Pi.h"
#include "Sfx.h"
#include "Ship.h"
#include "Space.h"
#include "lua/LuaEvent.h"
#include "lua/LuaTable.h"

CargoBody::CargoBody(const LuaRef &cargo, float selfdestructTimer) :
	m_cargo(cargo)
{
	SetModel("cargo");
	Init();
	SetMass(1.0);
	m_selfdestructTimer = selfdestructTimer; // number of seconds to live

	if (is_zero_exact(selfdestructTimer)) // turn off self destruct
		m_hasSelfdestruct = false;
}

CargoBody::CargoBody(const char *modelName, const LuaRef &cargo, float selfdestructTimer) :
	m_cargo(cargo)
{
	SetModel(modelName);
	Init();
	SetMass(1.0);
	m_selfdestructTimer = selfdestructTimer; // number of seconds to live

	if (is_zero_exact(selfdestructTimer)) // turn off self destruct
		m_hasSelfdestruct = false;
}

CargoBody::CargoBody(const Json &jsonObj, Space *space) :
	DynamicBody(jsonObj, space)
{
	GetModel()->SetLabel(GetLabel());

	try {
		Json cargoBodyObj = jsonObj["cargo_body"];

		m_cargo.LoadFromJson(cargoBodyObj);
		Init();
		m_hitpoints = cargoBodyObj["hit_points"];
		m_selfdestructTimer = cargoBodyObj["self_destruct_timer"];
		m_hasSelfdestruct = cargoBodyObj["has_self_destruct"];
	} catch (Json::type_error &) {
		throw SavedGameCorruptException();
	}
}

void CargoBody::SaveToJson(Json &jsonObj, Space *space)
{
	DynamicBody::SaveToJson(jsonObj, space);

	Json cargoBodyObj = Json::object(); // Create JSON object to contain cargo body data.

	m_cargo.SaveToJson(cargoBodyObj);
	cargoBodyObj["hit_points"] = m_hitpoints;
	cargoBodyObj["self_destruct_timer"] = m_selfdestructTimer;
	cargoBodyObj["has_self_destruct"] = m_hasSelfdestruct;

	jsonObj["cargo_body"] = cargoBodyObj; // Add cargo body object to supplied object.
}

void CargoBody::Init()
{
	m_hitpoints = 1.0f;
	std::string cargoname = ScopedTable(m_cargo).CallMethod<std::string>("GetName"); // instead of switching to lua twice for the same value
	SetLabel(cargoname);
	SetMassDistributionFromModel();
	m_hasSelfdestruct = true;

	std::vector<Color> colors;
	//metallic blue-orangeish color scheme
	colors.push_back(Color(255, 198, 64));
	colors.push_back(Color(0, 222, 255));
	colors.push_back(Color(255, 255, 255));

	SceneGraph::ModelSkin skin;
	skin.SetColors(colors);
	skin.SetDecal("pioneer");
	skin.Apply(GetModel());
	GetModel()->SetColors(colors);
}

void CargoBody::TimeStepUpdate(const float timeStep)
{

	// Suggestion: since cargo doesn't need thrust or AI, it could be
	// converted into an idle object on orbital rails, set up to only take
	// memory & save file space (not CPU power) when far from the player.
	// Until then, we kill it after some time, to not clutter up the current
	// star system.

	if (m_hasSelfdestruct) {
		m_selfdestructTimer -= timeStep;
		if (m_selfdestructTimer <= 0) {
			LuaEvent::Queue("onCargoDestroyed", this);
			Pi::game->GetSpace()->KillBody(this);
			SfxManager::Add(this, TYPE_EXPLOSION);
		}
	}
	DynamicBody::TimeStepUpdate(timeStep);
}

bool CargoBody::OnDamage(Body *attacker, float kgDamage, const CollisionContact &contactData)
{
	m_hitpoints -= kgDamage * 0.001f;
	if (m_hitpoints < 0) {
		if (attacker && attacker->IsType(ObjectType::BODY))
			LuaEvent::Queue("onCargoDestroyed", this, dynamic_cast<Body *>(attacker));
		else
			LuaEvent::Queue("onCargoDestroyed", this);
		Pi::game->GetSpace()->KillBody(this);
		SfxManager::Add(this, TYPE_EXPLOSION);
	}
	return true;
}

bool CargoBody::OnCollision(Body *b, Uint32 flags, double relVel)
{
	// ignore collision if its about to be scooped
	if (b->IsType(ObjectType::SHIP)) {
		int cargoscoop_cap = b->Properties().Get("cargo_scoop_cap");
		if (cargoscoop_cap > 0) {
			LuaEvent::Queue("onCargoDestroyed", this);
			return true;
		}
	}

	return DynamicBody::OnCollision(b, flags, relVel);
}

void CargoBody::Render(Graphics::Renderer *r, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform)
{
	RenderModel(r, camera, viewCoords, viewTransform);
}

void CargoBody::SetLabel(const std::string &label)
{
	assert(GetModel());
	GetModel()->SetLabel(label);
	Body::SetLabel(label);
}
