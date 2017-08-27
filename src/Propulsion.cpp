// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Propulsion.h"
#include "Pi.h"
#include "Game.h"
#include "Object.h" // <- here only for comment in AIFaceDirection (line 320)
#include "KeyBindings.h" // <- same here

void Propulsion::SaveToJson(Json::Value &jsonObj, Space *space)
{
	//Json::Value PropulsionObj(Json::objectValue); // Create JSON object to contain propulsion data.
	VectorToJson(jsonObj, m_angThrusters, "ang_thrusters");
	VectorToJson(jsonObj, m_thrusters, "thrusters");
	jsonObj["thruster_fuel"] = DoubleToStr( m_thrusterFuel );
	jsonObj["reserve_fuel"] = DoubleToStr( m_reserveFuel );
	// !!! These are commented to avoid savegame bumps:
	//jsonObj["tank_mass"] = m_fuelTankMass;
	//jsonObj["propulsion"] = PropulsionObj;
};

void Propulsion::LoadFromJson(const Json::Value &jsonObj, Space *space)
{

	if (!jsonObj.isMember("ang_thrusters")) throw SavedGameCorruptException();
	if (!jsonObj.isMember("thrusters")) throw SavedGameCorruptException();
	if (!jsonObj.isMember("thruster_fuel")) throw SavedGameCorruptException();
	if (!jsonObj.isMember("reserve_fuel")) throw SavedGameCorruptException();
	// !!! This is commented to avoid savegame bumps:
	//if (!jsonObj.isMember("tank_mass")) throw SavedGameCorruptException();

	vector3d temp_vector;
	JsonToVector(&temp_vector, jsonObj, "ang_thrusters");
	SetAngThrusterState( temp_vector );
	JsonToVector(&temp_vector, jsonObj, "thrusters");
	SetThrusterState( temp_vector );
	m_thrusterFuel = StrToDouble(jsonObj["thruster_fuel"].asString());
	m_reserveFuel = StrToDouble(jsonObj["reserve_fuel"].asString());
	// !!! This is commented to avoid savegame bumps:
	//m_fuelTankMass = jsonObj["tank_mass"].asInt();
};

Propulsion::Propulsion()
{
	m_fuelTankMass = 1;
	for ( int i=0; i< Thruster::THRUSTER_MAX; i++) m_linThrust[i]=0.0;
	m_angThrust = 0.0;
	m_effectiveExhaustVelocity = 100000.0;
	m_thrusterFuel= 0.0;	// remaining fuel 0.0-1.0
	m_reserveFuel= 0.0;
	m_FuelStateChange = false;
	m_thrusters = vector3d(0,0,0);
	m_angThrusters = vector3d(0,0,0);
	m_smodel = nullptr;
	m_dBody = nullptr;
}

void Propulsion::Init(DynamicBody *b, SceneGraph::Model *m, const int tank_mass, const double effExVel, const float lin_Thrust[], const float ang_Thrust )
{
		m_fuelTankMass = tank_mass;
		m_effectiveExhaustVelocity = effExVel;
		for (int i=0; i<Thruster::THRUSTER_MAX; i++ ) m_linThrust[i] = lin_Thrust[i];
		m_angThrust = ang_Thrust;
		m_smodel = m;
		m_dBody = b;
}

void Propulsion::SetThrustPowerMult(double p, const float lin_Thrust[], const float ang_Thrust)
{
	// Init of Propulsion:
	for (int i = 0; i<Thruster::THRUSTER_MAX; i++)
		m_linThrust[i] = lin_Thrust[i] * p;
	m_angThrust = ang_Thrust * p;

}

