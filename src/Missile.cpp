// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Missile.h"
#include "Serializer.h"
#include "Space.h"
#include "Sfx.h"
#include "ShipType.h"
#include "Lang.h"
#include "Pi.h"
#include "Game.h"
#include "LuaEvent.h"

Missile::Missile(ShipType::Id shipId, Body *owner, int power): Ship(shipId)
{
	if (power < 0) {
		m_power = 0;
		if (shipId == ShipType::MISSILE_GUIDED) m_power = 1;
		if (shipId == ShipType::MISSILE_SMART) m_power = 2;
		if (shipId == ShipType::MISSILE_NAVAL) m_power = 3;
	} else
		m_power = power;

	m_owner = owner;
	SetLabel(Lang::MISSILE);
	Disarm();
}

void Missile::ECMAttack(int power_val)
{
	if (power_val > m_power) {
		CollisionContact dummy;
		OnDamage(0, 1.0f, dummy);
	}
}

void Missile::PostLoadFixup(Space *space)
{
	Ship::PostLoadFixup(space);
	m_owner = space->GetBodyByIndex(m_ownerIndex);
}

void Missile::SaveToJson(Json::Value &jsonObj, Space *space)
{
	Ship::SaveToJson(jsonObj, space);

	Json::Value missileObj(Json::objectValue); // Create JSON object to contain missile data.

	missileObj["index_for_body"] = space->GetIndexForBody(m_owner);
	missileObj["power"] = m_power;
	missileObj["armed"] = m_armed;

	jsonObj["missile"] = missileObj; // Add missile object to supplied object.
}

void Missile::LoadFromJson(const Json::Value &jsonObj, Space *space)
{
	Ship::LoadFromJson(jsonObj, space);

	if (!jsonObj.isMember("missile")) throw SavedGameCorruptException();
	Json::Value missileObj = jsonObj["missile"];

	if (!missileObj.isMember("index_for_body")) throw SavedGameCorruptException();
	if (!missileObj.isMember("power")) throw SavedGameCorruptException();
	if (!missileObj.isMember("armed")) throw SavedGameCorruptException();

	m_ownerIndex = missileObj["index_for_body"].asUInt();
	m_power = missileObj["power"].asInt();
	m_armed = missileObj["armed"].asBool();
}

void Missile::TimeStepUpdate(const float timeStep)
{
	Ship::TimeStepUpdate(timeStep);

	const float MISSILE_DETECTION_RADIUS = 100.0f;
	if (!m_owner) {
		Explode();
	} else if (m_armed) {
		Space::BodyNearList nearby;
		Pi::game->GetSpace()->GetBodiesMaybeNear(this, MISSILE_DETECTION_RADIUS, nearby);
		for (Space::BodyNearIterator i = nearby.begin(); i != nearby.end(); ++i) {
			if (*i == this) continue;
			double dist = ((*i)->GetPosition() - GetPosition()).Length();
			if (dist < MISSILE_DETECTION_RADIUS) {
				Explode();
				break;
			}
		}
	}
}

bool Missile::OnCollision(Object *o, Uint32 flags, double relVel)
{
	if (!IsDead()) {
		Explode();
	}
	return true;
}

bool Missile::OnDamage(Object *attacker, float kgDamage, const CollisionContact& contactData)
{
	if (!IsDead()) {
		Explode();
	}
	return true;
}

void Missile::Explode()
{
	Pi::game->GetSpace()->KillBody(this);

	const double damageRadius = 200.0;
	const double kgDamage = 10000.0;

	CollisionContact dummy;
	Space::BodyNearList nearby;
	Pi::game->GetSpace()->GetBodiesMaybeNear(this, damageRadius, nearby);
	for (Space::BodyNearIterator i = nearby.begin(); i != nearby.end(); ++i) {
		if ((*i)->GetFrame() != GetFrame()) continue;
		double dist = ((*i)->GetPosition() - GetPosition()).Length();
		if (dist < damageRadius) {
			// linear damage decay with distance
			(*i)->OnDamage(m_owner, kgDamage * (damageRadius - dist) / damageRadius, dummy);
			if ((*i)->IsType(Object::SHIP))
				LuaEvent::Queue("onShipHit", dynamic_cast<Ship*>(*i), m_owner);
		}
	}

	Sfx::Add(this, Sfx::TYPE_EXPLOSION);
}

void Missile::NotifyRemoved(const Body* const removedBody)
{
	if (m_owner == removedBody) {
		m_owner = 0;
	}
}

void Missile::Arm()
{
	m_armed = true;
	Properties().Set("isArmed", true);
}

void Missile::Disarm()
{
	m_armed = false;
	Properties().Set("isArmed", false);
}
