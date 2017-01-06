#include "Propulsion.h"

Propulsion::Propulsion()
{
    //ctor
}

Propulsion::~Propulsion()
{
    //dtor
}

void Propulsion::ClearLinThrusterState()
{
	m_thrusters = vector3d(0,0,0);
}

void Propulsion::ClearAngThrusterState()
{
	m_angThrusters = vector3d(0,0,0);
}

void Ship::SetAngThrusterState(const vector3d &levels)
{
	m_angThrusters.x = Clamp(levels.x, -1.0, 1.0) * power_mul;
	m_angThrusters.y = Clamp(levels.y, -1.0, 1.0) * power_mul;
	m_angThrusters.z = Clamp(levels.z, -1.0, 1.0) * power_mul;
}

void Ship::SetThrusterState(const vector3d &levels)
{
	if (m_thrusterFuel <= 0.f) {
		m_thrusters = vector3d(0.0);
	} else {
		m_thrusters.x = Clamp(levels.x, -1.0, 1.0);
		m_thrusters.y = Clamp(levels.y, -1.0, 1.0);
		m_thrusters.z = Clamp(levels.z, -1.0, 1.0);
	}
}
