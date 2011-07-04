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



void Ship::AIModelCoordsMatchAngVel(vector3d desiredAngVel, double softness)
{
	const ShipType &stype = GetShipType();
	double angAccel = stype.angThrust / GetAngularInertia();
	const double softTimeStep = Pi::GetTimeStep() * softness;

	matrix4x4d rot;
	GetRotMatrix(rot);
	vector3d angVel = desiredAngVel - rot.InverseOf() * GetAngVelocity();

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


void Ship::AIModelCoordsMatchSpeedRelTo(const vector3d v, const Ship *other)
{
	matrix4x4d m; GetRotMatrix(m);
	vector3d relToVel = m.InverseOf() * other->GetVelocity() + v;
	AIAccelToModelRelativeVelocity(relToVel);
}


// Try to reach this model-relative velocity.
// (0,0,-100) would mean going 100m/s forward.

void Ship::AIAccelToModelRelativeVelocity(const vector3d v)
{
	// OK. For rotating frames linked to space stations we want to set
	// speed relative to non-rotating frame (so we apply Frame::GetStasisVelocityAtPosition.
	// For rotating frames linked to planets we want to set velocity relative to
	// surface, so we do not apply Frame::GetStasisVelocityAtPosition
	vector3d relVel = GetVelocity();
	if (GetFrame()->IsStationRotFrame()) {
		relVel -= GetFrame()->GetStasisVelocityAtPosition(GetPosition());
	}
	matrix4x4d m; GetRotMatrix(m);
	vector3d difVel = v - (relVel * m);		// required change in velocity

	vector3d maxThrust = GetMaxThrust(difVel);
	vector3d maxFrameAccel = maxThrust * Pi::GetTimeStep() / GetMass();

	SetThrusterState(0, difVel.x / maxFrameAccel.x);
	SetThrusterState(1, difVel.y / maxFrameAccel.y);
	SetThrusterState(2, difVel.z / maxFrameAccel.z);	// use clamping
}


// returns true if command is complete
bool Ship::AITimeStep(float timeStep)
{
	// allow the launch thruster thing to happen
	if (m_launchLockTimeout != 0) return false;

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
		Pi::luaOnAICompleted.Queue(this);
		return true;
	}
	else return false;
}

void Ship::AIClearInstructions()
{
	if (!m_curAICmd) return;

	delete m_curAICmd;		// rely on destructor to kill children
	m_curAICmd = 0;
}

void Ship::AIKamikaze(Body *target)
{
	AIClearInstructions();
	m_curAICmd = new AICmdKamikaze(this, target);
}

void Ship::AIKill(Ship *target)
{
	AIClearInstructions();
	m_curAICmd = new AICmdKill(this, target);
}

/*
void Ship::AIJourney(SBodyPath &dest)
{
	AIClearInstructions();
//	m_curAICmd = new AICmdJourney(this, dest);
}
*/

void Ship::AIFlyTo(Body *target)
{
	AIClearInstructions();
	m_curAICmd = new AICmdFlyTo(this, target);
}

void Ship::AIDock(SpaceStation *target)
{
	AIClearInstructions();
	m_curAICmd = new AICmdDock(this, target);
}

void Ship::AIOrbit(Body *target, double alt)
{
	AIClearInstructions();
	m_curAICmd = new AICmdFlyTo(this, target, alt);
}

void Ship::AIHoldPosition()
{
	AIClearInstructions();
	m_curAICmd = new AICmdHoldPosition(this);
}

// Because of issues when reducing timestep, must do parts of this as if 1x accel
// final frame has too high velocity to correct if timestep is reduced
// fix is too slow in the terminal stages:
//	if (endvel <= vel) { endvel = vel; ivel = dist / Pi::GetTimeStep(); }	// last frame discrete correction
//	ivel = std::min(ivel, endvel + 0.5*acc/PHYSICS_HZ);	// unknown next timestep discrete overshoot correction

// yeah ok, this doesn't work
// sometimes endvel is too low to catch moving objects
// worked around with half-accel hack in dynamicbody & pi.cpp

static double calc_ivel(double dist, double vel, double acc)
{
	bool inv = false;
	if (dist < 0) { dist = -dist; vel = -vel; inv = true; }
	double ivel = 0.9 * sqrt(vel*vel + 2.0 * acc * dist);		// fudge hardly necessary

	double endvel = ivel - (acc * Pi::GetTimeStep());
	if (vel == 0.0 && endvel <= 0) ivel = dist / Pi::GetTimeStep();	// last frame discrete correction
	else ivel = (ivel + endvel) * 0.5;					// discrete overshoot correction
//	else ivel = endvel + 0.5*acc/PHYSICS_HZ;			// unknown next timestep discrete overshoot correction

	return (inv) ? -ivel : ivel;
}

// vel is desired velocity in ship's frame
// returns true if this can be attained in a single timestep
// todo: check space station rotating frame case
bool Ship::AIMatchVel(const vector3d &vel)
{
	matrix4x4d rot; GetRotMatrix(rot);
	vector3d diffvel = (vel - GetVelocityRelTo(GetFrame())) * rot;		// convert to object space
	return AIChangeVelBy(diffvel);
}

