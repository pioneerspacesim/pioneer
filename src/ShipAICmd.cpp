// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
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


static const double VICINITY_MIN = 15000.0;
static const double VICINITY_MUL = 4.0;

AICommand *AICommand::LoadFromJson(const Json::Value &jsonObj)
{
	if (jsonObj.isMember("ai_command"))
	{
		Json::Value aiCommandObj = jsonObj["ai_command"];
		if (!aiCommandObj.isMember("common_ai_command")) throw SavedGameCorruptException();
		Json::Value commonAiCommandObj = aiCommandObj["common_ai_command"];
		if (!commonAiCommandObj.isMember("command_name")) throw SavedGameCorruptException();
		CmdName name = CmdName(commonAiCommandObj["command_name"].asInt());
		switch (name) {
			case CMD_NONE: default: return 0; // No longer need CMD_NONE (see AICommand::SaveToJson notes).
			case CMD_DOCK: return new AICmdDock(aiCommandObj);
			case CMD_FLYTO: return new AICmdFlyTo(aiCommandObj);
			case CMD_FLYAROUND: return new AICmdFlyAround(aiCommandObj);
			case CMD_KILL: return new AICmdKill(aiCommandObj);
			case CMD_KAMIKAZE: return new AICmdKamikaze(aiCommandObj);
			case CMD_HOLDPOSITION: return new AICmdHoldPosition(aiCommandObj);
			case CMD_FORMATION: return new AICmdFormation(aiCommandObj);
		}
	}
	return 0; // Return 0 if supplied object doesn't contain an "ai_command" object.
}

void AICommand::SaveToJson(Json::Value &jsonObj)
{
	// AICommand is an abstract base class, so it is guaranteed that the supplied object
	// is the top-level JSON object (always named "ai_command") encoding the specific ai command.
	// The top-level ai command object will contain:
	// (1) the specific ai command data (already created and added in the overriding concrete class function), and
	// (2) the common ai command object (created and added in this base class function).
	// The command name (enum CmdName) and a child ai command (if this ai command has one) are added to the common ai command object.
	// No longer need to save CMD_NONE when a child ai command does not exist (just don't add a child ai command object).
	Space *space = Pi::game->GetSpace();
	Json::Value commonAiCommandObj(Json::objectValue); // Create JSON object to contain common ai command data.
	commonAiCommandObj["command_name"] = Json::Value::Int(m_cmdName);
	commonAiCommandObj["index_for_body"] = space->GetIndexForBody(m_dBody);
	if (m_child) m_child->SaveToJson(commonAiCommandObj);
	jsonObj["common_ai_command"] = commonAiCommandObj; // Add common ai command object to supplied object.
}

AICommand::AICommand(const Json::Value &jsonObj, CmdName name) : m_cmdName(name)
{
	if (!jsonObj.isMember("common_ai_command")) throw SavedGameCorruptException();
	Json::Value commonAiCommandObj = jsonObj["common_ai_command"];

	if (!commonAiCommandObj.isMember("index_for_body")) throw SavedGameCorruptException();
	m_dBodyIndex = commonAiCommandObj["index_for_body"].asInt();

	m_child.reset(LoadFromJson(commonAiCommandObj));
}

void AICommand::PostLoadFixup(Space *space)
{
	// subsystem should be initializated on each inherited AICommand
	m_dBody = static_cast<DynamicBody *>(space->GetBodyByIndex(m_dBodyIndex));
	if (m_child) m_child->PostLoadFixup(space);
}

bool AICommand::ProcessChild()
{
	if (!m_child) return true;						// no child present
	if (!m_child->TimeStepUpdate()) return false;	// child still active
	m_child.reset();
	return true;								// child finished
}

/*
// temporary evasion-test version
bool AICmdKill::TimeStepUpdate()
{
	m_timeSinceChange += Pi::game->GetTimeStep();
	if (m_timeSinceChange < m_changeTime) {
		m_ship->AIFaceDirection(m_curDir);
		return false;
	}

	m_ship->ClearThrusterState();

	// ok, so now pick new direction
	vector3d targdir = m_target->GetPositionRelTo(m_ship).Normalized();
	vector3d tdir1 = targdir.Cross(vector3d(targdir.z+0.1, targdir.x, targdir.y));
	tdir1 = tdir1.Normalized();
	vector3d tdir2 = targdir.Cross(tdir1);

	double d1 = Pi::rng.Double() - 0.5;
	double d2 = Pi::rng.Double() - 0.5;

	m_curDir = (targdir + d1*tdir1 + d2*tdir2).Normalized();
	m_ship->AIFaceDirection(m_curDir);

	m_ship->SetThrusterState(ShipType::THRUSTER_FORWARD, 0.66);		// give player a chance
	switch(Pi::rng.Int32() & 0x3)
	{
		case 0x0: m_ship->SetThrusterState(ShipType::THRUSTER_LEFT, 0.7); break;
		case 0x1: m_ship->SetThrusterState(ShipType::THRUSTER_RIGHT, 0.7); break;
		case 0x2: m_ship->SetThrusterState(ShipType::THRUSTER_UP, 0.7); break;
		case 0x3: m_ship->SetThrusterState(ShipType::THRUSTER_DOWN, 0.7); break;
	}

	m_timeSinceChange = 0.0f;
	m_changeTime = (float)Pi::rng.Double() * 10.0f;
	return false;
}
*/

// goals of this command:
//	1. inflict damage on current target
//	2. avoid taking damage to self
// two sub-patterns:
//	1. point at leaddir, shift sideways, adjust range with front/rear
//	2. flypast - change angle to target as rapidly as possible
/*
bool AICmdKill::TimeStepUpdate()
{
	if (GetDockedWith()) Undock();
	if (m_ship->GetFlightState() != Ship::FLYING) return false;		// wait until active

	SetGunState(0,0);

	// do pattern timeout here

	bool rval = true;
	switch (m_state) {
		case 0: break;
		case 1: rval = PatternKill(); break;
		case 2: rval = PatternShift(); break;
		case 3: rval = PatternEvade(); break;		// full evades should be in higher level function
	}

// have the following things to pass from higher-level function:
// 1. whether to evade or counter-evade
// 2. desired separation (based on relative ship sizes + random)

	// long term factors:
	// if our angular accel is higher, flypast and close-range combat become more effective
	m_accRatio = (m_target->GetShipType().angThrust * m_ship->GetAngularInertia())
		/ (m_ship->GetShipType().angThrust * m_target->GetAngularInertia());

	// if their ship is relatively large, want to use longer distances or more evasion
	m_sizeRatio = m_target->GetBoundingRadius() / m_ship->GetBoundingRadius();

	// if their ship has higher-speed weaponry, want to use closer distances or less evasion

//	m_wpnSpeedRatio = Equip::types[m_ship->m_equipment.Get(Equip::SLOT_LASERS, 0)]


	// Immediate factors:
	// if their laser temperature is high, counter-evade and close range

	// if our laser temperature is high, full evade and increase range

	// if outmatched, run away?

	// if under attack from other ships, may evade randomly

	// if opponent is not visible, may enter random control mode

	// if not visible to opponent and close, may attempt to stay in blind spot?


	if (rval) {			// current pattern complete, pick which to use next


		// danger metrics: damage taken, target heading & range,
		// separate danger from target and danger from elsewhere?

		// *could* check
	}
}
*/

