// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Missile.h"

#include "Game.h"
#include "Lang.h"
#include "Json.h"
#include "Pi.h"
#include "Sfx.h"
#include "Ship.h"
#include "ShipAICmd.h"
#include "Space.h"
#include "collider/CollisionContact.h"
#include "lua/LuaEvent.h"
#include "ship/Propulsion.h"

// For debugging
// #include "core/Log.h"
// #include "EnumStrings.h"

Missile::Missile(Body *owner, const MissileDef &mdef)
{
	m_owner = owner;
	m_missileStats = mdef;
	m_propulsion = AddComponent<Propulsion>();

	m_type = &ShipType::types[mdef.shipType.c_str()];
	Init();
}

void Missile::Init()
{
	SetMass(m_type->hullMass * 1000);

	SetModel(m_type->modelName.c_str());
	SetMassDistributionFromModel();

	SetLabel(Lang::MISSILE);

	Disarm();

	m_propulsion->SetFuel(1.0);
	m_propulsion->SetFuelReserve(0.0);

	m_curAICmd = 0;
	m_aiMessage = AIERROR_NONE;
	m_decelerating = false;

	m_propulsion->Init(this, GetModel(), m_type->fuelTankMass, m_type->effectiveExhaustVelocity, m_type->linThrust, m_type->angThrust);
}

Missile::Missile(const Json &jsonObj, Space *space) :
	DynamicBody(jsonObj, space)
{
	m_propulsion = AddComponent<Propulsion>();
	m_propulsion->LoadFromJson(jsonObj, space);
	Json missileObj = jsonObj["missile"];

	try {
		m_missileStats.shipType = missileObj["ship_type_id"].get<std::string_view>();
		m_missileStats.fuzeRadius = missileObj["fuze_radius"];
		m_missileStats.warheadSize = missileObj["warhead_size"];
		m_missileStats.effectiveRadius = missileObj["effective_radius"];
		m_missileStats.chargeEffectiveness = missileObj["shaped_charge"];
		m_missileStats.ecmResist = missileObj["power"];

		m_type = &ShipType::types[m_missileStats.shipType.c_str()];
		SetModel(m_type->modelName.c_str());

		m_curAICmd = 0;
		m_curAICmd = AICommand::LoadFromJson(missileObj);
		m_aiMessage = AIError(missileObj["ai_message"]);

		m_ownerIndex = missileObj["index_for_body"];
		m_armed = missileObj["armed"];
	} catch (Json::type_error &) {
		throw SavedGameCorruptException();
	}

	m_propulsion->Init(this, GetModel(), m_type->fuelTankMass, m_type->effectiveExhaustVelocity, m_type->linThrust, m_type->angThrust);
}

void Missile::SaveToJson(Json &jsonObj, Space *space)
{
	DynamicBody::SaveToJson(jsonObj, space);
	m_propulsion->SaveToJson(jsonObj, space);
	Json missileObj = Json::object(); // Create JSON object to contain missile data.

	if (m_curAICmd) m_curAICmd->SaveToJson(missileObj);

	missileObj["ai_message"] = int(m_aiMessage);
	missileObj["index_for_body"] = space->GetIndexForBody(m_owner);

	missileObj["ship_type_id"] = m_missileStats.shipType.sv();
	missileObj["fuze_radius"] = m_missileStats.fuzeRadius;
	missileObj["warhead_size"] = m_missileStats.warheadSize;
	missileObj["effective_radius"] = m_missileStats.effectiveRadius;
	missileObj["shaped_charge"] = m_missileStats.chargeEffectiveness;
	missileObj["power"] = m_missileStats.ecmResist;

	missileObj["armed"] = m_armed;
	missileObj["ship_type_id"] = m_type->id;

	jsonObj["missile"] = missileObj; // Add missile object to supplied object.
}

void Missile::PostLoadFixup(Space *space)
{
	DynamicBody::PostLoadFixup(space);
	m_owner = space->GetBodyByIndex(m_ownerIndex);
	if (m_curAICmd) m_curAICmd->PostLoadFixup(space);
}

Missile::~Missile()
{
	if (m_curAICmd) delete m_curAICmd;
}

void Missile::ECMAttack(int power_val)
{
	if (power_val > m_missileStats.ecmResist) {
		CollisionContact dummy;
		OnDamage(0, 1.0f, dummy);
	}
}

void Missile::StaticUpdate(const float timeStep)
{
	// Note: direct call to AI->TimeStepUpdate

	if (!m_curAICmd) {
		m_propulsion->ClearLinThrusterState();
		m_propulsion->ClearAngThrusterState();
	} else if (m_curAICmd->TimeStepUpdate()) {
		delete m_curAICmd;
		m_curAICmd = nullptr;
	}
	//Add smoke trails for missiles on thruster state
	static double s_timeAccum = 0.0;
	s_timeAccum += timeStep;
	if (!is_equal_exact(m_propulsion->GetLinThrusterState().LengthSqr(), 0.0) && (s_timeAccum > 4 || 0.1 * Pi::rng.Double() < timeStep)) {
		s_timeAccum = 0.0;
		const vector3d pos = GetOrient() * vector3d(0, 0, 5);
		const float speed = std::min(10.0 * GetVelocity().Length() * std::max(1.0, fabs(m_propulsion->GetLinThrusterState().z)), 100.0);
		SfxManager::AddThrustSmoke(this, speed, pos);
	}
}