// diffvel is required change in velocity in object space
// returns true if this can be done in a single timestep
bool Ship::AIChangeVelBy(const vector3d &diffvel)
{
	// counter external forces unless we're in an orbital station rotating frame
	matrix4x4d rot; GetRotMatrix(rot);
	vector3d diffvel2 = GetExternalForce() * Pi::GetTimeStep() / GetMass();
	if (GetFrame()->IsStationRotFrame()) diffvel2 = diffvel;
	else diffvel2 = diffvel - diffvel2 * rot;

	vector3d maxThrust = GetMaxThrust(diffvel2);
	vector3d maxFrameAccel = maxThrust * Pi::GetTimeStep() / GetMass();
	vector3d thrust(diffvel2.x / maxFrameAccel.x,
					diffvel2.y / maxFrameAccel.y,
					diffvel2.z / maxFrameAccel.z);
	SetThrusterState(thrust);			// use clamping
	if (thrust.x*thrust.x > 1.0 || thrust.y*thrust.y > 1.0 || thrust.z*thrust.z > 1.0) return false;
	return true;
}

// relpos and relvel are position and velocity of ship relative to target in ship's frame
// targvel is in direction of motion, must be positive
// returns difference in closing speed from ideal, or zero if it thinks it's at the target
// flip == true means it uses main thruster value for determining decel point
double Ship::AIMatchPosVel(const vector3d &relpos, const vector3d &relvel, double targspeed, const vector3d &maxthrust)
{
	matrix4x4d rot; GetRotMatrix(rot);
	vector3d objpos = relpos * rot;
	vector3d reldir = objpos.NormalizedSafe();
	vector3d endvel = targspeed * reldir;
	double invmass = 1.0 / GetMass();

	// find ideal velocities at current time given reverse thrust level
	vector3d ivel;
	ivel.x = calc_ivel(objpos.x, endvel.x, maxthrust.x * invmass);
	ivel.y = calc_ivel(objpos.y, endvel.y, maxthrust.y * invmass);
	ivel.z = calc_ivel(objpos.z, endvel.z, maxthrust.z * invmass);

	vector3d objvel = relvel * rot;
	vector3d diffvel = ivel - objvel;		// required change in velocity
	AIChangeVelBy(diffvel);
	return diffvel.Dot(reldir);
}

// Input in object space
void Ship::AIMatchAngVelObjSpace(const vector3d &angvel)
{
	double maxAccel = GetShipType().angThrust / GetAngularInertia();
	double invFrameAccel = 1.0 / (maxAccel * Pi::GetTimeStep());

	matrix4x4d rot; GetRotMatrix(rot);
	vector3d diff = angvel - GetAngVelocity() * rot;		// find diff between current & desired angvel
	SetAngThrusterState(diff * invFrameAccel);
}

// just forces the orientation
void Ship::AIFaceDirectionImmediate(const vector3d &dir)
{
	vector3d zaxis = -dir;
	vector3d xaxis = vector3d(0.0,1.0,0.0).Cross(zaxis).Normalized();
	vector3d yaxis = zaxis.Cross(xaxis).Normalized();
	matrix4x4d wantOrient = matrix4x4d::MakeRotMatrix(xaxis, yaxis, zaxis).InverseOf();
	SetRotMatrix(wantOrient);
}

// position in ship's frame
vector3d Ship::AIGetNextFramePos()
{
	vector3d thrusters = GetThrusterState();
	vector3d maxThrust = GetMaxThrust(thrusters);
	vector3d thrust = vector3d(maxThrust.x*thrusters.x, maxThrust.y*thrusters.y,
		maxThrust.z*thrusters.z);
	matrix4x4d rot; GetRotMatrix(rot);
	vector3d vel = GetVelocity() + rot * thrust * Pi::GetTimeStep() / GetMass();
	vector3d pos = GetPosition() + vel * Pi::GetTimeStep();
	return pos;
}

double Ship::AIFaceOrient(const vector3d &dir, const vector3d &updir)
{
	double timeStep = Pi::GetTimeStep();
	matrix4x4d rot; GetRotMatrix(rot);
	double maxAccel = GetShipType().angThrust / GetAngularInertia();		// should probably be in stats anyway
	double frameAccel = maxAccel * timeStep;
	
	if (dir.Dot(vector3d(rot[8], rot[9], rot[10])) > -0.999999)
		{ AIFaceDirection(dir); return false; }
	
	vector3d uphead = (updir * rot).Normalized();		// create desired object-space updir
	vector3d dav(0.0, 0.0, 0.0);			// desired angular velocity
	double ang = 0.0;
	if (uphead.y < 0.999999)
	{
		ang = acos(Clamp(uphead.y, -1.0, 1.0));		// scalar angle from head to curhead
		double iangvel = sqrt(2.0 * maxAccel * ang);	// ideal angvel at current time

		double frameEndAV = iangvel - frameAccel;
		if (frameEndAV <= 0.0) iangvel = ang / timeStep;	// last frame discrete correction
		else iangvel = (iangvel + frameEndAV) * 0.5;		// discrete overshoot correction
		dav.z = -iangvel;
	}
	vector3d cav = (GetAngVelocity() - GetFrame()->GetAngVelocity()) * rot;				// current obj-rel angvel
//	vector3d cav = GetAngVelocity() * rot;				// current obj-rel angvel
	vector3d diff = (dav - cav) / frameAccel;			// find diff between current & desired angvel

	SetAngThrusterState(diff);
	return ang;
//	if (diff.x*diff.x > 1.0 || diff.y*diff.y > 1.0 || diff.z*diff.z > 1.0) return false;
//	else return true;
}