// assumes approximate target facing and cleared thruster state
// curdist => current distance from target
// curspeed => current speed towards target, positive towards
// reqdist => desired distance from target
// speedmargin => don't use thrust if within this value of ideal speed
/*
double AICmdKill::MaintainDistance(double curdist, double curspeed, double reqdist, double speedmargin)
{
	// use maximum *deceleration*
	const ShipType &stype = m_ship->GetShipType();
	double rearaccel = stype.linThrust[ShipType::THRUSTER_REVERSE] / m_ship->GetMass();
	// v = sqrt(2as)
	double ispeed = sqrt(2.0 * rearaccel * (curdist - reqdist));
	double vdiff = ispeed - curspeed;

	if (vdiff*vdiff < speedmargin*speedmargin) return 0;
	if (vdiff > 0.0) return -1.0;
	else return 1.0;
}
*/

static void LaunchShip(Ship *ship)
{
	if (ship->GetFlightState() == Ship::LANDED)
		ship->Blastoff();
	else if (ship->GetFlightState() == Ship::DOCKED)
		ship->Undock();
}

bool AICmdKamikaze::TimeStepUpdate()
{
	if (!m_target || m_target->IsDead()) return true;

	if (m_dBody->IsType(Object::SHIP)) {
		// "Standard" checks for a ship...
		Ship *ship = static_cast<Ship*>(m_dBody);
		assert(ship!=nullptr);
		if (ship->GetFlightState() == Ship::JUMPING) return false;
		if (ship->GetFlightState() == Ship::FLYING) ship->SetWheelState(false);
		else { LaunchShip(ship); return false; }

		ship->SetGunState(0,0);

	} else {
		// Missile, for now ;-)
	}

	const vector3d targetPos = m_target->GetPositionRelTo(m_dBody);
	const vector3d targetDir = targetPos.NormalizedSafe();
	const double dist = targetPos.Length();

	// Don't come in too fast when we're close, so we don't overshoot by
	// too much if we miss the target.

	// Aim to collide at a speed which would take us 4s to reverse.
	const double aimCollisionSpeed = m_prop->GetAccelFwd()*2;

	// Aim to use 1/4 of our acceleration for braking while closing
	// distance, leaving the rest for course adjustment.
	const double brake = m_prop->GetAccelFwd()/4;

	const double aimRelSpeed =
		sqrt(aimCollisionSpeed*aimCollisionSpeed + 2*dist*brake);

	const vector3d aimVel = aimRelSpeed*targetDir + m_target->GetVelocityRelTo(m_dBody->GetFrame());
	const vector3d accelDir = (aimVel - m_dBody->GetVelocity()).NormalizedSafe();

	m_prop->ClearLinThrusterState();
	m_prop->ClearAngThrusterState();
	m_prop->AIFaceDirection(accelDir);

	m_prop->AIAccelToModelRelativeVelocity(aimVel * m_dBody->GetOrient());

	return false;
}

bool AICmdKill::TimeStepUpdate()
{
	if (m_dBody->IsType(Object::SHIP)) {
		Ship *ship = static_cast<Ship*>(m_dBody);
		assert(ship!=nullptr);
		if (ship->GetFlightState() == Ship::JUMPING) return false;
		if (ship->GetFlightState() == Ship::FLYING) ship->SetWheelState(false);
		else { LaunchShip(ship); return false; }
	} else {
		// Maybe be a drone ;-)
		return false;
	}

	if (!ProcessChild()) return false;
	if (!m_target || m_target->IsDead()) return true;

	const matrix3x3d &rot = m_dBody->GetOrient();
	vector3d targpos = m_target->GetPositionRelTo(m_dBody);
	vector3d targvel = m_target->GetVelocityRelTo(m_dBody);
	vector3d targdir = targpos.NormalizedSafe();
	vector3d heading = -rot.VectorZ();
	// Accel will be wrong for a frame on timestep changes, but it doesn't matter
	vector3d targaccel = (m_target->GetVelocity() - m_lastVel) / Pi::game->GetTimeStep();
	m_lastVel = m_target->GetVelocity();		// may need next frame
	vector3d leaddir = m_prop->AIGetLeadDir(m_target, targaccel, m_fguns->GetProjSpeed(0));

	if (targpos.Length() >= VICINITY_MIN+1000.0) {	// if really far from target, intercept
//		Output("%s started AUTOPILOT\n", m_ship->GetLabel().c_str());
		m_child.reset(new AICmdFlyTo(m_dBody, m_target));
		ProcessChild(); return false;
	}

	// turn towards target lead direction, add inaccuracy
	// trigger recheck when angular velocity reaches zero or after certain time

	if (m_leadTime < Pi::game->GetTime())
	{
		double skillShoot = 0.5;		// todo: should come from AI stats

		double headdiff = (leaddir - heading).Length();
		double leaddiff = (leaddir - targdir).Length();
		m_leadTime = Pi::game->GetTime() + headdiff + (1.0*Pi::rng.Double()*skillShoot);

		// lead inaccuracy based on diff between heading and leaddir
		vector3d r(Pi::rng.Double()-0.5, Pi::rng.Double()-0.5, Pi::rng.Double()-0.5);
		vector3d newoffset = r * (0.02 + 2.0*leaddiff + 2.0*headdiff)*Pi::rng.Double()*skillShoot;
		m_leadOffset = (heading - leaddir);		// should be already...
		m_leadDrift = (newoffset - m_leadOffset) / (m_leadTime - Pi::game->GetTime());

		// Shoot only when close to target

		double vissize = 1.3 * m_dBody->GetPhysRadius() / targpos.Length();
		vissize += (0.05 + 0.5*leaddiff)*Pi::rng.Double()*skillShoot;
		if (vissize > headdiff) m_fguns->SetGunFiringState(0,1);
		else m_fguns->SetGunFiringState(0,0);
		float max_fire_dist = m_fguns->GetGunRange(0);
		if (max_fire_dist > 4000) max_fire_dist = 4000;
		max_fire_dist *= max_fire_dist;
		if (targpos.LengthSqr() > max_fire_dist) m_fguns->SetGunFiringState(0,0);		// temp
	}
	m_leadOffset += m_leadDrift * Pi::game->GetTimeStep();
	double leadAV = (leaddir-targdir).Dot((leaddir-heading).NormalizedSafe());	// leaddir angvel
	m_prop->AIFaceDirection((leaddir + m_leadOffset).Normalized(), leadAV);


	vector3d evadethrust(0,0,0);
	if (m_evadeTime < Pi::game->GetTime())		// evasion time!
	{
		double skillEvade = 0.5;			// todo: should come from AI stats
		m_evadeTime = Pi::game->GetTime() + Pi::rng.Double(3.0,10.0) * skillEvade;
		if (heading.Dot(targdir) < 0.7) skillEvade += 0.5;		// not in view
		skillEvade += Pi::rng.Double(-0.5,0.5);

		vector3d targhead = -m_target->GetOrient().VectorZ() * rot;		// obj space
		vector3d targav = m_target->GetAngVelocity();

		if (skillEvade < 1.6 && targhead.z < 0.0) {		// smart chase
			vector3d objvel = targvel * rot;			// obj space targvel
			if ((objvel.x*objvel.x + objvel.y*objvel.y) < 10000) {
				evadethrust.x = objvel.x > 0.0 ? 1.0 : -1.0;
				evadethrust.y = objvel.y > 0.0 ? 1.0 : -1.0;
			}
		}
		else
		{
			skillEvade += targpos.Length() / 2000;				// 0.25 per 500m

			if (skillEvade < 1.0 && targav.Length() < 0.05) {	// smart evade, assumes facing
				evadethrust.x = targhead.x < 0.0 ? 1.0 : -1.0;
				evadethrust.y = targhead.y < 0.0 ? 1.0 : -1.0;
			}
			else if (skillEvade < 1.3) {			// random two-thruster evade
				evadethrust.x = (Pi::rng.Int32()&8) ? 1.0 : -1.0;
				evadethrust.y = (Pi::rng.Int32()&4) ? 1.0 : -1.0;
			}
			else if (skillEvade < 1.6) {			// one thruster only
				if (Pi::rng.Int32()&8)
					evadethrust.x = (Pi::rng.Int32()&4) ? 1.0 : -1.0;
				else evadethrust.y = (Pi::rng.Int32()&4) ? 1.0 : -1.0;
			}
			// else no evade thrust
		}
	}
	else evadethrust = m_prop->GetThrusterState();


	// todo: some logic behind desired range? pass from higher level
	if (m_closeTime < Pi::game->GetTime())
	{
		double skillEvade = 0.5;
		if (heading.Dot(targdir) < 0.7) skillEvade += 0.5;		// not in view

		m_closeTime = Pi::game->GetTime() + skillEvade * Pi::rng.Double(1.0,5.0);

		double reqdist = 500.0 + skillEvade * Pi::rng.Double(-500.0, 250);
		double dist = targpos.Length(), ispeed;
		double rearaccel = m_prop->GetAccelRev();
		rearaccel += targaccel.Dot(targdir);
		// v = sqrt(2as), positive => towards
		double as2 = 2.0 * rearaccel * (dist - reqdist);
		if (as2 > 0) ispeed = sqrt(as2); else ispeed = -sqrt(-as2);
		double vdiff = ispeed + targvel.Dot(targdir);

		if (skillEvade + Pi::rng.Double() > 1.5) evadethrust.z = 0.0;
		else if (vdiff*vdiff < 400.0) evadethrust.z = 0.0;
		else evadethrust.z = (vdiff > 0.0) ? -1.0 : 1.0;
	}
	else evadethrust.z = m_prop->GetThrusterState().z;
	m_prop->SetThrusterState(evadethrust);

	return false;
}