bool Missile::IsValidTarget(const Body *body)
{
	switch (body->GetType()) {
	case ObjectType::MODELBODY:
	case ObjectType::CARGOBODY:
	case ObjectType::TERRAINBODY:
	case ObjectType::PLANET:
	case ObjectType::SHIP:
	case ObjectType::SPACESTATION:
	case ObjectType::PLAYER:
		return true;
	default:
		return false;
	}
}

void Missile::TimeStepUpdate(const float timeStep)
{

	const vector3d thrust = m_propulsion->GetActualLinThrust();
	AddRelForce(thrust);
	AddRelTorque(m_propulsion->GetActualAngThrust());

	DynamicBody::TimeStepUpdate(timeStep);
	m_propulsion->UpdateFuel(timeStep);

	const float MISSILE_DETECTION_RADIUS = m_missileStats.fuzeRadius;
	const float MISSILE_TRIGGER_RADIUS = 10.0f;

	const Body *target = GetTarget();

	if (!m_owner) {
		Explode();
	} else if (m_armed) {
		Space::BodyNearList nearby = Pi::game->GetSpace()->GetBodiesMaybeNear(this, MISSILE_DETECTION_RADIUS);
		for (Body *body : nearby) {
			if (body == this) continue;

			if (body != target && !IsValidTarget(body))
				continue;

			// Explode only when we've gotten as close as we possibly can to the target - if we start moving away then trigger an explosion immediately
			double dist = (body->GetPosition() - GetPosition()).Length();
			const bool trigger = dist < MISSILE_DETECTION_RADIUS && body->GetVelocityRelTo(GetFrame()).Dot(GetVelocity()) < 0.0;

			if (trigger || dist < MISSILE_TRIGGER_RADIUS) {
				Explode();
				break;
			}
		}
	}
}

bool Missile::OnCollision(Body *o, Uint32 flags, double relVel)
{
	if (!IsDead()) {
		Explode();
	}
	return true;
}

bool Missile::OnDamage(Body *attacker, float kgDamage, const CollisionContact &contactData)
{
	if (!IsDead()) {
		Explode();
	}
	return true;
}

double calcAreaSphere(const double r)
{
	return 4.0 * M_PI * r * r;
}

double calcAreaCircle(const double r)
{
	return M_PI * r * r;
}

void Missile::Explode()
{
	Pi::game->GetSpace()->KillBody(this);

	// defaults to 2 km, this is sufficient for most explosions
	double queryRadius = m_missileStats.effectiveRadius;

	CollisionContact dummy;
	Space::BodyNearList nearby = Pi::game->GetSpace()->GetBodiesMaybeNear(this, queryRadius);
	for (Body *body : nearby) {
		if (body->GetType() == ObjectType::PROJECTILE)
			continue; // early-out over projectiles, we can't actually damage them

		const double distSqr = (body->GetPosition() - GetPosition()).LengthSqr();
		if (body->GetFrame() != GetFrame() || body == this || distSqr >= queryRadius * queryRadius)
			continue;
		const double dist = (body->GetPosition() - GetPosition()).Length(); // distance from explosion in meter
		const double targetRadius = body->GetPhysRadius();					// radius of the hit target in meter

		const double areaSphere = calcAreaSphere(std::max(0.0, dist - targetRadius));
		const double crossSectionTarget = calcAreaCircle(targetRadius);
		double ratioArea = crossSectionTarget / areaSphere; // compute ratio of areas to know how much energy was transfered to target

		// We assume missiles with shaped-charge warheads to focus the blast towards the target have the warhead oriented correctly
		if (body == GetTarget())
			ratioArea = ratioArea * m_missileStats.chargeEffectiveness;

		ratioArea = std::min(ratioArea, 1.0); // we must limit received energy to finite amount

		// Compute received energy using warhead size expressed as equivalent to kg of TNT
		const double mjReceivedEnergy = ratioArea * m_missileStats.warheadSize * 4.184;

		double kgDamage = mjReceivedEnergy * 16.18033; // received energy back to damage in pioneer "kg" unit, using Phi*10 because we can
		if (kgDamage < 5.0)
			continue; // early-out if we're dealing a negligable amount of damage

		/*
		Log::Info("Missile impact on {} | {}\n\ttarget.radius={} dist={} sphereArea={} crossSection={} (ratio={}) => received energy {}mj={}kgD\n",
			body->GetLabel(), EnumStrings::GetString("PhysicsObjectType", int(body->GetType())),
			targetRadius, dist, areaSphere, crossSectionTarget, ratioArea, mjReceivedEnergy, kgDamage);
		*/

		body->OnDamage(m_owner, kgDamage, dummy);
		if (body->IsType(ObjectType::SHIP))
			LuaEvent::Queue("onShipHit", dynamic_cast<Ship *>(body), m_owner);
	}

	SfxManager::Add(this, TYPE_EXPLOSION);
}

void Missile::NotifyRemoved(const Body *const removedBody)
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

const Body *Missile::GetTarget() const
{
	if (m_curAICmd) {
		return static_cast<AICmdKamikaze *>(m_curAICmd)->GetTarget();
	}

	return nullptr;
}

void Missile::Render(Graphics::Renderer *renderer, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform)
{
	if (IsDead()) return;

	m_propulsion->Render(renderer, camera, viewCoords, viewTransform);
	RenderModel(renderer, camera, viewCoords, viewTransform);
}

void Missile::AIKamikaze(Body *target)
{
	//AIClearInstructions();
	if (m_curAICmd != 0)
		delete m_curAICmd;
	m_curAICmd = new AICmdKamikaze(this, target);
}
