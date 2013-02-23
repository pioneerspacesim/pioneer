// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Weapon.h"
#include "Ship.h"
#include "Polit.h"
#include "Sound.h"
#include "Projectile.h"

Weapon::Weapon(Equip::Type type)
: m_state(0)
, m_recharge(0.f)
, m_temperature(0.f)
, m_coolingRate(0.01f)
, m_coolingMultiplier(1.f)
, m_ship(0)
, m_equipType(type)
{
	m_laserType = Equip::lasers[Equip::types[m_equipType].tableIndex];
}

Weapon::~Weapon()
{
}

void Weapon::Save(Serializer::Writer &wr)
{
	wr.Int32(m_state);
	wr.Float(m_recharge);
	wr.Float(m_temperature);
}

void Weapon::Load(Serializer::Reader &rd)
{
	m_state = rd.Int32();
	m_recharge = rd.Float();
	m_temperature = rd.Float();
}

void Weapon::Update(float timeStep)
{
	m_recharge -= timeStep;
	m_temperature -= m_coolingRate * m_coolingMultiplier * timeStep;

	if (m_temperature < 0.0f) m_temperature = 0.f;
	if (m_recharge < 0.0f) m_recharge = 0.f;

	if (!m_state) return;
	if (m_recharge > 0.0f) return;
	if (m_temperature > 1.0) return;

	if (m_ship->GetFlightState() != Ship::FLYING) return;

	Fire();
}

void Weapon::Fire()
{
	m_temperature += 0.01f;
	m_recharge = m_laserType.rechargeTime;

	const matrix3x3d &m = m_ship->GetOrient();
	const vector3d dir = m * vector3d(0.0, 0.0, -1.0);//vector3d(m_type->gunMount[num].dir);
	const vector3d pos = m * vector3d(0.0, 0.0, 30.0) + m_ship->GetPosition();//vector3d(m_type->gunMount[num].pos) + GetPosition();

	const vector3d baseVel = m_ship->GetVelocity();
	const vector3d dirVel = m_laserType.speed * dir.Normalized();

	Projectile::Add(m_ship, m_equipType, pos, baseVel, dirVel);

	//XXX passing equipType to Projectile is silly, they are lasers anyway
	//XXX use correct orientation
	//XXX use correct position
	//XXX add projectile for each muzzle
/*
	if (lt.flags & Equip::LASER_DUAL)
	{
		const ShipType::DualLaserOrientation orient = m_type->gunMount[num].orient;
		const vector3d orient_norm =
				(orient == ShipType::DUAL_LASERS_VERTICAL) ? m.VectorX() : m.VectorY();
		const vector3d sep = m_type->gunMount[num].sep * dir.Cross(orient_norm).NormalizedSafe();

		Projectile::Add(this, t, pos + sep, baseVel, dirVel);
		Projectile::Add(this, t, pos - sep, baseVel, dirVel);
	}
	else
		Projectile::Add(this, t, pos, baseVel, dirVel);
*/
	Polit::NotifyOfCrime(m_ship, Polit::CRIME_WEAPON_DISCHARGE); //fishy
	Sound::BodyMakeNoise(m_ship, "Pulse_Laser", 1.0f); //appropriate
}