//Four modes for evasion vector...
//	1. target is main threat - consider heading and sweep
//	2. other ship is main threat - consider laser bolt position
//	3. no real threat - just reduce velocity vector
//	4. random

// ok, can't really decide what's best.
// best: evade from heading if low velocity, otherwise evade in direction of angvel


// first need to consider whether danger is sufficiently high to prioritise evasion
// back to the threat metrics thing

// ok, threat of target
// ideally just watch nearby laser bolts
// take account of:
//	1. range (both for hit chance and output)
//	2. laser output (consider recharge) vs armour
//	3. proximity and speed of lead angle

//	double range = targpos.Length(), rthreat;
//	if(range < 200.0) rthreat = 1.0;
//	else if(range > maxlrange) rthreat = 0.0;
//	else rthreat = (maxlrange-range) / (maxlrange-200.0);
//	rthreat *= rthreat;			// not enough maybe. consider aim, power and evasion time

// hmm. could consider heading strictly, like watching laser bolts.


//	vector3d targld = m_target->AIGetLeadDir(m_ship, vector3d(0,0,0), 0);
//	(-targpos).Normalized().Dot(targld);
// compare against target's actual heading and this ship's current velocity
// pure velocity or angular

// ok, what were the actual questions here?
// 1. whether to kill, flypast or hard evade
// - hard evade is useless except as flypast, delaying tactic, or specific laser bolt dodge
// - have dafter AIs do it anyway?

// kill if winning, basically?
// against ships with slower turn rate, might want to try to exploit that
// So what actually matters?

// 1. closer range, closing velocity => worth doing a flypast





// need fuzzy range-maintenance
// every time period, hit forward or reverse thruster or neither

// actually just use real one except only occasionally and with randomised distances
//



/*
bool AICmdKill::TimeStepUpdate()
{
	// do everything in object space
	matrix4x4d rot; GetRotMatrix(rot);
	vector3d targpos = inst.target->GetPositionRelTo(this) * rot;
	vector3d targvel = (inst.lastVel - inst.target->GetVelocity()) * inst.timeStep;
	targvel = (targvel + inst.target->GetVelocityRelativeTo(this)) * rot;
	// TODO: should adjust targpos for gunmount offset

	// store current target velocity for next frame's accel calc
	inst.lastVel = inst.target->GetVelocity();
	inst.timeStep = timeStep;

	int laser = Equip::types[m_equipment.Get(Equip::SLOT_LASER, 0)].tableIndex;
	double projspeed = Equip::lasers[laser].speed;

	vector3d leadpos = targpos + targvel*(targpos.Length()/projspeed);
	leadpos = targpos + targvel*(leadpos.Length()/projspeed);	// second order approx
	vector3d leaddir = leadpos.Normalized();

	AIFaceDirection(rot * leaddir, timeStep);

	// ok, now work out evasion and range adjustment
	// just generate preferred evasion and range vectors and span accordingly?
	// never mind that, just consider each thruster axis individually?

	// get representation of approximate angular distance
	// dot product of direction and enemy heading?
	// ideally use enemy angvel arc too - try to stay out of arc and away from heading

	// so, need three vectors in object space
	// 1. enemy position - targpos
	// 2. enemy heading - take from their rot matrix, transform to obj space
	// 2.5. enemy up vector, not using yet
	// 3. enemy angvel - transform to obj space

	matrix4x4d erot;
	inst.target->GetRotMatrix(erot);
	vector3d ehead = vector3d(-erot[8], -erot[9], -erot[10]) * rot;
//	vector3d eup = vector3d(erot[4], erot[5], erot[6]) * rot;
	vector3d eav = ((Ship *)inst.target)->GetAngVelocity() * rot;

	// stupid evade: away from heading
	vector3d evade1, evade2;
	evade1 = (ehead * targpos.Dot(ehead)) - targpos;

	// smarter evade? away from angular velocity plane
	if (eav.Length() > 0.0)	{
		evade2 = eav.Normalized();
		if (targpos.Dot(eav * targpos.Dot(eav)) > 0.0) evade2 *= -1.0;
	}
	else evade2 = evade1;

	// only do this if on target
	if (leaddir.z < -0.98)
	{
		if (evade1.x > 0.0) m_ship->SetThrusterState(ShipType::THRUSTER_RIGHT, 1.0);
		else m_ship->SetThrusterState(ShipType::THRUSTER_LEFT, 1.0);
		if (evade1.y > 0.0) m_ship->SetThrusterState(ShipType::THRUSTER_UP, 1.0);
		else m_ship->SetThrusterState(ShipType::THRUSTER_DOWN, 1.0);

		// basic range-maintenance?

		double relspeed = -targvel.Dot(targpos.Normalized());	// positive => closing
		// use maximum *deceleration*
		const ShipType &stype = GetShipType();
		double rearaccel = stype.linThrust[ShipType::THRUSTER_REVERSE] / GetMass();
		double fwdaccel = stype.linThrust[ShipType::THRUSTER_FORWARD] / GetMass();
		// v = sqrt(2as)
		double idist = 500.0;		// temporary
		double ivel = sqrt(2.0 * rearaccel * (targpos.Length() - idist));
		double vdiff = ivel - relspeed;

		if (vdiff > 0.0) m_ship->SetThrusterState(ShipType::THRUSTER_FORWARD, 1.0);
		else m_ship->SetThrusterState(ShipType::THRUSTER_REVERSE, 1.0);

		SetGunState(0,1);
	}

//	Possibly don't need this because angvel never reaches zero on moving target
	// and approximate target angular velocity at leaddir
//	vector3d leaddir2 = (leadpos + targvel*0.01).Normalized();
//	vector3d leadav = leaddir.Cross(leaddir2) * 100.0;
	// does this really give a genuine angvel? Probably

	// so have target heading and target angvel at that heading
	// can now use modified version of FaceDirection?
	// not really: direction of leaddir and leadangvel may be different
	// so blend two results: thrust to reach leaddir and thrust to attain leadangvel
	// bias towards leadangvel as heading approaches leaddir

	return false;
}
*/

