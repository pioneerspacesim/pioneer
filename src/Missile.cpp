#include "Missile.h"
#include "Serializer.h"
#include "Space.h"
#include "Sfx.h"
#include "ShipType.h"

Missile::Missile(ShipType::Type type, Body *owner, Body *target): Ship(type)
{
	m_owner = owner;
	m_target = target;
	m_distToTarget = FLT_MAX;
	SetLabel("missile");

	AIInstruct(DO_KAMIKAZE, target);
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
}

void Missile::Load()
{
	using namespace Serializer::Read;
	Ship::Load();
	m_owner = (Body*)rd_int();
	m_target = (Body*)rd_int();
	m_distToTarget = rd_double();
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

void Missile::NotifyDeath(const Body* const dyingBody)
{
	if (m_owner == dyingBody) {
		Explode();
	}
	else if (m_target == dyingBody) {
		Explode();
	}
}

