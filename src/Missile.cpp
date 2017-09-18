// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
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

Missile::Missile(const ShipType::Id &shipId, Body *owner, int power)//: Ship(shipId)
{
	AddFeature( Feature::PROPULSION ); // add component propulsion
	if (power < 0) {
		m_power = 0;
		if (shipId == ShipType::MISSILE_GUIDED) m_power = 1;
		if (shipId == ShipType::MISSILE_SMART) m_power = 2;
		if (shipId == ShipType::MISSILE_NAVAL) m_power = 3;
	} else
		m_power = power;

	m_owner = owner;
	m_type = &ShipType::types[shipId];

	SetMass(m_type->hullMass*1000);

	SetModel(m_type->modelName.c_str());
	SetMassDistributionFromModel();

	SetLabel(Lang::MISSILE);

	Disarm();

	GetPropulsion()->SetFuel(1.0);
	GetPropulsion()->SetFuelReserve(0.0);

	m_curAICmd = 0;
	m_aiMessage = AIERROR_NONE;
	m_decelerating = false;

	GetPropulsion()->Init( this, GetModel(), m_type->fuelTankMass, m_type->effectiveExhaustVelocity, m_type->linThrust, m_type->angThrust );

}

Missile::~Missile()
{
	if (m_curAICmd) delete m_curAICmd;
}

void Missile::ECMAttack(int power_val)
{
	if (power_val > m_power) {
		CollisionContact dummy;
		OnDamage(0, 1.0f, dummy);
	}
}

void Missile::SaveToJson(Json::Value &jsonObj, Space *space)
{
	DynamicBody::SaveToJson(jsonObj, space);
	GetPropulsion()->SaveToJson(jsonObj, space);
	Json::Value missileObj(Json::objectValue); // Create JSON object to contain missile data.

	if (m_curAICmd) m_curAICmd->SaveToJson(missileObj);

	missileObj["ai_message"] = int(m_aiMessage);
	missileObj["index_for_body"] = space->GetIndexForBody(m_owner);
	missileObj["power"] = m_power;
	missileObj["armed"] = m_armed;
	missileObj["ship_type_id"] = m_type->id;

	jsonObj["missile"] = missileObj; // Add missile object to supplied object.
}

void Missile::LoadFromJson(const Json::Value &jsonObj, Space *space)
{
	DynamicBody::LoadFromJson(jsonObj, space);
	GetPropulsion()->LoadFromJson(jsonObj, space);

	if (!jsonObj.isMember("missile")) throw SavedGameCorruptException();
	Json::Value missileObj = jsonObj["missile"];

	if (!missileObj.isMember("index_for_body")) throw SavedGameCorruptException();
	if (!missileObj.isMember("power")) throw SavedGameCorruptException();
	if (!missileObj.isMember("armed")) throw SavedGameCorruptException();
	if (!missileObj.isMember("ai_message")) throw SavedGameCorruptException();
	if (!missileObj.isMember("ship_type_id")) throw SavedGameCorruptException();

	m_type = &ShipType::types[missileObj["ship_type_id"].asString()];
	SetModel(m_type->modelName.c_str());

	m_curAICmd = 0;
	m_curAICmd = AICommand::LoadFromJson(missileObj);
	m_aiMessage = AIError(missileObj["ai_message"].asInt());

	m_ownerIndex = missileObj["index_for_body"].asUInt();
	m_power = missileObj["power"].asInt();
	m_armed = missileObj["armed"].asBool();

	GetPropulsion()->Init( this, GetModel(), m_type->fuelTankMass, m_type->effectiveExhaustVelocity, m_type->linThrust, m_type->angThrust );

}

void Missile::PostLoadFixup(Space *space)
{
	DynamicBody::PostLoadFixup(space);
	m_owner = space->GetBodyByIndex(m_ownerIndex);
	if (m_curAICmd) m_curAICmd->PostLoadFixup(space);
}

void Missile::StaticUpdate(const float timeStep)
{
	// Note: direct call to AI->TimeStepUpdate

	if (!m_curAICmd) {
		GetPropulsion()->ClearLinThrusterState();
		GetPropulsion()->ClearAngThrusterState();
	}
	else if (m_curAICmd->TimeStepUpdate()) {
		delete m_curAICmd;
		m_curAICmd = nullptr;
	}
	//Add smoke trails for missiles on thruster state
	static double s_timeAccum = 0.0;
	s_timeAccum += timeStep;
	if (!is_equal_exact(GetPropulsion()->GetThrusterState().LengthSqr(), 0.0) && (s_timeAccum > 4 || 0.1*Pi::rng.Double() < timeStep)) {
		s_timeAccum = 0.0;
		const vector3d pos = GetOrient() * vector3d(0, 0 , 5);
		const float speed = std::min(10.0*GetVelocity().Length()*std::max(1.0,fabs(GetPropulsion()->GetThrusterState().z)),100.0);
		SfxManager::AddThrustSmoke(this, speed, pos);
	}
}

void Missile::TimeStepUpdate(const float timeStep)
{

	const vector3d thrust=GetPropulsion()->GetActualLinThrust();
	AddRelForce( thrust );
	AddRelTorque( GetPropulsion()->GetActualAngThrust() );

	DynamicBody::TimeStepUpdate(timeStep);
	GetPropulsion()->UpdateFuel(timeStep);

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

	SfxManager::Add(this, TYPE_EXPLOSION);
}

void Missile::NotifyRemoved(const Body* const removedBody)
{
	if (m_curAICmd) m_curAICmd->OnDeleted(removedBody);
	if (m_owner == removedBody) {
		m_owner = 0;
	}
	DynamicBody::NotifyRemoved(removedBody);
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

void Missile::Render(Graphics::Renderer *renderer, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform)
{
	if (IsDead()) return;

	GetPropulsion()->Render( renderer, camera, viewCoords, viewTransform );
	RenderModel(renderer, camera, viewCoords, viewTransform);
}

void Missile::AIKamikaze(Body *target)
{
	//AIClearInstructions();
	if (m_curAICmd!=0)
		delete m_curAICmd;
	m_curAICmd = new AICmdKamikaze(this, target);
}
