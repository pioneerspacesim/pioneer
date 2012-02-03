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


static const double VICINITY_MIN = 5000.0;
static const double VICINITY_MUL = 4.0;

AICommand *AICommand::Load(Serializer::Reader &rd)
{
	CmdName name = CmdName(rd.Int32());
	switch (name) {
		case CMD_NONE: default: return 0;
//		case CMD_JOURNEY: return new AICmdJourney(rd);
		case CMD_DOCK: return new AICmdDock(rd);
		case CMD_FLYTO: return new AICmdFlyTo(rd);
		case CMD_FLYAROUND: return new AICmdFlyAround(rd);
		case CMD_KILL: return new AICmdKill(rd);
		case CMD_KAMIKAZE: return new AICmdKamikaze(rd);
		case CMD_HOLDPOSITION: return new AICmdHoldPosition(rd);
	}
}

void AICommand::Save(Serializer::Writer &wr)
{
	Space *space = Pi::game->GetSpace();
	wr.Int32(m_cmdName);
	wr.Int32(space->GetIndexForBody(m_ship));
	if (m_child) m_child->Save(wr);
	else wr.Int32(CMD_NONE);
}

AICommand::AICommand(Serializer::Reader &rd, CmdName name)
{
	m_cmdName = name;
	m_shipIndex = rd.Int32();
	m_child = Load(rd);
}

void AICommand::PostLoadFixup(Space *space)
{
	m_ship = static_cast<Ship *>(space->GetBodyByIndex(m_shipIndex));
	if (m_child) m_child->PostLoadFixup(space);
}


bool AICommand::ProcessChild()
{
	if (!m_child) return true;						// no child present
	if (!m_child->TimeStepUpdate()) return false;	// child still active
	delete m_child; m_child = 0;
	return true;								// child finished
}


/*
bool AICmdJourney::TimeStepUpdate()
{
	if (!ProcessChild()) return false;

	if (Pi::game->GetSpace()->GetStarSystem()->GetLocation() != (SysLoc)m_dest) {
		// need to hyperspace there
		int fuelRequired;
		double duration;
		enum Ship::HyperjumpStatus jumpStatus;
		m_ship->CanHyperspaceTo(&m_dest, fuelRequired, duration, &jumpStatus);
		if (jumpStatus == Ship::HYPERJUMP_OK) {
			switch (m_ship->GetFlightState()) {
			case Ship::FLYING:
				m_ship->TryHyperspaceTo(&m_dest);
				break;
			case Ship::DOCKING:
				// just wait
				break;
			case Ship::LANDED:
				if (m_ship->GetDockedWith()) {
					m_ship->Undock();
				} else {
					m_ship->Blastoff();
				}
				break;
			}
		} else {
			printf("AICmdJourney() can't get to destination (reason %d) :-(\n", (int)jumpStatus);
			if (!m_ship->GetDockedWith()) {
				// if we aren't docked then there is no point trying to
				// buy fuel, etc. just give up
				printf("AICmdJourney() failed (not docked, HyperjumpStatus=%d)\n", (int)jumpStatus);
				return true;
			}

			switch (jumpStatus) {		// todo: garbage that needs sorting
			case Ship::HYPERJUMP_INSUFFICIENT_FUEL:
				{
					Equip::Type fuelType = m_ship->GetHyperdriveFuelType();

					if (m_ship->BuyFrom(m_ship->GetDockedWith(), fuelType, false)) {
						// good. let's see if we are able to jump next tick
						return false;
					} else {
						printf("AICmdJourney() failed (docked, HyperjumpStatus=%d)\n", (int)jumpStatus);
						return true;
					}
				}
				break;
			case Ship::HYPERJUMP_NO_DRIVE:
			case Ship::HYPERJUMP_OUT_OF_RANGE:
				{
					const Equip::Type fuelType = m_ship->GetHyperdriveFuelType();
					const Equip::Type driveType = m_ship->m_equipment.Get(Equip::SLOT_ENGINE);
					const Equip::Type laserType = m_ship->m_equipment.Get(Equip::SLOT_LASER, 0);
					// preserve money
					Sint64 oldMoney = m_ship->GetMoney();
					m_ship->SetMoney(10000000);
					MarketAgent *trader = m_ship->GetDockedWith();
					// need to lose some equipment and see if we get light enough
					Equip::Type t = (Equip::Type)Pi::rng.Int32(Equip::TYPE_MAX);
					if ((Equip::types[t].slot == Equip::SLOT_ENGINE) && trader->CanSell(t)) {
						// try a different hyperdrive
						m_ship->SellTo(trader, driveType);
						if (!m_ship->BuyFrom(trader, t)) {
							m_ship->BuyFrom(trader, driveType);
						}
						printf("Switched drive to a %s\n", Equip::types[t].name);
					} else if ((t != fuelType) && (t != driveType) && (t != laserType)) {
						m_ship->SellTo(trader, t);
						printf("Removed a %s\n", Equip::types[t].name);
					}
					m_ship->SetMoney(oldMoney);
				}
				break;
			case Ship::HYPERJUMP_OK:
				break; // shouldn't reach this though
			}
		}
	} else if (m_ship->GetFlightState() == Ship::LANDED) return true;	// all done
	else {
		// we are in the desired system. fly to the target and dock
		// then specific instructions to get us there
		Body *b = Space::FindBodyForSBodyPath(&m_dest);
		if (b->IsType(Object::SPACESTATION))
			m_child = new AICmdDock(m_ship, static_cast<SpaceStation*>(b));
		else m_child = new AICmdFlyTo(m_ship, b);
	}
	return false;
}

*/

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
	if (!m_target) return true;

	if (m_ship->GetFlightState() == Ship::FLYING) m_ship->SetWheelState(false);
	else { LaunchShip(m_ship); return false; }

	m_ship->SetGunState(0,0);
	// needs to deal with frames, large distances, and success
	if (m_ship->GetFrame() == m_target->GetFrame()) {
		double dist = (m_target->GetPosition() - m_ship->GetPosition()).Length();
		vector3d vRel = m_ship->GetVelocityRelTo(m_target);
		vector3d dir = (m_target->GetPosition() - m_ship->GetPosition()).Normalized();

		const double eta = Clamp(dist / vRel.Dot(dir), 0.0, 10.0);
		const vector3d enemyProjectedPos = m_target->GetPosition() + eta*m_target->GetVelocity() - eta*m_ship->GetVelocity();
		dir = (enemyProjectedPos - m_ship->GetPosition()).Normalized();

		m_ship->ClearThrusterState();
		m_ship->AIFaceDirection(dir);

		// thunder at target at 400m/sec
		// todo: fix that static cast - redo this function anyway
		m_ship->AIModelCoordsMatchSpeedRelTo(vector3d(0,0,-400), static_cast<Ship*>(m_target));
	}
	return false;
}