void Propulsion::SetAngThrusterState(const vector3d &levels)
{
	if (m_thrusterFuel <= 0.f) {
		m_angThrusters = vector3d(0.0);
	} else {
		m_angThrusters.x = Clamp(levels.x, -1.0, 1.0);
		m_angThrusters.y = Clamp(levels.y, -1.0, 1.0);
		m_angThrusters.z = Clamp(levels.z, -1.0, 1.0);
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
	maxThrust.x = ((dir.x > 0) ? m_linThrust[THRUSTER_RIGHT] : -m_linThrust[THRUSTER_LEFT] );
	maxThrust.y = ((dir.y > 0) ? m_linThrust[THRUSTER_UP] : -m_linThrust[THRUSTER_DOWN] );
	maxThrust.z = ((dir.z > 0) ? m_linThrust[THRUSTER_REVERSE] : -m_linThrust[THRUSTER_FORWARD] );
	return maxThrust;
}

double Propulsion::GetThrustMin() const
{
	double val = m_linThrust[THRUSTER_UP];
	val = std::min(val, static_cast<double>(m_linThrust[THRUSTER_RIGHT]));
	val = std::min(val, static_cast<double>(-m_linThrust[THRUSTER_LEFT]));
	return val;
}

float Propulsion::GetFuelUseRate()
{
	const float denominator = m_fuelTankMass * m_effectiveExhaustVelocity * 10;
	return denominator > 0 ? -(m_linThrust[THRUSTER_FORWARD])/denominator : 1e9;
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

// returns speed that can be reached using fuel minus reserve according to the Tsiolkovsky equation
double Propulsion::GetSpeedReachedWithFuel() const
{
	const double mass = m_dBody->GetMass();
	const double fuelmass = 1000 * m_fuelTankMass * ( m_thrusterFuel - m_reserveFuel );
	if (fuelmass < 0) return 0.0;
	return m_effectiveExhaustVelocity * log( mass/( mass-fuelmass ));
}

void Propulsion::Render(Graphics::Renderer *r, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform)
{
	/* TODO: allow Propulsion to know SceneGraph::Thruster and
	 * to work directly with it (this could lead to movable
	 * thruster and so on)... this code is :-/
	*/
	//angthrust negated, for some reason
	if (m_smodel != nullptr ) m_smodel->SetThrust(vector3f( GetThrusterState() ), -vector3f( GetAngThrusterState() ));
}

void Propulsion::AIModelCoordsMatchAngVel(const vector3d &desiredAngVel, double softness)
{
	double angAccel = m_angThrust / m_dBody->GetAngularInertia();
	const double softTimeStep = Pi::game->GetTimeStep() * softness;

	vector3d angVel = desiredAngVel - m_dBody->GetAngVelocity() * m_dBody->GetOrient();
	vector3d thrust;
	for (int axis=0; axis<3; axis++) {
		if (angAccel * softTimeStep >= fabs(angVel[axis])) {
			thrust[axis] = angVel[axis] / (softTimeStep * angAccel);
		} else {
			thrust[axis] = (angVel[axis] > 0.0 ? 1.0 : -1.0);
		}
	}
	SetAngThrusterState(thrust);
}

void Propulsion::AIModelCoordsMatchSpeedRelTo(const vector3d &v, const DynamicBody *other)
{
	vector3d relToVel = other->GetVelocity() * m_dBody->GetOrient() + v;
	AIAccelToModelRelativeVelocity(relToVel);
}

// Try to reach this model-relative velocity.
// (0,0,-100) would mean going 100m/s forward.

void Propulsion::AIAccelToModelRelativeVelocity(const vector3d &v)
{
	vector3d difVel = v - m_dBody->GetVelocity() * m_dBody->GetOrient();	// required change in velocity
	vector3d maxThrust = GetThrustMax(difVel);
	vector3d maxFrameAccel = maxThrust * (Pi::game->GetTimeStep() / m_dBody->GetMass());

	SetThrusterState(0, is_zero_exact(maxFrameAccel.x) ? 0.0 : difVel.x / maxFrameAccel.x);
	SetThrusterState(1, is_zero_exact(maxFrameAccel.y) ? 0.0 : difVel.y / maxFrameAccel.y);
	SetThrusterState(2, is_zero_exact(maxFrameAccel.z) ? 0.0 : difVel.z / maxFrameAccel.z);	// use clamping
}

/* NOTE: following code were in Ship-AI.cpp file,
 * no changes were made, except those needed
 * to make it compatible with actual Propulsion
 * class (and yes: it's only a copy-paste,
 * including comments :) )
*/

// Because of issues when reducing timestep, must do parts of this as if 1x accel
// final frame has too high velocity to correct if timestep is reduced
// fix is too slow in the terminal stages:
//	if (endvel <= vel) { endvel = vel; ivel = dist / Pi::game->GetTimeStep(); }	// last frame discrete correction
//	ivel = std::min(ivel, endvel + 0.5*acc/PHYSICS_HZ);	// unknown next timestep discrete overshoot correction

// yeah ok, this doesn't work
// sometimes endvel is too low to catch moving objects
// worked around with half-accel hack in dynamicbody & pi.cpp

double calc_ivel(double dist, double vel, double acc)
{
	bool inv = false;
	if (dist < 0) { dist = -dist; vel = -vel; inv = true; }
	double ivel = 0.9 * sqrt(vel*vel + 2.0 * acc * dist);		// fudge hardly necessary

	double endvel = ivel - (acc * Pi::game->GetTimeStep());
	if (endvel <= 0.0) ivel = dist / Pi::game->GetTimeStep();	// last frame discrete correction
	else ivel = (ivel + endvel) * 0.5;					// discrete overshoot correction
//	else ivel = endvel + 0.5*acc/PHYSICS_HZ;			// unknown next timestep discrete overshoot correction

	return (inv) ? -ivel : ivel;
}

// version for all-positive values
double calc_ivel_pos(double dist, double vel, double acc)
{
	double ivel = 0.9 * sqrt(vel*vel + 2.0 * acc * dist);		// fudge hardly necessary

	double endvel = ivel - (acc * Pi::game->GetTimeStep());
	if (endvel <= 0.0) ivel = dist / Pi::game->GetTimeStep();	// last frame discrete correction
	else ivel = (ivel + endvel) * 0.5;					// discrete overshoot correction

	return ivel;
}

// vel is desired velocity in ship's frame
// returns true if this can be attained in a single timestep
bool Propulsion::AIMatchVel(const vector3d &vel)
{
	vector3d diffvel = (vel - m_dBody->GetVelocity()) * m_dBody->GetOrient();
	return AIChangeVelBy(diffvel);
}

// diffvel is required change in velocity in object space
// returns true if this can be done in a single timestep
bool Propulsion::AIChangeVelBy(const vector3d &diffvel)
{
	// counter external forces
	vector3d extf = m_dBody->GetExternalForce() * (Pi::game->GetTimeStep() / m_dBody->GetMass());
	vector3d diffvel2 = diffvel - extf * m_dBody->GetOrient();

	vector3d maxThrust = GetThrustMax(diffvel2);
	vector3d maxFrameAccel = maxThrust * (Pi::game->GetTimeStep() / m_dBody->GetMass());
	vector3d thrust(diffvel2.x / maxFrameAccel.x,
					diffvel2.y / maxFrameAccel.y,
					diffvel2.z / maxFrameAccel.z);
	SetThrusterState(thrust);			// use clamping
	if (thrust.x*thrust.x > 1.0 || thrust.y*thrust.y > 1.0 || thrust.z*thrust.z > 1.0) return false;
	return true;
}

// Change object-space velocity in direction of param
vector3d Propulsion::AIChangeVelDir(const vector3d &reqdiffvel)
{
	// get max thrust in desired direction after external force compensation
	vector3d maxthrust = GetThrustMax(reqdiffvel);
	maxthrust += m_dBody->GetExternalForce() * m_dBody->GetOrient();
	vector3d maxFA = maxthrust * (Pi::game->GetTimeStep() / m_dBody->GetMass());
	maxFA.x = fabs(maxFA.x); maxFA.y = fabs(maxFA.y); maxFA.z = fabs(maxFA.z);

	// crunch diffvel by relative thruster power to get acceleration in right direction
	vector3d diffvel = reqdiffvel;
	if (fabs(diffvel.x) > maxFA.x) diffvel *= maxFA.x / fabs(diffvel.x);
	if (fabs(diffvel.y) > maxFA.y) diffvel *= maxFA.y / fabs(diffvel.y);
	if (fabs(diffvel.z) > maxFA.z) diffvel *= maxFA.z / fabs(diffvel.z);

	AIChangeVelBy(diffvel);		// should always return true because it's already capped?
	return m_dBody->GetOrient() * (reqdiffvel - diffvel);		// should be remaining diffvel to correct
}

// Input in object space
void Propulsion::AIMatchAngVelObjSpace(const vector3d &angvel)
{
	double maxAccel = m_angThrust / m_dBody->GetAngularInertia();
	double invFrameAccel = 1.0 / (maxAccel * Pi::game->GetTimeStep());

	vector3d diff = angvel - m_dBody->GetAngVelocity() * m_dBody->GetOrient();		// find diff between current & desired angvel
	SetAngThrusterState(diff * invFrameAccel);
}

// get updir as close as possible just using roll thrusters
double Propulsion::AIFaceUpdir(const vector3d &updir, double av)
{
	double maxAccel = m_angThrust / m_dBody->GetAngularInertia();		// should probably be in stats anyway
	double frameAccel = maxAccel * Pi::game->GetTimeStep();

	vector3d uphead = updir * m_dBody->GetOrient();			// create desired object-space updir
	if (uphead.z > 0.99999) return 0;				// bail out if facing updir
	uphead.z = 0; uphead = uphead.Normalized();		// only care about roll axis

	double ang = 0.0, dav = 0.0;
	if (uphead.y < 0.99999999)
	{
		ang = acos(Clamp(uphead.y, -1.0, 1.0));		// scalar angle from head to curhead
		double iangvel = av + calc_ivel_pos(ang, 0.0, maxAccel);	// ideal angvel at current time

		dav = uphead.x > 0 ? -iangvel : iangvel;
	}
	double cav = (m_dBody->GetAngVelocity() * m_dBody->GetOrient()).z;	// current obj-rel angvel
	double diff = (dav - cav) / frameAccel;				// find diff between current & desired angvel

	SetAngThrusterState(2, diff);
	return ang;
}

// Input: direction in ship's frame, doesn't need to be normalized
// Approximate positive angular velocity at match point
// Applies thrust directly
// old: returns whether it can reach that direction in this frame
// returns angle to target
double Propulsion::AIFaceDirection(const vector3d &dir, double av)
{
	double maxAccel = m_angThrust / m_dBody->GetAngularInertia();		// should probably be in stats anyway

	vector3d head = (dir * m_dBody->GetOrient()).Normalized();		// create desired object-space heading
	vector3d dav(0.0, 0.0, 0.0);	// desired angular velocity

	double ang = 0.0;
	if (head.z > -0.99999999)
	{
		ang = acos (Clamp(-head.z, -1.0, 1.0));		// scalar angle from head to curhead
		double iangvel = av + calc_ivel_pos(ang, 0.0, maxAccel);	// ideal angvel at current time

		// Normalize (head.x, head.y) to give desired angvel direction
		if (head.z > 0.999999) head.x = 1.0;
		double head2dnorm = 1.0 / sqrt(head.x*head.x + head.y*head.y);		// NAN fix shouldn't be necessary if inputs are normalized
		dav.x = head.y * head2dnorm * iangvel;
		dav.y = -head.x * head2dnorm * iangvel;
	}
	const vector3d cav = m_dBody->GetAngVelocity() * m_dBody->GetOrient();				// current obj-rel angvel
	const double frameAccel = maxAccel * Pi::game->GetTimeStep();
	vector3d diff = is_zero_exact(frameAccel) ? vector3d(0.0) : (dav - cav) / frameAccel;	// find diff between current & desired angvel

	// If the player is pressing a roll key, don't override roll.
	// XXX this really shouldn't be here. a better way would be to have a
	// field in Ship describing the wanted angvel adjustment from input. the
	// baseclass version in Ship would always be 0. the version in Player
	// would be constructed from user input. that adjustment could then be
	// considered by this method when computing the required change
	if (m_dBody->IsType(Object::PLAYER) && (KeyBindings::rollLeft.IsActive() || KeyBindings::rollRight.IsActive()))
		diff.z = GetAngThrusterState().z;
	SetAngThrusterState(diff);
	return ang;
}

// returns direction in ship's frame from this ship to target lead position
vector3d Propulsion::AIGetLeadDir(const Body *target, const vector3d& targaccel, double projspeed)
{
	assert(target);
	const vector3d targpos = target->GetPositionRelTo(m_dBody);
	const vector3d targvel = target->GetVelocityRelTo(m_dBody);
	// todo: should adjust targpos for gunmount offset
	vector3d leadpos;
	// avoid a divide-by-zero floating point exception (very nearly zero is ok)
	if( !is_zero_exact(projspeed) ) {
		// first attempt
		double projtime = targpos.Length() / projspeed;
		leadpos = targpos + targvel*projtime + 0.5*targaccel*projtime*projtime;

		// second pass
		projtime = leadpos.Length() / projspeed;
		leadpos = targpos + targvel*projtime + 0.5*targaccel*projtime*projtime;
	} else {
		// default
		leadpos = targpos;
	}
	return leadpos.Normalized();
}

