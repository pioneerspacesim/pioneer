// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Weapon.h"
#include "Ship.h"
#include "Polit.h"
#include "Sound.h"
#include "Projectile.h"

Weapon::Weapon(Equip::Type type, Ship *s, const ShipType::GunMount &hardpoint)
: m_state(0)
, m_recharge(0.f)
, m_temperature(0.f)
, m_coolingRate(0.01f)
, m_coolingMultiplier(1.f)
, m_ship(s)
, m_equipType(type)
, m_position(hardpoint.pos)
, m_direction(hardpoint.dir)
{
	m_laserType = Equip::lasers[Equip::types[m_equipType].tableIndex];

	if (m_laserType.flags & Equip::LASER_DUAL) {
		const vector3d orient = hardpoint.orient == ShipType::DUAL_LASERS_HORIZONTAL ?
			vector3d(1.0, 0.0, 0.0) : vector3d(0.0, 1.0, 0.0);
		m_muzzles.push_back(hardpoint.sep * orient);
		m_muzzles.push_back(hardpoint.sep * -orient);
	} else {
		m_muzzles.push_back(vector3d(0.0));
	}
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

bool Weapon::CanFire() const
{
	if (m_recharge > 0.0f) return false;
	if (m_temperature > 1.0) return false;
	if (m_ship->GetFlightState() != Ship::FLYING) return false;

	return true;
}

void Weapon::Update(float timeStep)
{
	m_recharge -= timeStep;
	m_temperature -= m_coolingRate * m_coolingMultiplier * timeStep;

	if (m_temperature < 0.0f) m_temperature = 0.f;
	if (m_recharge < 0.0f) m_recharge = 0.f;

	if (m_state == 1 && CanFire()) Fire();
}

void Weapon::Fire()
{
	m_temperature += 0.01f;
	m_recharge = m_laserType.rechargeTime;

	const matrix3x3d &m = m_ship->GetOrient();
	const vector3d dir = m * m_direction;

	const vector3d baseVel = m_ship->GetVelocity();
	const vector3d dirVel = m_laserType.speed * dir.Normalized();

	for (unsigned int i = 0; i < m_muzzles.size(); i++) {
		const vector3d pos = m * (m_position + m_muzzles[i]) + m_ship->GetPosition();
		Projectile::Add(m_ship, m_equipType, pos, baseVel, dirVel);
	}

	Sound::BodyMakeNoise(m_ship, "Pulse_Laser", 1.0f);
}
