#include "Missile.h"
#include "Serializer.h"
#include "Space.h"
#include "Sfx.h"
#include "ShipType.h"

Missile::Missile(ShipType::Type type, Body *owner, Body *target): Ship(type)
{
	switch (type) {
		case ShipType::MISSILE_GUIDED: m_power = 1; break;
		case ShipType::MISSILE_SMART: m_power = 2; break;
		case ShipType::MISSILE_NAVAL: m_power = 3; break;
		default:
		case ShipType::MISSILE_UNGUIDED: m_power = 0; break;
	};
	m_owner = owner;
	m_target = target;
	m_distToTarget = FLT_MAX;
	SetLabel("missile");

	AIInstruct(DO_KAMIKAZE, target);
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
	m_owner = Serializer::LookupBody((size_t)m_owner);
	m_target = Serializer::LookupBody((size_t)m_target);
}

void Missile::Save()
{
	using namespace Serializer::Write;
	Ship::Save();
	wr_int(Serializer::LookupBody(m_owner));
	wr_int(Serializer::LookupBody(m_target));
	wr_double(m_distToTarget);
	wr_int(m_power);
}

void Missile::Load()
{
	using namespace Serializer::Read;
	Ship::Load();
	m_owner = (Body*)rd_int();
	m_target = (Body*)rd_int();
	m_distToTarget = rd_double();
	m_power = rd_int();
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

