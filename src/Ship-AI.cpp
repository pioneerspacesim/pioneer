// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "Ship.h"
#include "ShipAICmd.h"
#include "Pi.h"
#include "Player.h"
#include "perlin.h"
#include "Frame.h"
#include "Planet.h"
#include "SpaceStation.h"
#include "Space.h"
#include "LuaConstants.h"
#include "LuaEvent.h"
#include "KeyBindings.h"
#include "EnumStrings.h"


void Ship::AIModelCoordsMatchAngVel(const vector3d &desiredAngVel, double softness)
{
	double angAccel = m_type->angThrust / GetAngularInertia();
	const double softTimeStep = Pi::game->GetTimeStep() * softness;

	vector3d angVel = desiredAngVel - GetAngVelocity() * GetOrient();
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


void Ship::AIModelCoordsMatchSpeedRelTo(const vector3d &v, const Ship *other)
{
	vector3d relToVel = other->GetVelocity() * GetOrient() + v;
	AIAccelToModelRelativeVelocity(relToVel);
}


// Try to reach this model-relative velocity.
// (0,0,-100) would mean going 100m/s forward.

void Ship::AIAccelToModelRelativeVelocity(const vector3d &v)
{
	vector3d difVel = v - GetVelocity() * GetOrient();		// required change in velocity
	vector3d maxThrust = GetMaxThrust(difVel);
	vector3d maxFrameAccel = maxThrust * (Pi::game->GetTimeStep() / GetMass());

	SetThrusterState(0, is_zero_exact(maxFrameAccel.x) ? 0.0 : difVel.x / maxFrameAccel.x);
	SetThrusterState(1, is_zero_exact(maxFrameAccel.y) ? 0.0 : difVel.y / maxFrameAccel.y);
	SetThrusterState(2, is_zero_exact(maxFrameAccel.z) ? 0.0 : difVel.z / maxFrameAccel.z);	// use clamping
}


// returns true if command is complete
bool Ship::AITimeStep(float timeStep)
{
	// allow the launch thruster thing to happen
	if (m_launchLockTimeout > 0.0) return false;

	m_decelerating = false;
	if (!m_curAICmd) {
		if (this == Pi::player) return true;

		// just in case the AI left it on
		ClearThrusterState();
		for (int i=0; i<ShipType::GUNMOUNT_MAX; i++)
			SetGunState(i,0);
		return true;
	}

	if (m_curAICmd->TimeStepUpdate()) {
		AIClearInstructions();
//		ClearThrusterState();		// otherwise it does one timestep at 10k and gravity is fatal
		LuaEvent::Queue("onAICompleted", this, EnumStrings::GetString("ShipAIError", AIMessage()));
		return true;
	}
	else return false;
}

void Ship::AIClearInstructions()
{
	if (!m_curAICmd) return;

	delete m_curAICmd;		// rely on destructor to kill children
	m_curAICmd = 0;
	m_decelerating = false;		// don't adjust unless AI is running
}

void Ship::AIGetStatusText(char *str)
{
	if (!m_curAICmd) strcpy(str, "AI inactive");
	else m_curAICmd->GetStatusText(str);
}

void Ship::AIKamikaze(Body *target)
{
	AIClearInstructions();
	m_curAICmd = new AICmdKamikaze(this, target);
}

void Ship::AIKill(Ship *target)
{
	AIClearInstructions();
	SetFuelReserve((GetFuel() < 0.5) ? GetFuel() / 2 : 0.25);

	m_curAICmd = new AICmdKill(this, target);
}

/*
void Ship::AIJourney(SystemBodyPath &dest)
{
	AIClearInstructions();
//	m_curAICmd = new AICmdJourney(this, dest);
}
*/

void Ship::AIFlyTo(Body *target)
{
	AIClearInstructions();
	SetFuelReserve((GetFuel() < 0.5) ? GetFuel() / 2 : 0.25);

	if (target->IsType(Object::SHIP)) {		// test code
		vector3d posoff(-1000.0, 0.0, 1000.0);
		m_curAICmd = new AICmdFormation(this, static_cast<Ship*>(target), posoff);
	}
	else m_curAICmd = new AICmdFlyTo(this, target);
}

void Ship::AIDock(SpaceStation *target)
{
	AIClearInstructions();
	SetFuelReserve((GetFuel() < 0.5) ? GetFuel() / 2 : 0.25);

	m_curAICmd = new AICmdDock(this, target);
}

void Ship::AIOrbit(Body *target, double alt)
{
	AIClearInstructions();
	SetFuelReserve((GetFuel() < 0.5) ? GetFuel() / 2 : 0.25);

	m_curAICmd = new AICmdFlyAround(this, target, alt);
}

void Ship::AIHoldPosition()
{
	AIClearInstructions();
	m_curAICmd = new AICmdHoldPosition(this);
}

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
bool Ship::AIMatchVel(const vector3d &vel)
{
	vector3d diffvel = (vel - GetVelocity()) * GetOrient();
	return AIChangeVelBy(diffvel);
}

// diffvel is required change in velocity in object space
// returns true if this can be done in a single timestep
bool Ship::AIChangeVelBy(const vector3d &diffvel)
{
	// counter external forces
	vector3d extf = GetExternalForce() * (Pi::game->GetTimeStep() / GetMass());
	vector3d diffvel2 = diffvel - extf * GetOrient();

	vector3d maxThrust = GetMaxThrust(diffvel2);
	vector3d maxFrameAccel = maxThrust * (Pi::game->GetTimeStep() / GetMass());
	vector3d thrust(diffvel2.x / maxFrameAccel.x,
					diffvel2.y / maxFrameAccel.y,
					diffvel2.z / maxFrameAccel.z);
	SetThrusterState(thrust);			// use clamping
	if (thrust.x*thrust.x > 1.0 || thrust.y*thrust.y > 1.0 || thrust.z*thrust.z > 1.0) return false;
	return true;
}

// Change object-space velocity in direction of param
vector3d Ship::AIChangeVelDir(const vector3d &reqdiffvel)
{
	// get max thrust in desired direction after external force compensation
	vector3d maxthrust = GetMaxThrust(reqdiffvel);
	maxthrust += GetExternalForce() * GetOrient();
	vector3d maxFA = maxthrust * (Pi::game->GetTimeStep() / GetMass());
	maxFA.x = fabs(maxFA.x); maxFA.y = fabs(maxFA.y); maxFA.z = fabs(maxFA.z);

	// crunch diffvel by relative thruster power to get acceleration in right direction
	vector3d diffvel = reqdiffvel;
	if (fabs(diffvel.x) > maxFA.x) diffvel *= maxFA.x / fabs(diffvel.x);
	if (fabs(diffvel.y) > maxFA.y) diffvel *= maxFA.y / fabs(diffvel.y);
	if (fabs(diffvel.z) > maxFA.z) diffvel *= maxFA.z / fabs(diffvel.z);

	AIChangeVelBy(diffvel);		// should always return true because it's already capped?
	return GetOrient() * (reqdiffvel - diffvel);		// should be remaining diffvel to correct
}

// Input in object space
void Ship::AIMatchAngVelObjSpace(const vector3d &angvel)
{
	double maxAccel = m_type->angThrust / GetAngularInertia();
	double invFrameAccel = 1.0 / (maxAccel * Pi::game->GetTimeStep());

	vector3d diff = angvel - GetAngVelocity() * GetOrient();		// find diff between current & desired angvel
	SetAngThrusterState(diff * invFrameAccel);
}

// get updir as close as possible just using roll thrusters
double Ship::AIFaceUpdir(const vector3d &updir, double av)
{
	double maxAccel = m_type->angThrust / GetAngularInertia();		// should probably be in stats anyway
	double frameAccel = maxAccel * Pi::game->GetTimeStep();

	vector3d uphead = updir * GetOrient();			// create desired object-space updir
	if (uphead.z > 0.99999) return 0;				// bail out if facing updir
	uphead.z = 0; uphead = uphead.Normalized();		// only care about roll axis

	double ang = 0.0, dav = 0.0;
	if (uphead.y < 0.99999999)
	{
		ang = acos(Clamp(uphead.y, -1.0, 1.0));		// scalar angle from head to curhead
		double iangvel = av + calc_ivel_pos(ang, 0.0, maxAccel);	// ideal angvel at current time

		dav = uphead.x > 0 ? -iangvel : iangvel;
	}
	double cav = (GetAngVelocity() * GetOrient()).z;	// current obj-rel angvel
	double diff = (dav - cav) / frameAccel;				// find diff between current & desired angvel

	SetAngThrusterState(2, diff);
	return ang;
}

// Input: direction in ship's frame, doesn't need to be normalized
// Approximate positive angular velocity at match point
// Applies thrust directly
// old: returns whether it can reach that direction in this frame
// returns angle to target
double Ship::AIFaceDirection(const vector3d &dir, double av)
{
	double maxAccel = m_type->angThrust / GetAngularInertia();		// should probably be in stats anyway

	vector3d head = (dir * GetOrient()).Normalized();		// create desired object-space heading
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
	const vector3d cav = GetAngVelocity() * GetOrient();				// current obj-rel angvel
	const double frameAccel = maxAccel * Pi::game->GetTimeStep();
	vector3d diff = is_zero_exact(frameAccel) ? vector3d(0.0) : (dav - cav) / frameAccel;	// find diff between current & desired angvel

	// If the player is pressing a roll key, don't override roll.
	// XXX this really shouldn't be here. a better way would be to have a
	// field in Ship describing the wanted angvel adjustment from input. the
	// baseclass version in Ship would always be 0. the version in Player
	// would be constructed from user input. that adjustment could then be
	// considered by this method when computing the required change
	if (IsType(Object::PLAYER) && (KeyBindings::rollLeft.IsActive() || KeyBindings::rollRight.IsActive()))
		diff.z = m_angThrusters.z;
	SetAngThrusterState(diff);
	return ang;
}


// returns direction in ship's frame from this ship to target lead position
vector3d Ship::AIGetLeadDir(const Body *target, const vector3d& targaccel, int gunindex)
{
	assert(target);
	if (ScopedTable(m_equipSet).CallMethod<int>("OccupiedSpace", "laser_front") == 0)
		return target->GetPositionRelTo(this).Normalized();

	const vector3d targpos = target->GetPositionRelTo(this);
	const vector3d targvel = target->GetVelocityRelTo(this);
	// todo: should adjust targpos for gunmount offset

	double projspeed = 0;
	Properties().Get(gunindex?"laser_rear_speed":"laser_front_speed", projspeed);

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

// underestimates if endspeed isn't reachable
/*
double Ship::AITravelTime(double targdist, double relspeed, double endspeed, double maxdecel)
{
//	double speed = relvel.Dot(reldir);		// speed >0 is towards
	double dist = targdist;
	double faccel = GetAccelFwd();
	double time1, time2, time3;

	// time to reduce speed to zero:
	time1 = -relspeed / faccel;
	dist += 0.5 * time1 * -relspeed;

	// time to reduce speed to zero after target reached:
	time3 = -endspeed / maxdecel;
	dist += 0.5 * time3 * -endspeed;

	// now time to cover distance between zero-vel points
	// midpoint = intercept of two gradients
	double m = dist*maxdecel / (faccel+maxdecel);
	time2 = sqrt(2*m/faccel) + sqrt(2*(dist-m)/maxdecel);

	return time1+time2+time3;
}
*/
