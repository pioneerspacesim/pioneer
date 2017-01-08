#include "Propulsion.h"
#include "libs.h"

Propulsion::Propulsion()
{
	m_power_mul = 1.0;
	m_fuelTankMass = 1;
	for ( int i=0; i< Thruster::THRUSTER_MAX; i++) m_linThrust[i]=0.0;
	m_effectiveExhaustVelocity = 1000.0;
	m_thrusterFuel= 0.0;	// remaining fuel 0.0-1.0
	m_reserveFuel= 0.0;	// 0-1, fuel not to touch for the current AI program
	m_FuelStateChange = false;
	m_thrusters = vector3d(0,0,0);
	m_angThrusters = vector3d(0,0,0);
}

void Propulsion::Init( int tank_mass, double effExVel, float ang_Thrust )
{
		m_fuelTankMass = tank_mass;
		m_effectiveExhaustVelocity = effExVel;
		m_ang_thrust = ang_Thrust;
}

void Propulsion::SetAngThrusterState(const vector3d &levels)
{
	if (m_thrusterFuel <= 0.f) {
		m_angThrusters = vector3d(0.0);
	} else {
		m_angThrusters.x = Clamp(levels.x, -1.0, 1.0) * m_power_mul;
		m_angThrusters.y = Clamp(levels.y, -1.0, 1.0) * m_power_mul;
		m_angThrusters.z = Clamp(levels.z, -1.0, 1.0) * m_power_mul;
	}
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

float Propulsion::GetFuelUseRate()
{
	const float denominator = m_fuelTankMass * m_effectiveExhaustVelocity * 10;
	return denominator > 0 ? -m_linThrust[THRUSTER_FORWARD]/denominator : 1e9;
}

void Propulsion::UpdateFuel(const float timeStep)
{
	const double fuelUseRate = GetFuelUseRate() * 0.01;
	double totalThrust = (fabs( m_thrusters.x) + fabs(m_thrusters.y) + fabs(m_thrusters.z));
	FuelState lastState = GetFuelState();
	m_thrusterFuel -= timeStep * (totalThrust * fuelUseRate);
	FuelState currentState = GetFuelState();

	if (currentState != lastState) m_FuelStateChange = true;
	else m_FuelStateChange = false;
}
