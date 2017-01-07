#include "Propulsion.h"

Propulsion::Propulsion()
{
	m_power_mul = 1.0;
	m_fuelTankMass = 1;
	for ( int i=0; i< Thruster::THRUSTER_MAX; i++) m_linThrust[i]=1.0;
	m_effectiveExhaustVelocity = 1000.0;
	m_thrusterFuel= 0.0;	// remaining fuel 0.0-1.0
	m_reserveFuel= 0.0;	// 0-1, fuel not to touch for the current AI program
	m_FuelStateChange = false;
	m_thrusters = vector3d(0,0,0);
	m_angThrusters = vector3d(0,0,0);
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

void Propulsion::SetAngThrusterState(const vector3d &levels)
{
	m_angThrusters.x = Clamp(levels.x, -1.0, 1.0) * m_power_mul;
	m_angThrusters.y = Clamp(levels.y, -1.0, 1.0) * m_power_mul;
	m_angThrusters.z = Clamp(levels.z, -1.0, 1.0) * m_power_mul;
}

vector3d Propulsion::GetThrustMax(const vector3d &dir) const
{
	vector3d maxThrust;
	maxThrust.x = ((dir.x > 0) ? m_linThrust[THRUSTER_RIGHT]	: -m_linThrust[THRUSTER_LEFT]	);
	maxThrust.y = ((dir.y > 0) ? m_linThrust[THRUSTER_UP]		: -m_linThrust[THRUSTER_DOWN]	);
	maxThrust.z = ((dir.z > 0) ? m_linThrust[THRUSTER_REVERSE]	: -m_linThrust[THRUSTER_FORWARD]);
	return maxThrust;
}

double Propulsion::GetThrustMin() const
{
	float val = m_linThrust[THRUSTER_UP];
	val = std::min(val, m_linThrust[THRUSTER_RIGHT]);
	val = std::min(val, -m_linThrust[THRUSTER_LEFT]);
	return val;
}

void Propulsion::SetThrusterState(const vector3d &levels)
{
	if (m_thrusterFuel <= 0.f) {
		m_thrusters = vector3d(0.0);
	} else {
		m_thrusters.x = Clamp(levels.x, -1.0, 1.0);
		m_thrusters.y = Clamp(levels.y, -1.0, 1.0);
		m_thrusters.z = Clamp(levels.z, -1.0, 1.0);
	}
}

float Propulsion::GetFuelUseRate()
{
	const float denominator = m_fuelTankMass * m_effectiveExhaustVelocity * 10;
	return denominator > 0 ? -m_linThrust[THRUSTER_FORWARD]/denominator : 1e9;
}

void Propulsion::UpdateFuel(const float timeStep, const vector3d &thrust)
{
	const double fuelUseRate = GetFuelUseRate() * 0.01;
	double totalThrust = (fabs(thrust.x) + fabs(thrust.y) + fabs(thrust.z))
		/ -m_linThrust[THRUSTER_FORWARD];

	FuelState lastState = GetFuelState();
	SetFuel(GetFuel() - timeStep * (totalThrust * fuelUseRate));
	FuelState currentState = GetFuelState();

	if (currentState != lastState) m_FuelStateChange = true;
	else m_FuelStateChange = false;
}