static double MaxFeatureRad(Body *body)
{
	if(!body) return 0.0;
	else return body->GetPhysRadius();
}

static double MaxEffectRad(Body *body, Propulsion *prop)
{
	if(!body) return 0.0;
	if(!body->IsType(Object::TERRAINBODY)) {
		if (!body->IsType(Object::SPACESTATION)) return body->GetPhysRadius() + 1000.0;
		return static_cast<SpaceStation*>(body)->GetStationType()->ParkingDistance() + 1000.0;
	}
	return std::max(body->GetPhysRadius(), sqrt(G * body->GetMass() / prop->GetAccelUp()));
}

// returns acceleration due to gravity at that point
static double GetGravityAtPos(Frame *targframe, const vector3d &posoff)
{
	Body *body = targframe->GetBody();
	if (!body || body->IsType(Object::SPACESTATION)) return 0;
	double rsqr = posoff.LengthSqr();
	return G * body->GetMass() / rsqr;
	// inverse is: sqrt(G * m1m2 / thrust)
}

// gets position of (target + offset in target's frame) in frame
static vector3d GetPosInFrame(Frame *frame, Frame *target, const vector3d &offset)
{
	return target->GetOrientRelTo(frame) * offset + target->GetPositionRelTo(frame);
}

static vector3d GetVelInFrame(Frame *frame, Frame *target, const vector3d &offset)
{
	vector3d vel = vector3d(0.0);
	if (target != frame && target->IsRotFrame()) {
//		double ang = Pi::game->GetTimeStep() * target->GetAngSpeed();
//		vector3d newpos = offset * matrix3x3d::RotateYMatrix(ang);
//		vel = (newpos - offset) / Pi::game->GetTimeStep();
		vel = -target->GetStasisVelocity(offset);		// stasis velocity not accurate enough
	}
	return target->GetOrientRelTo(frame) * vel + target->GetVelocityRelTo(frame);
}

// generates from (0,0,0) to spos, in plane of target
// formula uses similar triangles
// shiptarg in ship's frame
// output in targframe
static vector3d GenerateTangent(DynamicBody *dBody, Frame *targframe, const vector3d &shiptarg, double alt)
{
	vector3d spos = dBody->GetPositionRelTo(targframe);
	vector3d targ = GetPosInFrame(targframe, dBody->GetFrame(), shiptarg);
	double a = spos.Length(), b = alt;
	if (b*1.02 > a) { spos *= b*1.02/a; a = b*1.02; }		// fudge if ship gets under radius
	double c = sqrt(a*a - b*b);
	return (spos*b*b)/(a*a) + spos.Cross(targ).Cross(spos).Normalized()*b*c/a;
}

// check whether ship is at risk of colliding with frame body on current path
// return values:
//0 - no collision
//1 - below feature height
//2 - unsafe escape from effect radius
//3 - unsafe entry to effect radius
//4 - probable path intercept
static int CheckCollision(DynamicBody *dBody, const vector3d &pathdir, double pathdist, const vector3d &tpos, double endvel, double r)
{
	if (!dBody->Have(DynamicBody::PROPULSION)) return 0;
	Propulsion *prop = dBody->GetPropulsion();
	assert(prop!=nullptr);
	// ship is in obstructor's frame anyway, so is tpos
	if (pathdist < 100.0) return 0;
	Body *body = dBody->GetFrame()->GetBody();
	if (!body) return 0;
	vector3d spos = dBody->GetPosition();
	double tlen = tpos.Length(), slen = spos.Length();
	double fr = MaxFeatureRad(body);

	// if target inside, check if direct entry is safe (30 degree)
	if (tlen < r) {
		double af = (tlen > fr) ? 0.5 * (1 - (tlen-fr) / (r-fr)) : 0.5;
		if (pathdir.Dot(tpos) > -af*tlen)
			if (slen < fr) return 1; else return 3;
		else return 0;
	}

	// if ship inside, check for max feature height and direct escape (30 degree)
	if (slen < r) {
		if (slen < fr) return 1;
		double af = (slen > fr) ? 0.5 * (1 - (slen-fr) / (r-fr)) : 0.5;
		if (pathdir.Dot(spos) < af*slen) return 2; else return 0;
	}

	// now for the intercept calc
	// find closest point to obstructor
	double tanlen = -spos.Dot(pathdir);
	if (tanlen < 0 || tanlen > pathdist) return 0;		// closest point outside path

	vector3d perpdir = (tanlen*pathdir + spos).Normalized();
	double perpspeed = dBody->GetVelocity().Dot(perpdir);
	double parspeed = dBody->GetVelocity().Dot(pathdir);
	if (parspeed < 0) parspeed = 0;			// shouldn't break any important case
	if (perpspeed > 0) perpspeed = 0;		// prevent attempts to speculatively fly through planets

	// find time that dBody will pass through that point
	// get velocity as if accelerating from start or end, pick smallest
	double ivelsqr = endvel*endvel + 2*prop->GetAccelFwd()*(pathdist-tanlen);		// could put endvel in here
	double fvelsqr = parspeed*parspeed + 2*prop->GetAccelFwd()*tanlen;
	double tanspeed = sqrt(ivelsqr < fvelsqr ? ivelsqr : fvelsqr);
	double time = tanlen / (0.5 * (parspeed + tanspeed));		// actually correct?

	double dist = spos.Dot(perpdir) + perpspeed*time;		// spos.perpdir should be positive
	if (dist < r) return 4;
	return 0;
}