// Input: direction in ship's frame, doesn't need to be normalized
// Approximate positive angular velocity at match point
// Applies thrust directly
// old: returns whether it can reach that direction in this frame
// returns angle to target
double Ship::AIFaceDirection(const vector3d &dir, double av)
{
	double timeStep = Pi::GetTimeStep();

	double maxAccel = GetShipType().angThrust / GetAngularInertia();		// should probably be in stats anyway
	if (!maxAccel)
		// happens if no angular thrust is set for the model eg MISSILE_UNGUIDED
		return 0.0;
	double frameAccel = maxAccel * timeStep;

	matrix4x4d rot; GetRotMatrix(rot);

	vector3d head = (dir * rot).Normalized();		// create desired object-space heading
	vector3d dav(0.0, 0.0, 0.0);	// desired angular velocity

	double ang = 0.0;
	if (head.z > -0.99999999)			
	{
		ang = acos (Clamp(-head.z, -1.0, 1.0));		// scalar angle from head to curhead
		double iangvel = av + sqrt (2.0 * maxAccel * ang);	// ideal angvel at current time

		double frameEndAV = iangvel - frameAccel;
		if (frameEndAV <= 0.0) iangvel = ang / timeStep;	// last frame discrete correction
		else iangvel = (iangvel + frameEndAV) * 0.5;		// discrete overshoot correction

		// Normalize (head.x, head.y) to give desired angvel direction
		if (head.z > 0.9999) head.x = 1.0;
		double head2dnorm = 1.0 / sqrt(head.x*head.x + head.y*head.y);		// NAN fix shouldn't be necessary if inputs are normalized
		dav.x = head.y * head2dnorm * iangvel;
		dav.y = -head.x * head2dnorm * iangvel;
	}
	vector3d cav = (GetAngVelocity() - GetFrame()->GetAngVelocity()) * rot;		// current obj-rel angvel
//	vector3d cav = GetAngVelocity() * rot;				// current obj-rel angvel
	vector3d diff = (dav - cav) / frameAccel;					// find diff between current & desired angvel

	SetAngThrusterState(diff);
	return ang;
//if (diff.x*diff.x > 1.0 || diff.y*diff.y > 1.0 || diff.z*diff.z > 1.0) return false;
//	else return true;
}


// returns direction in ship's frame from this ship to target lead position
vector3d Ship::AIGetLeadDir(const Body *target, const vector3d& targaccel, int gunindex)
{
	vector3d targpos = target->GetPositionRelTo(this);
	vector3d targvel = target->GetVelocityRelTo(this);
	// todo: should adjust targpos for gunmount offset

	int laser = Equip::types[m_equipment.Get(Equip::SLOT_LASER, gunindex)].tableIndex;
	double projspeed = Equip::lasers[laser].speed;

	// first attempt
	double projtime = targpos.Length() / projspeed;
	vector3d leadpos = targpos + targvel*projtime + 0.5*targaccel*projtime*projtime;

	// second pass
	projtime = leadpos.Length() / projspeed;
	leadpos = targpos + targvel*projtime + 0.5*targaccel*projtime*projtime;

	return leadpos.Normalized();
}

/* Don't actually need this
// same inputs as matchposvel, returns approximate travel time instead
double Ship::AITravelTime(const vector3d &relpos, const vector3d &relvel, double targspeed, bool flip)
{
	matrix4x4d rot; GetRotMatrix(rot);
	double dist = relpos.Length();
	double speed = -(relvel * rot).z;		// speed >0 is towards

	double faccel = -GetShipType().linThrust[ShipType::THRUSTER_FORWARD] / GetMass();
	double raccel = GetShipType().linThrust[ShipType::THRUSTER_REVERSE] / GetMass();
	if (flip) raccel = faccel;

	// first part is time necessary to change speed to zero
	double time1, time2, time3;
	time1 = (targspeed-speed) / faccel;
	dist += 0.5 * time1 * (targspeed-speed);

	// now time to cover the remaining distance		

	time2 = sqrt(2 * naccel)

	// find ideal velocities at current time given reverse thrust level
	double ispeed = 0.9 * sqrt(targspeed*targspeed + 2.0*paccel*dist);

	vector3d diffvel = ivel - objvel;		// required change in velocity
	AIChangeVelBy(diffvel);

	return diffvel.Dot(reldir);
}
*/
