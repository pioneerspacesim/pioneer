#include "Missile.h"
#include "Serializer.h"
#include "Space.h"
#include "Sfx.h"
#include "ShipType.h"
#include "Lang.h"
#include "Pi.h"
#include "Game.h"

Missile::Missile(ShipType::Type type, Body *owner, Body *target): Ship(type)
{
	m_power = 0;
	if (type == ShipType::MISSILE_GUIDED) m_power = 1;
	if (type == ShipType::MISSILE_SMART) m_power = 2;
	if (type == ShipType::MISSILE_NAVAL) m_power = 3;

	m_owner = owner;
	m_target = target;
	m_distToTarget = FLT_MAX;
	SetLabel(Lang::MISSILE);

	AIKamikaze(target);
}

void Missile::ECMAttack(int power_val)
{
	if (power_val > m_power) {
		OnDamage(0, 1.0f);
	}
}

void Missile::PostLoadFixup(Space *space)
{
	Ship::PostLoadFixup(space);
	m_owner = space->GetBodyByIndex(m_ownerIndex);
	m_target = space->GetBodyByIndex(m_targetIndex);
}

void Missile::Save(Serializer::Writer &wr, Space *space)
{
	Ship::Save(wr, space);
	wr.Int32(space->GetIndexForBody(m_owner));
	wr.Int32(space->GetIndexForBody(m_target));
	wr.Double(m_distToTarget);
	wr.Int32(m_power);
}

void Missile::Load(Serializer::Reader &rd, Space *space)
{
	Ship::Load(rd, space);
	m_ownerIndex = rd.Int32();
	m_targetIndex = rd.Int32();
	m_distToTarget = rd.Double();
	m_power = rd.Int32();
}

void Missile::TimeStepUpdate(const float timeStep)
{
	Ship::TimeStepUpdate(timeStep);
	
	if (!m_target || !m_owner) {
		Explode();
	} else {
		double dist = (GetPosition() - m_target->GetPosition()).Length();
		if ((m_distToTarget < 150.0) && (dist > m_distToTarget)) {
			Explode();
		}
		m_distToTarget = dist;
	}
}

bool Missile::OnCollision(Object *o, Uint32 flags, double relVel)
{
	if (!IsDead()) {
		Explode();
	}
	return true;
}

bool Missile::OnDamage(Object *attacker, float kgDamage)
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

	for (Space::BodyIterator i = Pi::game->GetSpace()->BodiesBegin(); i != Pi::game->GetSpace()->BodiesEnd(); ++i) {
		if ((*i)->GetFrame() != GetFrame()) continue;
		double dist = ((*i)->GetPosition() - GetPosition()).Length();
		if (dist < damageRadius) {
			// linear damage decay with distance
			(*i)->OnDamage(m_owner, kgDamage * (damageRadius - dist) / damageRadius);
			if ((*i)->IsType(Object::SHIP))
				Pi::luaOnShipHit->Queue(dynamic_cast<Ship*>(*i), m_owner);
		}
	}

	Sfx::Add(this, Sfx::TYPE_EXPLOSION);
}

void Missile::NotifyRemoved(const Body* const removedBody)
{
	if (m_owner == removedBody) {
		m_owner = 0;
	}
	else if (m_target == removedBody) {
		m_target = 0;
	}
}