// ok, need thing to step down through bodies and find closest approach
// modify targpos directly to aim short of dangerous bodies
static bool ParentSafetyAdjust(DynamicBody *dBody, Frame *targframe, vector3d &targpos, vector3d &targvel)
{
	Body *body = 0;
	Frame *frame = targframe->GetNonRotFrame();
	while (frame)
	{
		if (dBody->GetFrame()->GetNonRotFrame() == frame) break;		// ship in frame, stop
		if (frame->GetBody()) body = frame->GetBody();			// ignore grav points?

		double sdist = dBody->GetPositionRelTo(frame).Length();
		if (sdist < frame->GetRadius()) break;					// ship inside frame, stop

		// we should always be inside the root frame, so if we're not inside 'frame'
		// then it must not be the root frame (ie, it must have a parent)
		assert(frame->GetParent());

		frame = frame->GetParent()->GetNonRotFrame();			// check next frame down
	}
	if (!body) return false;

	// aim for zero velocity at surface of that body
	// still along path to target
	Propulsion *prop = dBody->GetPropulsion();
	if (prop==nullptr) return false;
	vector3d targpos2 = targpos - dBody->GetPosition();
	double targdist = targpos2.Length();
	double bodydist = body->GetPositionRelTo(dBody).Length() - MaxEffectRad(body, prop)*1.5;
	if (targdist < bodydist) return false;
	targpos -= (targdist - bodydist) * targpos2 / targdist;
	targvel = body->GetVelocityRelTo(dBody->GetFrame());
	return true;
}

// check for collision course with frame body
// tandir is normal vector from planet to target pos or dir
static bool CheckSuicide(DynamicBody *dBody, const vector3d &tandir)
{
	Body *body = dBody->GetFrame()->GetBody();
	if (dBody->Have(DynamicBody::PROPULSION)) return false;
	Propulsion *prop = dBody->GetPropulsion();
	assert(prop!=nullptr);
	if (!body || !body->IsType(Object::TERRAINBODY)) return false;

	double vel = dBody->GetVelocity().Dot(tandir);		// vel towards is negative
	double dist = dBody->GetPosition().Length() - MaxFeatureRad(body);
	if (vel < -1.0 && vel*vel > 2.0*prop->GetAccelMin()*dist)
		return true;
	return false;
}


extern double calc_ivel(double dist, double vel, double acc);

// Fly to vicinity of body
AICmdFlyTo::AICmdFlyTo(DynamicBody *dBody, Body *target) : AICommand(dBody, CMD_FLYTO)
{
	m_prop = dBody->GetPropulsion();
	assert(m_prop!=nullptr);
	m_frame = 0; m_state = -6; m_lockhead = true; m_endvel = 0; m_tangent = false;
	if (!target->IsType(Object::TERRAINBODY)) m_dist = VICINITY_MIN;
	else m_dist = VICINITY_MUL*MaxEffectRad(target, m_prop);

	if (target->IsType(Object::SPACESTATION) && static_cast<SpaceStation*>(target)->IsGroundStation()) {
		m_posoff = target->GetPosition() + 15000.0 * target->GetOrient().VectorY();
//		m_posoff += 500.0 * target->GetOrient().VectorX();
		m_targframe = target->GetFrame(); m_target = 0;
	}
	else { m_target = target; m_targframe = 0; }

	if (dBody->GetPositionRelTo(target).Length() <= 15000.0) m_targframe = 0;
}

// Specified pos, endvel should be > 0
AICmdFlyTo::AICmdFlyTo(DynamicBody *dBody, Frame *targframe, const vector3d &posoff, double endvel, bool tangent) :
	AICommand(dBody, CMD_FLYTO),
	m_target(nullptr),
	m_targframe(targframe),
	m_posoff(posoff),
	m_endvel(endvel),
	m_tangent(tangent),
	m_state(-6),
	m_lockhead(true),
	m_frame(nullptr)
{
	m_prop = dBody->GetPropulsion();
	assert(m_prop!=nullptr);
}