bool AICmdKill::TimeStepUpdate()
{
	if (!m_target) return true;

	if (m_ship->GetFlightState() == Ship::FLYING) m_ship->SetWheelState(false);
	else { LaunchShip(m_ship); return false; }

	matrix4x4d rot; m_ship->GetRotMatrix(rot);				// some world-space params
	const ShipType &stype = m_ship->GetShipType();
	vector3d targpos = m_target->GetPositionRelTo(m_ship);
	vector3d targvel = m_target->GetVelocityRelTo(m_ship);		
	vector3d targdir = targpos.NormalizedSafe();
	vector3d heading = vector3d(-rot[8], -rot[9], -rot[10]);
	// Accel will be wrong for a frame on timestep changes, but it doesn't matter
	vector3d targaccel = (m_target->GetVelocity() - m_lastVel) / Pi::game->GetTimeStep();
	m_lastVel = m_target->GetVelocity();		// may need next frame
	vector3d leaddir = m_ship->AIGetLeadDir(m_target, targaccel, 0);

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

		double vissize = 1.3 * m_ship->GetBoundingRadius() / targpos.Length();
		vissize += (0.05 + 0.5*leaddiff)*Pi::rng.Double()*skillShoot;
		if (vissize > headdiff) m_ship->SetGunState(0,1);
		else m_ship->SetGunState(0,0);
		if (targpos.LengthSqr() > 4000*4000) m_ship->SetGunState(0,0);		// temp
	}
	m_leadOffset += m_leadDrift * Pi::game->GetTimeStep();
	double leadAV = (leaddir-targdir).Dot((leaddir-heading).NormalizedSafe());	// leaddir angvel
	m_ship->AIFaceDirection((leaddir + m_leadOffset).Normalized(), leadAV);


	vector3d evadethrust(0,0,0);
	if (m_evadeTime < Pi::game->GetTime())		// evasion time!
	{
		double skillEvade = 0.5;			// todo: should come from AI stats
		m_evadeTime = Pi::game->GetTime() + Pi::rng.Double(3.0,10.0) * skillEvade;
		if (heading.Dot(targdir) < 0.7) skillEvade += 0.5;		// not in view
		skillEvade += Pi::rng.Double(-0.5,0.5);

		matrix4x4d trot; m_target->GetRotMatrix(trot);
		vector3d targhead = vector3d(-trot[8], -trot[9], -trot[10]) * rot;		// obj space
		vector3d targav = m_target->GetAngVelocity();

		if (skillEvade < 1.6 && targhead.z < 0.0) {		// smart chase
			vector3d objvel = targvel * rot;		// obj space targvel
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
	else evadethrust = m_ship->GetThrusterState();


	// todo: some logic behind desired range? pass from higher level
	if (m_closeTime < Pi::game->GetTime())
	{
		double skillEvade = 0.5;
		if (heading.Dot(targdir) < 0.7) skillEvade += 0.5;		// not in view

		m_closeTime = Pi::game->GetTime() + skillEvade * Pi::rng.Double(1.0,5.0);
	
		double reqdist = 500.0 + skillEvade * Pi::rng.Double(-500.0, 250);
		double dist = targpos.Length(), ispeed;
		double rearaccel = stype.linThrust[ShipType::THRUSTER_REVERSE] / m_ship->GetMass();
		rearaccel += targaccel.Dot(targdir);
		// v = sqrt(2as), positive => towards
		double as2 = 2.0 * rearaccel * (dist - reqdist);
		if (as2 > 0) ispeed = sqrt(as2); else ispeed = -sqrt(-as2);
		double vdiff = ispeed + targvel.Dot(targdir);

		if (skillEvade + Pi::rng.Double() > 1.5) evadethrust.z = 0.0;
		else if (vdiff*vdiff < 400.0) evadethrust.z = 0.0;
		else evadethrust.z = (vdiff > 0.0) ? -1.0 : 1.0;
	}
	else evadethrust.z = m_ship->GetThrusterState().z;
	m_ship->SetThrusterState(evadethrust);

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
	leadpos = targpos + targvel*(leadpos.Length()/projspeed); 	// second order approx
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

static Frame *GetNonRotFrame(Body *body)
{
	if (!body->HasDoubleFrame()) return body->GetFrame();
	else return body->GetFrame()->m_parent;
}

static double MaxFeatureRad(Body *body)
{
	if(!body) return 0.0;
	if(!body->IsType(Object::TERRAINBODY)) return body->GetBoundingRadius();
	return static_cast<TerrainBody *>(body)->GetMaxFeatureRadius() + 200.0;		// + building height
}

static double MaxEffectRad(Body *body, Ship *ship)
{
	if(!body) return 0.0;
	if(!body->IsType(Object::TERRAINBODY)) {
		double brad = body->GetBoundingRadius();
		return std::max(brad*1.1, brad+2000.0);
	}
	double frad = static_cast<TerrainBody *>(body)->GetMaxFeatureRadius() * 1.1;
	return std::max(frad, sqrt(G * body->GetMass() / ship->GetAccelMin()));
}

// returns acceleration due to gravity at that point
static double GetGravityAtPos(Frame *targframe, const vector3d &posoff)
{
	Body *body = targframe->GetBodyFor();
	if (!body || body->IsType(Object::SPACESTATION)) return 0;
	double rsqr = posoff.LengthSqr();
	return G * body->GetMass() / rsqr;
	// inverse is: sqrt(G * m1m2 / thrust)
}

// gets position of (target + offset in target's frame) in frame
// if object has its own rotational frame, ignores it
static vector3d GetPosInFrame(Frame *frame, Frame *target, const vector3d &offset)
{
	matrix4x4d m; Frame::GetFrameTransform(target, frame, m);
	return m * offset;
}

static vector3d GetVelInFrame(Frame *frame, Frame *target, const vector3d &offset)
{
	matrix4x4d m; vector3d vel(0.0);
	Frame::GetFrameTransform(target, frame, m);
	if (target != frame) vel = -target->GetStasisVelocityAtPosition(offset);
	return (m.ApplyRotationOnly(vel) + Frame::GetFrameRelativeVelocity(frame, target));
}

// generates from (0,0,0) to spos, in plane of target 
// formula uses similar triangles
// shiptarg in ship's frame
// output in targframe
static vector3d GenerateTangent(Ship *ship, Frame *targframe, const vector3d &shiptarg, double alt)
{
	matrix4x4d tran;
	Frame::GetFrameTransform(ship->GetFrame(), targframe, tran);
	vector3d spos = tran * ship->GetPosition();
	vector3d targ = tran * shiptarg;
	double a = spos.Length(), b = alt;
	if (b*1.02 > a) { spos *= b*1.02/a; a = b*1.02; }		// fudge if ship gets under radius
	double c = sqrt(a*a - b*b);
	return (spos*b*b)/(a*a) + spos.Cross(targ).Cross(spos).Normalized()*b*c/a;
}

static int GetFlipMode(Ship *ship, const vector3d &relpos, const vector3d &relvel)
{
	double dist = relpos.Length();
	double vel = relvel.Dot(relpos) / dist;
	if (vel > 0.0 && vel*vel > 1.7 * ship->GetAccelRev() * dist) return 2;
	if (dist > 100000000.0) return 1;	// arbitrary
	return 0;
}

static double GetMaxDecel(Ship *ship, const vector3d &reldir, int flip, double *angr=0)
{
	matrix4x4d rot; ship->GetRotMatrix(rot);
	vector3d heading = vector3d(-rot[8], -rot[9], -rot[10]);
	double ang = heading.Dot(reldir);
	if (angr) *angr = ang;

	if (flip != 1 && flip != 2) { if (ang > 0.99) return ship->GetAccelRev(); }
	else if (ang < -0.99 || ang > 0.99) return ship->GetAccelFwd();
	return ship->GetAccelMin();
}

// check whether ship is at risk of colliding with frame body on current path
// return values:
//0 - no collision
//1 - below feature height
//2 - unsafe escape from effect radius
//3 - unsafe entry to effect radius
//4 - probable path intercept
static int CheckCollision(Ship *ship, const vector3d &pathdir, double pathdist, const vector3d &tpos, double endvel, double r)
{
	// ship is in obstructor's frame anyway, so is tpos
	if (pathdist < 100.0) return 0;
	Body *body = ship->GetFrame()->GetBodyFor();
	vector3d spos = ship->GetPosition();
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
	double perpspeed = ship->GetVelocity().Dot(perpdir);
	double parspeed = ship->GetVelocity().Dot(pathdir);
	if (parspeed < 0) parspeed = 0;			// shouldn't break any important case
	if (perpspeed > 0) perpspeed = 0;		// prevent attempts to speculatively fly through planets

	// find time that ship will pass through that point
	// get velocity as if accelerating from start or end, pick smallest
	double ivelsqr = endvel*endvel + 2*ship->GetAccelFwd()*(pathdist-tanlen);		// could put endvel in here
	double fvelsqr = parspeed*parspeed + 2*ship->GetAccelFwd()*tanlen;
	double tanspeed = sqrt(ivelsqr < fvelsqr ? ivelsqr : fvelsqr);
	double time = tanlen / (0.5 * (parspeed + tanspeed));		// actually correct?

	double dist = spos.Dot(perpdir) + perpspeed*time;		// spos.perpdir should be positive
	if (dist < r) return 4;
	return 0;
}

// ok, need thing to step down through bodies and find closest approach
// modify targpos directly to aim short of dangerous bodies
static bool ParentSafetyAdjust(Ship *ship, Frame *targframe, const vector3d &posoff, vector3d &targpos)
{
	Body *body = 0;
	Frame *frame = targframe->IsRotatingFrame() ? targframe->m_parent : targframe;
	while (frame)
	{
		double sdist = ship->GetPositionRelTo(frame).Length();				// ship position in that frame
		if (sdist < frame->GetRadius()) break;

		while (frame && !(body = frame->GetBodyFor()))
			frame = frame->m_parent;
		if (!frame) return false;

		frame = body->GetFrame()->m_parent;
		if (body->HasDoubleFrame()) frame = frame->m_parent;
	}
	if (!body) return false;

	// ok, so if body != 0, aim for zero velocity at distance to surface of that body
	// still along path to target

	vector3d targpos2 = targpos - ship->GetPosition();
	double targdist = targpos2.Length();
	double bodydist = body->GetPositionRelTo(ship).Length() - MaxEffectRad(body, ship)*1.5;
	if (targdist < bodydist) return false;
	targpos -= (targdist - bodydist) * targpos2 / targdist;
//	printf("Adjusted targpos for safety from %s: old = %.1f, new = %.1f\n",
//		body->GetLabel().c_str(), targdist, (targpos-ship->GetPosition()).Length());
	return true;
}

// alternative plane-based version
//	vector3d bodypos = body->GetPositionRelTo(ship);		// body position relative to ship
//	double bodydist = bodypos.Length();
//	vector3d planenorm = bodypos / bodydist;
//	vector3d planedist = bodydist - MaxFeatureRad(body);

//	vector3d targpos2 = targpos - ship->GetPosition();
//	if (targpos2.Dot(planenorm) < planedist) return false;		// target closer than body-feature
//	targpos *= planedist / targpos2.Dot(planenorm);
//	return true;


// check for collision course with frame body
// tandir is normal vector from planet to target pos or dir
static bool CheckSuicide(Ship *ship, const vector3d &tandir)
{
	Body *body = ship->GetFrame()->GetBodyFor();
	if (!body || !body->IsType(Object::TERRAINBODY)) return false;

	double vel = ship->GetVelocity().Dot(tandir);		// vel towards is negative
	double dist = ship->GetPosition().Length() - MaxFeatureRad(body);
	if (vel < -1.0 && vel*vel > 2.0*ship->GetAccelMin()*dist)
		return true;
	return false;
}

// check for inability to reach target waypoint without overshooting
static bool CheckOvershoot(Ship *ship, const vector3d &reldir, double targdist, const vector3d &relvel, double endvel)
{
	if (targdist < 100.0) return false;		// spazzes out occasionally otherwise
	// only slightly fake minimum time to target
	// based on s = (sv+ev)*t/2 + a*t*t/4
	double fwdacc = ship->GetAccelFwd();
	double u = 0.5 * (relvel.Dot(reldir) + endvel);	if (u<0) u = 0;
	double t = (-u + sqrt(u*u + fwdacc*targdist)) / (fwdacc * 0.5);
	if (t < Pi::game->GetTimeStep()) t = Pi::game->GetTimeStep();
//	double t2 = ship->AITravelTime(reldir, targdist, relvel, endvel, true);

	// check for uncorrectable side velocity
	vector3d perpvel = relvel - reldir * relvel.Dot(reldir);
	if (perpvel.Length() > 0.5*ship->GetAccelMin()*t)
		return true;
	return false;
}


// Fly to "vicinity" of body
AICmdFlyTo::AICmdFlyTo(Ship *ship, Body *target) : AICommand (ship, CMD_FLYTO)
{
	double dist = std::max(VICINITY_MIN, VICINITY_MUL*MaxEffectRad(target, ship));
	if (target->IsType(Object::SPACESTATION) && static_cast<SpaceStation *>(target)->IsGroundStation()) {
		matrix4x4d rot; target->GetRotMatrix(rot);
		m_posoff = target->GetPosition() + dist * vector3d(rot[4], rot[5], rot[6]);		// up vector for starport
		m_targframe = target->GetFrame();
	}
	else {
		m_targframe = GetNonRotFrame(target);
		m_posoff = dist * m_ship->GetPositionRelTo(m_targframe).Normalized();
		m_posoff += target->GetPosition();
	}

	m_endvel = 0; m_tangent = 0;
	m_state = -1; m_frame = 0;

	// check if we're already close enough
	if (dist > m_ship->GetPositionRelTo(target).Length()) m_state = 5;
}

// Specified pos, endvel should be > 0
AICmdFlyTo::AICmdFlyTo(Ship *ship, Frame *targframe, const vector3d &posoff, double endvel, bool tangent)
	: AICommand (ship, CMD_FLYTO)
{
	m_targframe = targframe;
	m_posoff = posoff;
	m_endvel = endvel;
	m_state = -1; m_frame = 0;
	m_tangent = tangent;
}

extern double calc_ivel(double dist, double vel, double acc);

bool AICmdFlyTo::TimeStepUpdate()
{
	double timestep = Pi::game->GetTimeStep();
	vector3d targvel = GetVelInFrame(m_ship->GetFrame(), m_targframe, m_posoff);
	vector3d relvel = m_ship->GetVelocity() - targvel;
	vector3d targpos = GetPosInFrame(m_ship->GetFrame(), m_targframe, m_posoff);
	ParentSafetyAdjust(m_ship, m_targframe, m_posoff, targpos);
	vector3d relpos = targpos - m_ship->GetPosition();
	vector3d reldir = relpos.NormalizedSafe();
	double targdist = relpos.Length();

	// sort out gear, launching
	if (m_ship->GetFlightState() == Ship::FLYING) m_ship->SetWheelState(false);
	else { LaunchShip(m_ship); return false; }

	// frame switch stuff - clear children/collision state
	if (m_frame != m_ship->GetFrame()) {
		if (m_child) { delete m_child; m_child = 0; }
		m_frame = m_ship->GetFrame();
		m_reldir = reldir;							// for +vel termination condition
	}

#ifdef DEBUG_AUTOPILOT
printf("Autopilot dist = %.1f, speed = %.1f, zthrust = %.2f, term = %.3f, state = %i\n",
	targdist, relvel.Length(), m_ship->GetThrusterState().z, reldir.Dot(m_reldir), m_state);
#endif

	Body *body = m_frame->GetBodyFor();
	double erad = MaxEffectRad(body, m_ship);
	if (!m_tangent || !(body == m_targframe->GetBodyFor()))
	{
		// process path collisions with frame body
		int coll = CheckCollision(m_ship, reldir, targdist, targpos, m_endvel, erad);
		if (coll == 0) {				// no collision
			if (m_child) { delete m_child; m_child = 0; m_state = -1; }
		}
		else if (coll == 1) {			// below feature height, target not below
			double ang = m_ship->AIFaceDirection(m_ship->GetPosition());
			m_ship->AIMatchVel(ang < 0.05 ? 1000.0 * m_ship->GetPosition().Normalized() : 0.0);
			m_state = -3; return false;
		}
		else {							// same thing for 2/3/4
			if (!m_child) m_child =
				new AICmdFlyAround(m_ship, body, erad, 0.0, m_targframe, m_posoff);
			ProcessChild(); m_state = -5; return false;
		}
	}
	
	// if dangerously close to local body, pretend target isn't moving
	double localdist = m_ship->GetPosition().Length();
	if (targdist > localdist && localdist < 1.5*MaxFeatureRad(body))
		relvel += targvel;

	// regenerate state to flipmode if we're off course
	bool overshoot = CheckOvershoot(m_ship, reldir, targdist, relvel, m_endvel);
	if (m_tangent && m_state == -4 && !overshoot) return true;			// bail out
	if (m_state < 0) m_state = GetFlipMode(m_ship, relpos, relvel);

	// linear thrust
	double ang, maxdecel = GetMaxDecel(m_ship, reldir, m_state, &ang);
	maxdecel -= GetGravityAtPos(m_targframe, m_posoff);
	if(maxdecel <= 0) { m_ship->AIMessage(Ship::AIERROR_GRAV_TOO_HIGH); return true; }
	bool cap = m_ship->AIMatchPosVel2(reldir, targdist, relvel, m_endvel, maxdecel);

	// path overshoot check, response
	if (m_state < 3 && overshoot) {
		double ispeed = calc_ivel(targdist, m_endvel, maxdecel);
		m_ship->AIFaceDirection(ispeed*reldir - relvel);
		m_state = -4; return false;
	}

	// flip check - if facing forward and not accelerating at maximum
	if (m_state == 1 && ang > 0.99 && !cap) m_state = 2;

	// termination conditions
	if (m_state == 3) m_state++;					// finished last adjustment, hopefully
	else if (m_endvel > 0.0) { if (reldir.Dot(m_reldir) < 0.9) m_state = 4; }
	else if (targdist < 0.5*m_ship->GetAccelMin()*timestep*timestep) m_state = 3;

	// set heading according to current state
	if (m_state < 2) {		// this shit still needed? yeah, sort of
		vector3d newrelpos = targpos - m_ship->AIGetNextFramePos();
		if ((newrelpos + reldir*100.0).Dot(relpos) <= 0.0) 
			m_ship->AIFaceDirection(reldir);			// last frames turning workaround
		else m_ship->AIFaceDirection(newrelpos);
	}
	else if (m_state == 2) m_ship->AIFaceDirection(-reldir);	// hmm. -relvel instead?
	else m_ship->AIMatchAngVelObjSpace(vector3d(0.0));

	if (m_state == 4) return true;
	return false;
}


// m_state values:
// 0: get data for docking start pos
// 1: Fly to docking start pos
// 2: get data for docking end pos
// 3: Fly to docking end pos

bool AICmdDock::TimeStepUpdate()
{
	if (!ProcessChild()) return false;
	if (!m_target) return true;
	if (m_state == 1) m_state = 2;				// finished moving into dock start pos
	if (m_ship->GetFlightState() != Ship::FLYING) {		// todo: should probably launch if docked with something else
		m_ship->ClearThrusterState();
		return true; // docked, hopefully
	}

	// if we're not close to target, do a flyto first
	double targdist = m_target->GetPositionRelTo(m_ship).Length();
	if (targdist > m_target->GetBoundingRadius() * VICINITY_MUL * 1.5) {
		m_child = new AICmdFlyTo(m_ship, m_target);
		ProcessChild(); return false;
	}

	int port = m_target->GetMyDockingPort(m_ship);
	if (port == -1) {
		std::string msg;
		m_target->GetDockingClearance(m_ship, msg);
		port = m_target->GetMyDockingPort(m_ship);
		if (port == -1) { m_ship->AIMessage(Ship::AIERROR_REFUSED_PERM); return true; }
	}

	// state 0,2: Get docking data
	if (m_state == 0 || m_state == 2 || m_state == 4) {
		const SpaceStationType *type = m_target->GetSpaceStationType();
		SpaceStationType::positionOrient_t dockpos;
		type->GetShipApproachWaypoints(port, (m_state==0)?1:2, dockpos);
		matrix4x4d trot; m_target->GetRotMatrix(trot);
		if (m_state != 2) m_dockpos = trot * dockpos.pos + m_target->GetPosition();
		m_dockdir = (trot * dockpos.xaxis.Cross(dockpos.yaxis)).Normalized();
		m_dockupdir = (trot * dockpos.yaxis).Normalized();		// don't trust these enough
		if (type->dockMethod == SpaceStationType::ORBITAL) m_dockupdir = -m_dockupdir;
		m_state++;
	}

	if (m_state == 1) {			// fly to first docking waypoint
		m_child = new AICmdFlyTo(m_ship, m_target->GetFrame(), m_dockpos, 0.0, false);
		ProcessChild(); return false;
	}

	// second docking waypoint
	m_ship->SetWheelState(true);
	vector3d targpos = GetPosInFrame(m_ship->GetFrame(), m_target->GetFrame(), m_dockpos);
	vector3d relpos = targpos - m_ship->GetPosition();
	vector3d reldir = relpos.NormalizedSafe();
	vector3d relvel = m_ship->GetVelocityRelTo(m_target);
	double maxdecel = GetMaxDecel(m_ship, reldir, 0, 0);
	maxdecel -= GetGravityAtPos(m_target->GetFrame(), m_dockpos);
	m_ship->AIMatchPosVel2(reldir, relpos.Length(), relvel, 0.0, maxdecel);

	// massive pile of crap needed to get updir right outside the frame
	Frame *sframe = m_target->GetFrame();
	double ang = sframe->GetAngVelocity().Length() * Pi::game->GetTimeStep();
	matrix4x4d m; Frame::GetFrameTransform(sframe, m_ship->GetFrame(), m);
	if (!float_is_zero_general(ang) && sframe != m_ship->GetFrame()) {
		vector3d axis = sframe->GetAngVelocity().Normalized();
		m = m * matrix4x4d::RotateMatrix(ang, axis.x, axis.y, axis.z);
	}
	vector3d updir = m.ApplyRotationOnly(m_dockupdir);
	bool fin = m_ship->AIFaceOrient(m_dockdir, updir);
	if (m_state < 5 && fin && m_ship->GetWheelState() >= 1.0f) m_state++;

#ifdef DEBUG_AUTOPILOT
printf("AICmdDock dist = %.1f, speed = %.1f, ythrust = %.2f, state = %i\n",
	targdist, relvel.Length(), m_ship->GetThrusterState().y, m_state);
#endif

	return false;
}

bool AICmdHoldPosition::TimeStepUpdate()
{
	// XXX perhaps try harder to move back to the original position
	m_ship->AIMatchVel(vector3d(0,0,0));
	return false;
}


void AICmdFlyAround::Setup(Body *obstructor, double alt, double vel, int targmode, Body *target, Frame *targframe, const vector3d &posoff)
{
	m_obstructor = obstructor; m_alt = alt; m_vel = vel;
	m_targmode = targmode; m_target = target; m_targframe = targframe; m_posoff = posoff;
	
	// generate suitable velocity if none provided
	double minacc = (targmode == 3) ? 0 : m_ship->GetAccelMin();
	double mass = obstructor->IsType(Object::TERRAINBODY) ? obstructor->GetMass() : 0;
	if (vel < 1e-30) m_vel = sqrt(m_alt*0.8*minacc + mass*G/m_alt);

	// check if altitude is within obstructor frame
	if (alt > 0.9 * GetNonRotFrame(obstructor)->GetRadius()) {
		m_ship->AIMessage(Ship::AIERROR_ORBIT_IMPOSSIBLE);
		m_targmode = 1; m_target = 0;			// force an exit
	}
}

AICmdFlyAround::AICmdFlyAround(Ship *ship, Body *obstructor, double relalt)
	: AICommand (ship, CMD_FLYAROUND)
{
	double alt = relalt*MaxEffectRad(obstructor, ship);
	Setup(obstructor, alt, 0.0, 3, 0, 0, vector3d(0.0));
}

AICmdFlyAround::AICmdFlyAround(Ship *ship, Body *obstructor, double alt, double vel)
	: AICommand (ship, CMD_FLYAROUND)
{
	Setup(obstructor, alt, vel, 0, 0, 0, vector3d(0.0));
}

AICmdFlyAround::AICmdFlyAround(Ship *ship, Body *obstructor, double alt, double vel, Body *target, const vector3d &posoff)
	: AICommand (ship, CMD_FLYAROUND)
{
	Setup(obstructor, alt, vel, 1, target, 0, posoff);
}

AICmdFlyAround::AICmdFlyAround(Ship *ship, Body *obstructor, double alt, double vel, Frame *targframe, const vector3d &posoff)
	: AICommand (ship, CMD_FLYAROUND)
{
	Setup(obstructor, alt, vel, 2, 0, targframe, posoff);
}

vector3d AICmdFlyAround::Targpos()
{
	switch (m_targmode) {
		default: return m_ship->GetPosition() + m_ship->GetVelocity();
		case 1: return GetPosInFrame(m_ship->GetFrame(), m_target->GetFrame(), m_target->GetPosition());
		case 2: return GetPosInFrame(m_ship->GetFrame(), m_targframe, m_posoff);
	}
}

double AICmdFlyAround::MaxVel(double targdist, double targalt)
{
	if (targalt > m_alt) return m_vel;
	double t = sqrt(2.0 * targdist / m_ship->GetAccelFwd());
	double vmaxprox = m_ship->GetAccelMin()*t;			// limit by target proximity
	double vmaxstep = std::max(m_alt*0.05, m_alt-targalt);
	vmaxstep /= Pi::game->GetTimeStep();			// limit by distance covered per timestep
	return std::min(m_vel, std::min(vmaxprox, vmaxstep));
}

bool AICmdFlyAround::TimeStepUpdate()
{
	if (m_targmode == 1 && !m_target) return true;
	if (!ProcessChild()) return false;

	// Not necessary unless it's a tier 1 AI
	if (m_ship->GetFlightState() == Ship::FLYING) m_ship->SetWheelState(false);
	else { LaunchShip(m_ship); return false; }

	double timestep = Pi::game->GetTimeStep();
	vector3d targpos = Targpos();		// target position in ship's frame
	vector3d obspos = m_obstructor->GetPositionRelTo(m_ship);
	double obsdist = obspos.Length();
	vector3d obsdir = obspos / obsdist;
	vector3d relpos = targpos - m_ship->GetPosition();

	// if too far away, fly to tangent
	if (obsdist > 1.1*m_alt)
	{
		double v;
		Frame *obsframe = GetNonRotFrame(m_obstructor);
		vector3d tangent = GenerateTangent(m_ship, obsframe, targpos, m_alt);
		vector3d tpos_obs = GetPosInFrame(obsframe, m_ship->GetFrame(), targpos);
		if (m_targmode != 1 && m_targmode != 2) v = m_vel;
		else if (relpos.LengthSqr() < obsdist) v = 0.0;
		else v = MaxVel((tpos_obs-tangent).Length(), tpos_obs.Length());
		m_child = new AICmdFlyTo(m_ship, obsframe, tangent, v, true);
		ProcessChild(); return false;
	}

	// limit m_vel by target proximity & distance covered per frame
	double vel = (m_targmode != 1 && m_targmode != 2) ? m_vel
		: MaxVel(relpos.Length(), targpos.Length());

	// all calculations in ship's frame
	vector3d fwddir = (obsdir.Cross(relpos).Cross(obsdir)).Normalized();
	vector3d tanvel = vel * fwddir;

	// frame body suicide check, response
	if (CheckSuicide(m_ship, -obsdir)) {
		m_ship->AIFaceDirection(m_ship->GetPosition());		// face away from planet
		m_ship->AIMatchVel(vector3d(0.0)); return false;
	}

	// calculate target velocity
	double alt = (tanvel * timestep + obspos).Length();		// unnecessary?
	double ivel = calc_ivel(alt - m_alt, 0.0, m_ship->GetAccelMin());

	vector3d finalvel = tanvel + ivel * obsdir;
	m_ship->AIMatchVel(finalvel);
	m_ship->AIFaceDirection(fwddir);

//	vector3d newhead = GenerateTangent(m_ship, m_obstructor->GetFrame(), fwddir);
//	newhead = GetPosInFrame(m_ship->GetFrame(), m_obstructor->GetFrame(), newhead);
//	m_ship->AIFaceDirection(newhead-m_ship->GetPosition());

	// termination condition for orbits
	vector3d thrust = m_ship->GetThrusterState();
	if (m_targmode >= 3 && thrust.LengthSqr() < 0.01) m_targmode++;
	if (m_targmode == 5) { m_ship->SetThrusterState(vector3d(0.0)); return true; }
	return false;
}


