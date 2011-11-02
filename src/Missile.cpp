#include "Missile.h"
#include "Serializer.h"
#include "Space.h"
#include "Sfx.h"
#include "ShipType.h"
#include "Lang.h"

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

void Missile::PostLoadFixup()
{
	Ship::PostLoadFixup();
	m_owner = Serializer::LookupBody(m_ownerIndex);
	m_target = Serializer::LookupBody(m_targetIndex);
}

void Missile::Save(Serializer::Writer &wr)
{
	Ship::Save(wr);
	wr.Int32(Serializer::LookupBody(m_owner));
	wr.Int32(Serializer::LookupBody(m_target));
	wr.Double(m_distToTarget);
	wr.Int32(m_power);
}

void Missile::Load(Serializer::Reader &rd)
{
	Ship::Load(rd);
	m_ownerIndex = rd.Int32();
	m_targetIndex = rd.Int32();
	m_distToTarget = rd.Double();
	m_power = rd.Int32();
}

void Missile::TimeStepUpdate(const float timeStep)
{
	Ship::TimeStepUpdate(timeStep);

	double dist = (GetPosition() - m_target->GetPosition()).Length();
	if ((m_distToTarget < 150.0) && (dist > m_distToTarget)) {
		Explode();
	}
	m_distToTarget = dist;
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
	Space::KillBody(this);
	Space::RadiusDamage(m_owner, GetFrame(), GetPosition(), 200.0f, 10000.0f);
	Sfx::Add(this, Sfx::TYPE_EXPLOSION);
}

void Missile::NotifyDeleted(const Body* const deletedBody)
{
	if (m_owner == deletedBody) {
		Explode();
	}
	else if (m_target == deletedBody) {
		Explode();
	}
}