bool AICmdFlyTo::TimeStepUpdate()
{
	/* TODO: ship is used ONLY to calls
	 * wheels, launch and flightstate, so
	 * it is better to split them in a module
	*/
	if (m_dBody->IsType(Object::SHIP)) {
		Ship *ship = static_cast<Ship*>(m_dBody);
		assert(ship!=nullptr);
		if (ship->GetFlightState() == Ship::JUMPING) return false;
		// sort out gear, launching
		if (ship->GetFlightState() == Ship::FLYING) ship->SetWheelState(false);
		else { LaunchShip(ship); return false; }
	} else {
		// may be an exploration probe ;-)
		return false;
	}
	if (!m_target && !m_targframe) return true;			// deleted object

	// generate base target pos (with vicinity adjustment) & vel
	double timestep = Pi::game->GetTimeStep();
	vector3d targpos, targvel;
	if (m_target) {
		targpos = m_target->GetPositionRelTo(m_dBody->GetFrame());
		targpos -= (targpos - m_dBody->GetPosition()).NormalizedSafe() * m_dist;
		targvel = m_target->GetVelocityRelTo(m_dBody->GetFrame());
	} else {
		targpos = GetPosInFrame(m_dBody->GetFrame(), m_targframe, m_posoff);
		targvel = GetVelInFrame(m_dBody->GetFrame(), m_targframe, m_posoff);
	}
	Frame *targframe = m_target ? m_target->GetFrame() : m_targframe;
	ParentSafetyAdjust(m_dBody, targframe, targpos, targvel);
	vector3d relpos = targpos - m_dBody->GetPosition();
	vector3d reldir = relpos.NormalizedSafe();
	vector3d relvel = targvel - m_dBody->GetVelocity();
	double targdist = relpos.Length();

#ifdef DEBUG_AUTOPILOT
if (m_ship->IsType(Object::PLAYER))
Output("Autopilot dist = %.1f, speed = %.1f, zthrust = %.2f, state = %i\n",
	targdist, relvel.Length(), m_ship->GetThrusterState().z, m_state);
#endif

	// frame switch stuff - clear children/collision state
	if (m_frame != m_dBody->GetFrame()) {
		if (m_child) { m_child.reset(); }
		if (m_tangent && m_frame) return true;		// regen tangent on frame switch
		m_reldir = reldir;							// for +vel termination condition
		m_frame = m_dBody->GetFrame();
	}

// TODO: collision needs to be processed according to vdiff, not reldir?

	Body *body = m_frame->GetBody();
	double erad = MaxEffectRad(body, m_prop);
	if ((m_target && body != m_target)
		|| (m_targframe && (!m_tangent || body != m_targframe->GetBody())))
	{
		int coll = CheckCollision(m_dBody, reldir, targdist, targpos, m_endvel, erad);
		if (coll == 0) {				// no collision
			if (m_child) { m_child.reset(); }
		}
		else if (coll == 1) {			// below feature height, target not below
			double ang = m_prop->AIFaceDirection(m_dBody->GetPosition());
			m_prop->AIMatchVel(ang < 0.05 ? 1000.0 * m_dBody->GetPosition().Normalized() : vector3d(0.0));
		}
		else {					// same thing for 2/3/4
			if (!m_child) m_child.reset(new AICmdFlyAround(m_dBody, m_frame->GetBody(), erad*1.05, 0.0));
			static_cast<AICmdFlyAround*>(m_child.get())->SetTargPos(targpos);
			ProcessChild();
		}
		if (coll) { m_state = -coll; return false; }
	}
	if (m_state < 0 && m_state > -6 && m_tangent) return true;			// bail out
	if (m_state < 0) m_state = targdist > 10000000.0 ? 1 : 0;			// still lame

	double maxdecel = m_state ? m_prop->GetAccelFwd() : m_prop->GetAccelRev();
	double gravdir = -reldir.Dot(m_dBody->GetPosition().Normalized());
	maxdecel -= gravdir * GetGravityAtPos(m_dBody->GetFrame(), m_dBody->GetPosition());
	bool bZeroDecel = false;
	if (maxdecel < 0) {
		maxdecel = 0.0;
		bZeroDecel = true;
	}

	// target ship acceleration adjustment
	if (m_target && m_target->IsType(Object::SHIP)) {
		Ship *targship = static_cast<Ship*>(m_target);
		matrix3x3d orient = m_target->GetFrame()->GetOrientRelTo(m_frame);
		vector3d targaccel = orient * targship->GetLastForce() / m_target->GetMass();
		// fudge: targets accelerating towards you are usually going to flip
		if (targaccel.Dot(reldir) < 0.0 && !targship->IsDecelerating()) targaccel *= 0.5;
		relvel += targaccel * timestep;
		maxdecel += targaccel.Dot(reldir);
		// if we have margin lower than 10%, fly as if 10% anyway
		maxdecel = std::max(maxdecel, 0.1*m_prop->GetAccelFwd());
	}

	const double curspeed = -relvel.Dot(reldir);
	const double tt = (bZeroDecel) ? timestep : std::max( sqrt(2.0*targdist / maxdecel), timestep );
	const vector3d perpvel = relvel + reldir * curspeed;
	double perpspeed = perpvel.Length();
	const vector3d perpdir = (perpspeed > 1e-30) ? perpvel / perpspeed : vector3d(0,0,1);

	double sidefactor = perpspeed / (tt*0.5);
	if (curspeed > (tt+timestep)*maxdecel || maxdecel < sidefactor) {
		m_prop->AIFaceDirection(relvel);
		m_prop->AIMatchVel(targvel);
		m_state = -5; return false;
	}
	else maxdecel = sqrt(maxdecel*maxdecel - sidefactor*sidefactor);

	// ignore targvel if we could clear with side thrusters in a fraction of minimum time
//	if (perpspeed < tt*0.01*m_ship->GetAccelMin()) perpspeed = 0;

	// calculate target speed
	double ispeed = (maxdecel < 1e-10) ? 0.0 : calc_ivel(targdist, m_endvel, maxdecel);

	// cap target speed according to spare fuel remaining
	double fuelspeed = m_prop->GetSpeedReachedWithFuel();
	if (m_target && m_target->IsType(Object::SHIP)) fuelspeed -=
		m_dBody->GetVelocityRelTo(Pi::game->GetSpace()->GetRootFrame()).Length();
	if (ispeed > curspeed && curspeed > 0.9*fuelspeed) ispeed = curspeed;

	// Don't exit a frame faster than some fraction of radius
//	double maxframespeed = 0.2 * m_frame->GetRadius() / timestep;
//	if (m_frame->GetParent() && ispeed > maxframespeed) ispeed = maxframespeed;

	// cap perpspeed according to what's needed now
	perpspeed = std::min(perpspeed, 2.0*sidefactor*timestep);

	// cap sdiff by thrust...
	double sdiff = ispeed - curspeed;
	double linaccel = sdiff < 0 ?
		std::max(sdiff, -m_prop->GetAccelFwd()*timestep) :
		std::min(sdiff, m_prop->GetAccelFwd()*timestep);

	// linear thrust application, decel check
	vector3d vdiff = linaccel*reldir + perpspeed*perpdir;
	bool decel = sdiff <= 0;
	// TODO: what is "SetDecelerating"??? => needs to be moved
	m_dBody->SetDecelerating(decel);
	if (decel) m_prop->AIChangeVelBy(vdiff * m_dBody->GetOrient());
	else m_prop->AIChangeVelDir(vdiff * m_dBody->GetOrient());

	// work out which way to head
	vector3d head = reldir;
	if (!m_state && sdiff < -1.2*maxdecel*timestep) m_state = 1;
	// if we're not coasting due to fuel constraints, and we're in the deceleration phase
	// then flip the ship so we can use our main thrusters to decelerate
	if (m_state && !is_zero_exact(sdiff) && sdiff < maxdecel*timestep*60) head = -head;
	if (!m_state && decel) sidefactor = -sidefactor;
	head = head*maxdecel + perpdir*sidefactor;

	// face appropriate direction
	if (m_state >= 3) m_prop->AIMatchAngVelObjSpace(vector3d(0.0));
	else m_prop->AIFaceDirection(head);
	if (body && body->IsType(Object::PLANET) && m_dBody->GetPosition().LengthSqr() < 2*erad*erad)
		m_prop->AIFaceUpdir(m_dBody->GetPosition());		// turn bottom thruster towards planet

	// termination conditions: check
	if (m_state >= 3) return true;					// finished last adjustment, hopefully
	if (m_endvel > 0.0) { if (reldir.Dot(m_reldir) < 0.9) return true; }
	else if (targdist < 0.5*m_prop->GetAccelMin()*timestep*timestep) m_state = 3;
	return false;
}

AICmdDock::AICmdDock(DynamicBody *dBody, SpaceStation *target) : AICommand(dBody, CMD_DOCK),
	m_target(target), m_state(eDockGetDataStart)
{
	Ship *ship = nullptr;
	if (!dBody->IsType(Object::SHIP)) return;
	ship = static_cast<Ship*>(dBody);
	assert(ship!=nullptr);

	m_prop = ship->GetPropulsion(); // dBody->GetPropulsion();
	assert(m_prop!=nullptr);

	double grav = GetGravityAtPos(m_target->GetFrame(), m_target->GetPosition());
	if (m_prop->GetAccelUp() < grav) {
		m_dBody->AIMessage(Ship::AIERROR_GRAV_TOO_HIGH);
		m_target = 0;			// bail out on next timestep call
	}
}

// m_state values:
// 0: get data for docking start pos
// 1: Fly to docking start pos
// 2: get data for docking end pos
// 3: Fly to docking end pos

bool AICmdDock::TimeStepUpdate()
{
	Ship *ship = nullptr;
	if (!ProcessChild()) return false;
	if (!m_target) return true;

	if (!m_dBody->IsType(Object::SHIP)) return false;
	ship = static_cast<Ship*>(m_dBody);
	assert(ship!=nullptr);

	// finished moving into dock start pos (done by child FlyTo command)
	if (m_state == eDockFlyToStart) IncrementState();

	// If we're docked with the target, then we're finished!
	if (ship->GetDockedWith() == m_target) {
		ship->ClearThrusterState();
		return true;
	}

	switch (ship->GetFlightState()) {
	case Ship::UNDOCKING: return false; // allow undock animation to proceed
	case Ship::DOCKED:
	case Ship::LANDED:
		LaunchShip(ship);
		return false;
	case Ship::JUMPING:
	case Ship::HYPERSPACE:
		return false;
	case Ship::FLYING:
	case Ship::DOCKING:
		break;
	}

	// if we're not close to target, do a flyto first
	double targdist = m_target->GetPositionRelTo(ship).Length();
	if (targdist > 16000.0) {
		m_child.reset(new AICmdFlyTo(m_dBody, m_target));
		ProcessChild(); return false;
	}

	int port = m_target->GetMyDockingPort(ship);
	if (port == -1) {
		std::string msg;
		const bool cleared = m_target->GetDockingClearance(ship, msg);
		port = m_target->GetMyDockingPort(ship);
		if (!cleared || (port == -1)) {
			ship->AIMessage(Ship::AIERROR_REFUSED_PERM);
			return true;
		}
	}

	// state 0,2: Get docking data
	if (m_state == eDockGetDataStart
		|| m_state == eDockGetDataEnd
		|| m_state == eDockingComplete)
	{
		const SpaceStationType *type = m_target->GetStationType();
		SpaceStationType::positionOrient_t dockpos;
		type->GetShipApproachWaypoints(port, (m_state==0)?1:2, dockpos);
		if (m_state != eDockGetDataEnd) {
			m_dockpos = dockpos.pos;
		}

		m_dockdir = dockpos.zaxis.Normalized();
		m_dockupdir = dockpos.yaxis.Normalized();		// don't trust these enough
		if (type->IsOrbitalStation()) {
			m_dockupdir = -m_dockupdir;
		} else if (m_state == eDockingComplete) {
			m_dockpos -= m_dockupdir * (ship->GetLandingPosOffset() + 0.1);
		}

		if (m_state != eDockGetDataEnd) {
			m_dockpos = m_target->GetOrient() * m_dockpos + m_target->GetPosition();
		}
		IncrementState();
		// should have m_dockpos in target frame, dirs relative to target orient
	}

	if (m_state == eDockFlyToStart) {			// fly to first docking waypoint
		m_child.reset(new AICmdFlyTo(m_dBody, m_target->GetFrame(), m_dockpos, 0.0, false));
		ProcessChild(); return false;
	}

	// second docking waypoint
	ship->SetWheelState(true);
	const vector3d targpos = GetPosInFrame(m_dBody->GetFrame(), m_target->GetFrame(), m_dockpos);
	const vector3d relpos = targpos - m_dBody->GetPosition();
	const vector3d reldir = relpos.NormalizedSafe();
	const vector3d relvel = -m_target->GetVelocityRelTo(m_dBody);

	const double maxdecel = m_prop->GetAccelUp() - GetGravityAtPos(m_target->GetFrame(), m_dockpos);
	const double ispeed = calc_ivel(relpos.Length(), 0.0, maxdecel);
	const vector3d vdiff = ispeed*reldir - relvel;
	m_prop->AIChangeVelDir(vdiff * m_dBody->GetOrient());
	if (vdiff.Dot(reldir) < 0) {
		m_dBody->SetDecelerating(true);
	}

	// get rotation of station for next frame
	matrix3x3d trot = m_target->GetOrientRelTo(m_dBody->GetFrame());
	double av = m_target->GetAngVelocity().Length();
	double ang = av * Pi::game->GetTimeStep();
	if (ang > 1e-16) {
		vector3d axis = m_target->GetAngVelocity().Normalized();
		trot = trot * matrix3x3d::Rotate(ang, axis);
	}
	double af;
	if (m_target->GetStationType()->IsOrbitalStation()) {
		af = m_prop->AIFaceDirection(trot * m_dockdir);
	} else {
		af = m_prop->AIFaceDirection(m_dBody->GetPosition().Cross(m_dBody->GetOrient().VectorX()));
	}
	if (af < 0.01) {
		af = m_prop->AIFaceUpdir(trot * m_dockupdir, av) - ang;
	}
	if (m_state < eInvalidDockingStage && af < 0.01 && ship->GetWheelState() >= 1.0f) {
		IncrementState();
	}

#ifdef DEBUG_AUTOPILOT
Output("AICmdDock dist = %.1f, speed = %.1f, ythrust = %.2f, state = %i\n",
	targdist, relvel.Length(), m_ship->GetThrusterState().y, m_state);
#endif

	return false;
}

bool AICmdHoldPosition::TimeStepUpdate()
{
	// XXX perhaps try harder to move back to the original position
	m_prop->AIMatchVel(vector3d(0,0,0));
	return false;
}

void AICmdFlyAround::Setup(Body *obstructor, double alt, double vel, int mode)
{
	assert(!std::isnan(alt));
	assert(!std::isnan(vel));
	m_obstructor = obstructor; m_alt = alt; m_vel = vel; m_targmode = mode;

	// push out of effect radius (gravity safety & station parking zones)
	alt = std::max(alt, MaxEffectRad(obstructor, m_prop));

	// drag within frame because orbits are impossible otherwise
	// timestep code also doesn't work correctly for ex-frame cases, should probably be fixed 
	alt = std::min(alt, 0.95 * obstructor->GetFrame()->GetNonRotFrame()->GetRadius());

	// generate suitable velocity if none provided
	double minacc = (mode == 2) ? 0 : m_prop->GetAccelMin();
	double mass = obstructor->IsType(Object::TERRAINBODY) ? obstructor->GetMass() : 0;
	if (vel < 1e-30) m_vel = sqrt(m_alt*0.8*minacc + mass*G/m_alt);
}

AICmdFlyAround::AICmdFlyAround(DynamicBody *dBody, Body *obstructor, double relalt, int mode)
	: AICommand (dBody, CMD_FLYAROUND)
{
	m_prop = dBody->GetPropulsion();
	assert(m_prop!=nullptr);

	assert(!std::isnan(relalt));
	double alt = relalt * obstructor->GetPhysRadius();
	assert(!std::isnan(alt));
	Setup(obstructor, alt, 0.0, mode);
}

AICmdFlyAround::AICmdFlyAround(DynamicBody *dBody, Body *obstructor, double alt, double vel, int mode)
	: AICommand (dBody, CMD_FLYAROUND)
{
	m_prop = dBody->GetPropulsion();
	assert(m_prop!=nullptr);

	assert(!std::isnan(alt));
	Setup(obstructor, alt, vel, mode);
}

double AICmdFlyAround::MaxVel(double targdist, double targalt)
{
	Propulsion *prop = m_dBody->GetPropulsion();
	assert(prop!=0);

	if (targalt > m_alt) return m_vel;
	double t = sqrt(2.0 * targdist / prop->GetAccelFwd());
	double vmaxprox = prop->GetAccelMin()*t;			// limit by target proximity
	double vmaxstep = std::max(m_alt*0.05, m_alt-targalt);
	vmaxstep /= Pi::game->GetTimeStep();			// limit by distance covered per timestep
	return std::min(m_vel, std::min(vmaxprox, vmaxstep));
}

bool AICmdFlyAround::TimeStepUpdate()
{
	if (m_dBody->IsType(Object::SHIP)) {
		Ship *ship = nullptr;
		ship = static_cast<Ship*>(m_dBody);
		assert(ship!=0);

		if (ship->GetFlightState() == Ship::JUMPING) return false;
		if (!ProcessChild()) return false;

		// Not necessary unless it's a tier 1 AI
		if (ship->GetFlightState() == Ship::FLYING) ship->SetWheelState(false);
		else { LaunchShip(ship); return false; }

	} else;// return false;

	double timestep = Pi::game->GetTimeStep();
	vector3d targpos = (!m_targmode) ? m_targpos :
		m_dBody->GetVelocity().NormalizedSafe()*m_dBody->GetPosition().LengthSqr();
	vector3d obspos = m_obstructor->GetPositionRelTo(m_dBody);
	double obsdist = obspos.Length();
	vector3d obsdir = obspos / obsdist;
	vector3d relpos = targpos - m_dBody->GetPosition();

	// frame body suicide check, response
	if (CheckSuicide(m_dBody, -obsdir)) {
		m_prop->AIFaceDirection(m_dBody->GetPosition());		// face away from planet
		m_prop->AIMatchVel(vector3d(0.0)); return false;
	}

	// if too far away, fly to tangent
	if (obsdist > 1.1*m_alt)
	{
		double v;
		Frame *obsframe = m_obstructor->GetFrame()->GetNonRotFrame();
		vector3d tangent = GenerateTangent(m_dBody, obsframe, targpos, m_alt);
		vector3d tpos_obs = GetPosInFrame(obsframe, m_dBody->GetFrame(), targpos);
		if (m_targmode) v = m_vel;
		else if (relpos.LengthSqr() < obsdist + tpos_obs.LengthSqr()) v = 0.0;
		else v = MaxVel((tpos_obs-tangent).Length(), tpos_obs.Length());
		m_child.reset(new AICmdFlyTo(m_dBody, obsframe, tangent, v, true));
		ProcessChild(); return false;
	}

	// limit m_vel by target proximity & distance covered per frame
	double vel = (m_targmode) ? m_vel : MaxVel(relpos.Length(), targpos.Length());

	// all calculations in ship's frame
	vector3d fwddir = (obsdir.Cross(relpos).Cross(obsdir)).NormalizedSafe();
	vector3d tanvel = vel * fwddir;

	// max feature avoidance check, response
	if (obsdist < MaxFeatureRad(m_obstructor)) {
		double ang = m_prop->AIFaceDirection(-obsdir);
		m_prop->AIMatchVel(ang < 0.05 ? 1000.0 * -obsdir : vector3d(0.0));
		return false;
	}

	// calculate target velocity
	double alt = (tanvel * timestep + obspos).Length();		// unnecessary?
	double ivel = calc_ivel(alt - m_alt, 0.0, m_prop->GetAccelMin());

	vector3d finalvel = tanvel + ivel * obsdir;
	m_prop->AIMatchVel(finalvel);
	m_prop->AIFaceDirection(fwddir);
	m_prop->AIFaceUpdir(-obsdir);

//	vector3d newhead = GenerateTangent(m_ship, m_obstructor->GetFrame(), fwddir);
//	newhead = GetPosInFrame(m_ship->GetFrame(), m_obstructor->GetFrame(), newhead);
//	m_ship->AIFaceDirection(newhead-m_ship->GetPosition());

	// termination condition for orbits
	vector3d thrust = m_prop->GetThrusterState();
	if (m_targmode >= 2 && thrust.LengthSqr() < 0.01) m_targmode++;
	if (m_targmode == 4) { m_prop->SetThrusterState(vector3d(0.0)); return true; }
	return false;
}

AICmdFormation::AICmdFormation(DynamicBody *dBody, DynamicBody *target, const vector3d &posoff) :
	AICommand(dBody, CMD_FORMATION),
	m_target(target),
	m_posoff(posoff)
{
	m_prop = dBody->GetPropulsion();
	assert(m_prop!=nullptr);
}

bool AICmdFormation::TimeStepUpdate()
{
	if (m_dBody->IsType(Object::SHIP)) {
		Ship *ship = static_cast<Ship*>(m_dBody);
		assert(ship!=0);

		if (ship->GetFlightState() == Ship::JUMPING) return false;
		if (ship->GetFlightState() == Ship::FLYING) ship->SetWheelState(false);
		else { LaunchShip(ship); return false; }
	}
	if (!m_target) return true;
	if (!ProcessChild()) return false;		// In case we're doing an intercept


	// if too far away, do an intercept first
	// TODO: adjust distance cap by timestep so we don't bounce?
	if (m_target->GetPositionRelTo(m_dBody).Length() > 30000.0) {
		m_child.reset(new AICmdFlyTo(m_dBody, m_target));
		ProcessChild(); return false;
	}

	matrix3x3d torient = m_target->GetOrientRelTo(m_dBody->GetFrame());
	vector3d relpos = m_target->GetPositionRelTo(m_dBody) + torient * m_posoff;
	vector3d relvel = -m_target->GetVelocityRelTo(m_dBody);
	double targdist = relpos.Length();
	vector3d reldir = (targdist < 1e-16) ? vector3d(1,0,0) : relpos/targdist;

	// adjust for target acceleration
	matrix3x3d forient = m_target->GetFrame()->GetOrientRelTo(m_dBody->GetFrame());
	vector3d targaccel = forient * m_target->GetLastForce() / m_target->GetMass();
	relvel -= targaccel * Pi::game->GetTimeStep();
	double maxdecel = m_prop->GetAccelFwd() + targaccel.Dot(reldir);
	if (maxdecel < 0.0) maxdecel = 0.0;

	// linear thrust
	double ispeed = calc_ivel(targdist, 0.0, maxdecel);
	vector3d vdiff = ispeed*reldir - relvel;
	m_prop->AIChangeVelDir(vdiff * m_dBody->GetOrient());
	if (m_target->IsType(Object::SHIP)) {
		Ship *target_ship = static_cast<Ship*>(m_target);
		if (target_ship->IsDecelerating()) m_dBody->SetDecelerating(true);
	} else {
		m_dBody->SetDecelerating(false);
	}

	m_prop->AIFaceDirection(-torient.VectorZ());
	return false;					// never self-terminates
}
