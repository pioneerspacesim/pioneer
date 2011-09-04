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
		case CMD_KILL: return new AICmdKill(rd);
		case CMD_KAMIKAZE: return new AICmdKamikaze(rd);
		case CMD_HOLDPOSITION: return new AICmdHoldPosition(rd);
	}
}

void AICommand::Save(Serializer::Writer &wr)
{
	wr.Int32(m_cmdName);
	wr.Int32(Serializer::LookupBody(m_ship));
	if (m_child) m_child->Save(wr);
	else wr.Int32(CMD_NONE);
}

AICommand::AICommand(Serializer::Reader &rd, CmdName name)
{
	m_cmdName = name;
	m_shipIndex = rd.Int32();
	m_child = Load(rd);
}

void AICommand::PostLoadFixup()
{
	m_ship = static_cast<Ship *>(Serializer::LookupBody(m_shipIndex));
	if (m_child) m_child->PostLoadFixup();
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

	if (Pi::currentSystem->GetLocation() != (SysLoc)m_dest) {
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
					if ((EquipType::types[t].slot == Equip::SLOT_ENGINE) && trader->CanSell(t)) {
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
	m_timeSinceChange += Pi::GetTimeStep();
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
	vector3d targaccel = (m_target->GetVelocity() - m_lastVel) / Pi::GetTimeStep();
	m_lastVel = m_target->GetVelocity();		// may need next frame
	vector3d leaddir = m_ship->AIGetLeadDir(m_target, targaccel, 0);

	// turn towards target lead direction, add inaccuracy
	// trigger recheck when angular velocity reaches zero or after certain time

	if (m_leadTime < Pi::GetGameTime())
	{
		double skillShoot = 0.5;		// todo: should come from AI stats

		double headdiff = (leaddir - heading).Length();
		double leaddiff = (leaddir - targdir).Length();
		m_leadTime = Pi::GetGameTime() + headdiff + (1.0*Pi::rng.Double()*skillShoot);

		// lead inaccuracy based on diff between heading and leaddir
		vector3d r(Pi::rng.Double()-0.5, Pi::rng.Double()-0.5, Pi::rng.Double()-0.5);
		vector3d newoffset = r * (0.02 + 2.0*leaddiff + 2.0*headdiff)*Pi::rng.Double()*skillShoot;
		m_leadOffset = (heading - leaddir);		// should be already...
		m_leadDrift = (newoffset - m_leadOffset) / (m_leadTime - Pi::GetGameTime());

		// Shoot only when close to target

		double vissize = 1.3 * m_ship->GetBoundingRadius() / targpos.Length();
		vissize += (0.05 + 0.5*leaddiff)*Pi::rng.Double()*skillShoot;
		if (vissize > headdiff) m_ship->SetGunState(0,1);
		else m_ship->SetGunState(0,0);
		if (targpos.LengthSqr() > 4000*4000) m_ship->SetGunState(0,0);		// temp
	}
	m_leadOffset += m_leadDrift * Pi::GetTimeStep();
	double leadAV = (leaddir-targdir).Dot((leaddir-heading).NormalizedSafe());	// leaddir angvel
	m_ship->AIFaceDirection((leaddir + m_leadOffset).Normalized(), leadAV);


	vector3d evadethrust(0,0,0);
	if (m_evadeTime < Pi::GetGameTime())		// evasion time!
	{
		double skillEvade = 0.5;			// todo: should come from AI stats
		m_evadeTime = Pi::GetGameTime() + Pi::rng.Double(3.0,10.0) * skillEvade;
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
	if (m_closeTime < Pi::GetGameTime())
	{
		double skillEvade = 0.5;
		if (heading.Dot(targdir) < 0.7) skillEvade += 0.5;		// not in view

		m_closeTime = Pi::GetGameTime() + skillEvade * Pi::rng.Double(1.0,5.0);
	
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


static double GetGravityAtPos(Ship *ship, Frame *targframe, vector3d &posoff)
{
	Body *body = targframe->GetBodyFor();
	if (!body || body->IsType(Object::SPACESTATION)) return 0;
	double rsqr = posoff.LengthSqr();
	double m1m2 = ship->GetMass() * body->GetMass();
	return G * m1m2 / rsqr;
	// inverse is: sqrt(G * m1m2 / thrust)
}

// gets position of (target + offset in target's frame) in frame
// if object has its own rotational frame, ignores it
static vector3d GetPosInFrame(Frame *frame, Frame *target, vector3d &offset)
{
	matrix4x4d m; Frame::GetFrameTransform(target, frame, m);
	return m * offset;
}

static vector3d GetVelInFrame(Frame *frame, Frame *target, vector3d &offset)
{
	matrix4x4d m; vector3d vel(0.0);
	Frame::GetFrameTransform(target, frame, m);
	if (target != frame) vel = -target->GetStasisVelocityAtPosition(offset);
	return (m.ApplyRotationOnly(vel) + Frame::GetFrameRelativeVelocity(frame, target));
}

// targpos in frame of obj1
static bool CheckCollision(Ship *obj1, Body *obj2, vector3d &targpos)
{
	vector3d p = obj2->GetPositionRelTo(obj1->GetFrame());
	vector3d p1 = obj1->GetPosition() - p;
	vector3d p2 = targpos - p;
	vector3d p1n = p1.Normalized();
	vector3d p2p1dir = (p2-p1).Normalized();
	double r = obj1->GetBoundingRadius() + obj2->GetBoundingRadius();
	if (p2.LengthSqr() > r * 1.08) r *= 1.05;			// take wider line if target outside
	
	// ignore if targpos is closer than body surface
	if ((p2-p1).Length() < p1.Length() - r) return false;

	// check if direct escape is safe (30 degree limit)
	if (p2p1dir.Dot(p1n) > 0.5) return false;

	// add velocity / distance modifier to radius if body is ahead
	if (p2p1dir.Dot(p1n) < -0.5) {
		vector3d v1 = obj1->GetVelocityRelTo(obj2);
		double acc = obj1->GetMaxThrust(vector3d(-1.0)).z / obj1->GetMass();
		double perpvel = (v1 - p1n * v1.Dot(p1n)).Length();
		double time = sqrt(2 * p1.Length() / acc);
		r += perpvel * 2 * time;
	}

	if (p1.LengthSqr() < r*r) return true;
	if (p2.LengthSqr() < r*r) return true;		// either point actually within radius
	double tanlen = -p1.Dot(p2p1dir);
	if (tanlen < 0 || tanlen > (p2-p1).Length()) return false;	// closest point outside path 
	vector3d tan = p1 + tanlen * p2p1dir;
	return (tan.LengthSqr() < r*r) ? true : false;		// closest point within radius?
}

// used to detect whether it's safe to head into atmosphere to reach target
// breaks for ship very close to offset, should use terrain height value really
static bool TargetWellTest(Ship *ship, Frame *targframe, vector3d &offset)
{
	vector3d ourdir = (ship->GetPositionRelTo(targframe) - offset).NormalizedSafe();
	vector3d targupdir = offset.NormalizedSafe();
	return (ourdir.Dot(targupdir) > 0.5) ? true : false; // 30 degree escape
}

// generates from (0,0,0) to spos, in plane of target 
// formula uses similar triangles
// shiptarg in ship's frame
// output in targframe
static vector3d GenerateTangent(Ship *ship, Frame *targframe, vector3d &shiptarg)
{
	matrix4x4d tran;
	Frame::GetFrameTransform(ship->GetFrame(), targframe, tran);
	vector3d spos = tran * ship->GetPosition();
	vector3d targ = tran * shiptarg;
	double a = spos.Length();
	double b = targframe->GetBodyFor()->GetBoundingRadius();
	if (b*1.02 > a) { spos *= b*1.02/a; a = b*1.02; }		// fudge if ship gets under radius
	double c = sqrt(a*a - b*b);
	return spos.Normalized()*b*b/a + spos.Cross(targ).Cross(spos).Normalized()*b*c/a;
}

// obj1 is ship, obj2 is target body, targpos is destination in obj1's frame
static Body *FindNearestObstructor(Ship *obj1, Body *obj2, vector3d &targpos)
{
	if (!obj2) return 0;
	Body *body = obj2->GetFrame()->GetBodyFor();
	if (body && CheckCollision(obj1, body, targpos)) return body;
	Frame *parent = obj2->GetFrame()->m_parent;
	if (!parent || !parent->m_parent) return 0;
	return FindNearestObstructor(obj1, parent->m_parent->GetBodyFor(), targpos);
}

static int GetFlipMode(Ship *ship, Frame *targframe, vector3d &posoff)
{
	vector3d targpos = GetPosInFrame(ship->GetFrame(), targframe, posoff);
	targpos -= ship->GetPosition();
	return (targpos.Length() > 100000000.0) ? 1 : 0;		// arbitrary
}

// needs work for high-gravity bodies

void AICmdFlyTo::NavigateAroundBody(Body *body, vector3d &targpos)
{
//if(m_ship->IsType(Object::PLAYER)) {
//	printf("Flying to tangent of body: %s\n", body->GetLabel().c_str());
//}
	// build tangent vector in body's rotating frame unless space station (or distant)
	Frame *targframe = body->GetFrame();
	double tanmul = 1.02;
	if (m_targframe->GetBodyFor() != body) { tanmul = 1.2; targframe = targframe->m_parent; }
	else if (body->IsType(Object::SPACESTATION)) targframe = targframe->m_parent;
	if (!targframe) targframe = body->GetFrame();

	// offset tangent by a bit
	vector3d newpos = tanmul * GenerateTangent(m_ship, targframe, targpos);
	vector3d curpos = m_ship->GetPositionRelTo(targframe);
	vector3d targpos2 = GetPosInFrame(targframe, m_ship->GetFrame(), targpos);
	newpos = 0.5 * (newpos + curpos);

	// terminal velocity based on centripetal force:
	// v = sqrt(rad * force / mass)
	double sideacc = m_ship->GetMaxThrust(vector3d(0.0)).x / m_ship->GetMass();
	sideacc += GetGravityAtPos(m_ship, targframe, newpos) / m_ship->GetMass();
	double endvel = sqrt(newpos.Length() * 0.25*sideacc);
	// limit by targdir - not sure this one is needed with the heading limit
	double maxendvel = sqrt(0.5*sideacc*(targpos2-newpos).Length());
	if (maxendvel < endvel) endvel = maxendvel;
	// limit by step length vs timestep too. possibly not needed except beyond 10000x
	if (Pi::GetTimeStep() > sqrt((newpos-curpos).Length() / (2*sideacc))) endvel = 0;

	// ignore further collisions
	int newmode = GetFlipMode(m_ship, targframe, newpos) ? 1 : 3;
	m_child = new AICmdFlyTo(m_ship, targframe, newpos, endvel, newmode, false);
	static_cast<AICmdFlyTo *>(m_child)->SetOrigTarg(m_targframe, m_posoff);			// needed for tangent heading
	m_frame = 0;		// trigger rebuild when child finishes
}


void AICmdFlyTo::CheckCollisions()
{
	// first get target pos in our frame
	m_frame = m_ship->GetFrame();				// set up the termination test
	vector3d targpos = GetPosInFrame(m_frame, m_targframe, m_posoff);
	m_reldir = (targpos - m_ship->GetPosition()).NormalizedSafe();

	// check collisions
	if (!m_coll) return;
	Body *body = FindNearestObstructor(m_ship, m_frame->GetBodyFor(), targpos);
	if (!body) body = FindNearestObstructor(m_ship, m_targframe->GetBodyFor(), targpos);
	if (!body) return;
//	if (m_orbitrad > 0 && body == m_targframe->GetBodyFor()) return;		// don't bother checking orbit target
	double dist = m_ship->GetPositionRelTo(body).Length();
	double rad = body->GetBoundingRadius();

	// Check whether target is relative to obstructor and nearby
	if (rad * 2 > dist && body->GetFrame() == m_targframe &&
		TargetWellTest(m_ship, m_targframe, m_posoff)) return;

	// Else if closer, fly to sort-of tangent
	if (rad*VICINITY_MUL*1.5 > dist)
		NavigateAroundBody(body, targpos);
	
	// Otherwise just head to vicinity of obstructor unless we were already
	else if (body != m_targframe->GetBodyFor() || m_posoff.Length() < rad*VICINITY_MUL*0.5) {
		m_frame = 0;		// trigger recheck when done
		m_child = new AICmdFlyTo(m_ship, body);
	}
}


// Check for the player doing bad things
void AICmdFlyTo::CheckSuicide()
{
	if (!m_frame || m_state == 4 || m_state == 5) return;			// always called after CheckCollision
	vector3d relvel = m_ship->GetVelocity() - GetVelInFrame(m_frame, m_targframe, m_posoff);
	vector3d relpos = GetPosInFrame(m_frame, m_targframe, m_posoff) - m_ship->GetPosition();
	vector3d reldir = relpos.NormalizedSafe();
	double targdist = relpos.Length();
	vector3d maxacc = m_ship->GetMaxThrust(vector3d(1,1,-1)) / m_ship->GetMass();
	vector3d plandir = m_ship->GetPosition().Normalized();	

	// 1. on collision course with frame body
	// how to detect? altitude, sideacc & velocity towards planet
	SBody *sbody = m_frame->GetSBodyFor();
	if (sbody && m_frame != m_targframe) {
		// fuck knows what sbody->GetRadius is
		double alt = m_ship->GetPosition().Length() - sbody->GetRadius();
		double dirvel = plandir.Dot(m_ship->GetVelocity());

		if (dirvel < 0 && dirvel*dirvel > 2*maxacc.x*alt) {
			printf("Planet suicide dive detected\n");
			m_child = new AICmdFlyTo(m_ship, m_targframe, m_posoff, m_endvel, 5, false);
			m_frame = 0; return;
		}
	}

	// 2. too much sidevel to reach target
	// only slightly fake minimum time to target
	// based on s = (sv+ev)/2 + a*t*t/4
	double u = 0.5 * (relvel.Dot(reldir) + m_endvel);
	if (m_state != 1) maxacc.z *= 0.66;
	double t = (-u + sqrt(u*u + maxacc.z*targdist)) / (maxacc.z * 0.5);
	assert(t>0);

	// check for uncorrectable side velocity
	if (m_state == 3) maxacc.x *= 2;
	vector3d perpvel = relvel - reldir * relvel.Dot(reldir);
	if (perpvel.Length() > maxacc.x*t) {
		printf("Uncorrectable sidevel detected\n");
		m_child = new AICmdFlyTo(m_ship, m_targframe, m_posoff, m_endvel, 4, false);
		m_frame = 0; return;
	}
}

// Fly to "vicinity" of body
AICmdFlyTo::AICmdFlyTo(Ship *ship, Body *target) : AICommand (ship, CMD_FLYTO)
{
	double dist = std::max(VICINITY_MIN, VICINITY_MUL*target->GetBoundingRadius());
	if (target->IsType(Object::SPACESTATION) && static_cast<SpaceStation *>(target)->IsGroundStation()) {
		matrix4x4d rot; target->GetRotMatrix(rot);
		m_posoff = target->GetPosition() + dist * vector3d(rot[4], rot[5], rot[6]);		// up vector for starport
		m_targframe = target->GetFrame();
	}
	else {
		if(target->HasDoubleFrame()) m_targframe = target->GetFrame()->m_parent;
		else m_targframe = target->GetFrame();
		m_posoff = dist * m_ship->GetPositionRelTo(m_targframe).Normalized();
		m_posoff += target->GetPosition();
	}

	m_endvel = 0;
	m_orbitrad = 0;
	m_state = GetFlipMode(m_ship, m_targframe, m_posoff);
	m_coll = true;

	// check if we're already close enough
	if (dist > m_ship->GetPositionRelTo(target).Length()) m_state = 6;
	else {
		CheckCollisions();
		if (!m_child) CheckSuicide();
	}
}

// Orbit
AICmdFlyTo::AICmdFlyTo(Ship *ship, Body *target, double alt) : AICommand (ship, CMD_FLYTO)
{
	if(target->HasDoubleFrame()) m_targframe = target->GetFrame()->m_parent;
	else m_targframe = target->GetFrame();				// use non-rot frame
	m_orbitrad = target->GetBoundingRadius() * alt;
	m_endvel = sqrt(target->GetMass() * G / m_orbitrad);

	matrix4x4d rot; m_ship->GetRotMatrix(rot);
	vector3d heading(-rot[8], -rot[9], -rot[10]);
	m_posoff = GenerateTangent(m_ship, m_targframe, heading) * alt;
	m_state = GetFlipMode(m_ship, m_targframe, m_posoff);
	m_coll = true;

	CheckCollisions();
	if (!m_child) CheckSuicide();
}

// Specified pos, endvel should be > 0
AICmdFlyTo::AICmdFlyTo(Ship *ship, Frame *targframe, vector3d &posoff, double endvel, int headmode, bool coll)
	: AICommand (ship, CMD_FLYTO)
{
	m_targframe = targframe;
	m_posoff = posoff;
	m_endvel = endvel;
	m_orbitrad = 0;
	m_state = headmode;
	m_coll = coll;

	CheckCollisions();
	if (!m_child) CheckSuicide();
}


// shift this functionality to ship class later, maybe
// limits z thrusters to the minimum to make heading non-critical
// issue because a thruster might be used for countering external forces?
// hopefully only called in initial stage - issue with fast-rotating planets?
// only clamp main then?
static void ClampMainThruster(Ship *ship)
{
	vector3d tstate = ship->GetThrusterState();
	vector3d maxthrust = ship->GetMaxThrust(tstate);
	double clampz = maxthrust.x / maxthrust.z;
	tstate.z = Clamp(tstate.z, -clampz, clampz);
	ship->SetThrusterState(tstate);
}

// terminal orbit function: force into proper orbit at current alt
bool AICmdFlyTo::OrbitCorrection()
{
	if (!m_targframe->GetBodyFor()) return true;	// tried to orbit something that isn't a planet or star
	if ( m_ship->GetFrame() != m_targframe) return true;		// no gravity outside frame anyway
	vector3d pos = m_ship->GetPosition();
	vector3d vel = m_ship->GetVelocity();

	double orbspd = sqrt(m_targframe->GetBodyFor()->GetMass() * G / pos.Length());
	vector3d targvel = orbspd * pos.Cross(vel).Cross(pos).Normalized();
	m_ship->AIFaceDirection(targvel);		// face towards target at end maybe?
	if (m_ship->AIMatchVel(targvel)) return true;
	return false;				// applied thrust this frame, wait until next to bail
}


// m_state values:
// 0, head towards
// 1, head towards unless flip conditions pass, flip++
// 2, head away
// 3, head towards tangent in direction of target
// 4, head in opposite direction to velocity, kill velocity
// 5, head away from planet, kill velocity
// 6, reserved
// 7, don't change heading
// 8, don't change heading, one timestep before termination
// 9, applying final velocity cancellation hopefully 
// 10, terminal orbital adjustment mode
// 11, started within vicinity, terminate immediately

bool AICmdFlyTo::TimeStepUpdate()
{
	if (!ProcessChild()) return false;		// child not finished
	if (m_state == 11) return true;			// started within range

	if (m_ship->GetFlightState() == Ship::FLYING) m_ship->SetWheelState(false);
	else { LaunchShip(m_ship); return false; }

	if (m_state == 10) return OrbitCorrection();		// terminal orbit mode

	if (m_frame != m_ship->GetFrame()) {				// frame switch check
		if (m_state >= 3 && m_state <= 5) return true;			// bailout case for accidental planet-dives
		if (!m_frame) m_state = GetFlipMode(m_ship, m_targframe, m_posoff);
		CheckCollisions();
		if (!m_child) CheckSuicide();
		if (m_child) { ProcessChild(); return false; }			// child can handle at least one timestep
	}

	double timestep = Pi::GetTimeStep();
	vector3d targvel = GetVelInFrame(m_frame, m_targframe, m_posoff);
	vector3d relvel = m_ship->GetVelocity() - targvel;
	vector3d targpos = GetPosInFrame(m_frame, m_targframe, m_posoff) - targvel * timestep;
	vector3d relpos = targpos - m_ship->GetPosition();
	vector3d reldir = relpos.NormalizedSafe();
	double targdist = relpos.Length();
	double sideacc = m_ship->GetMaxThrust(vector3d(0.0)).x / m_ship->GetMass();

//if(m_ship->IsType(Object::PLAYER)) {
//	printf("Autopilot dist = %f, speed = %f, term = %f, state = 0x%x\n", targdist, relvel.Length(),
//		reldir.Dot(m_reldir), m_state);
//}

	// planet evasion case
	if (m_state == 4 || m_state == 5) {
		if (m_state == 4) m_ship->AIFaceDirection(-relvel);
		else m_ship->AIFaceDirection(m_ship->GetPosition());		// face away from planet
		if (m_ship->AIMatchVel(vector3d(0.0))) return true;
		return false;
	}

	// termination conditions
	if (m_state == 8) m_state = 9;					// finished last adjustment, hopefully
	else if (m_endvel <= 0) { if (targdist < 0.5*sideacc*timestep*timestep) m_state = 8; }
	else if (reldir.Dot(m_reldir) < 0.9) m_state = (m_orbitrad > 0) ? 10 : 9;

	// linear thrust
	vector3d maxthrust = m_ship->GetMaxThrust(vector3d(1,1,(m_state==1)?-1:1));
	maxthrust.z -= GetGravityAtPos(m_ship, m_targframe, m_posoff);
	if(maxthrust.z <= 0) { m_ship->AIMessage(Ship::GRAV_TOO_HIGH); return true; }
	double decel = m_ship->AIMatchPosVel(relpos, relvel, m_endvel, maxthrust);
	if (m_state == 1 && decel < 0) m_state = 2;		// time to flip
	
	// set heading according to current state
	double ang = 0.0;
	if (m_state < 2) {
		vector3d nextpos = m_ship->AIGetNextFramePos();			// position next frame before atmos/grav
		if ((targpos-nextpos).Dot(relpos) <= 0.0) nextpos = m_ship->GetPosition();	// last frame turning workaround
		ang = m_ship->AIFaceDirection(targpos-nextpos);
	}
	else if (m_state == 2) ang = m_ship->AIFaceDirection(-reldir);
	else if (m_state == 3) {
		vector3d origtarg = GetPosInFrame(m_frame, m_origframe, m_origpos);
		vector3d newhead = GenerateTangent(m_ship, m_targframe, origtarg);
		newhead = GetPosInFrame(m_frame, m_targframe, newhead);
		ang = m_ship->AIFaceDirection(newhead-m_ship->GetPosition());
	}
	else m_ship->AIMatchAngVelObjSpace(vector3d(0.0));

	// limit forward acceleration when facing wrong way
	if (decel > 0 && fabs(ang) > 0.02) ClampMainThruster(m_ship);

	if (m_state == 9) return true;
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
		return TimeStepUpdate();			// should be safe...
	}

	int port = m_target->GetMyDockingPort(m_ship);
	if (port == -1) {
		std::string msg;
		m_target->GetDockingClearance(m_ship, msg);
		port = m_target->GetMyDockingPort(m_ship);
		if (port == -1) { m_ship->AIMessage(Ship::REFUSED_PERM); return true; }
	}

	// state 0,2: Get docking data
	if (m_state == 0 || m_state == 2) {
		const SpaceStationType *type = m_target->GetSpaceStationType();
		SpaceStationType::positionOrient_t dockpos;
		bool good;
        good = type->GetShipApproachWaypoints(port, (m_state>>1)+1, dockpos);
		matrix4x4d trot; m_target->GetRotMatrix(trot);
		m_dockpos = trot * dockpos.pos + m_target->GetPosition();
		m_dockdir = (trot * dockpos.xaxis.Cross(dockpos.yaxis)).Normalized();
		m_dockupdir = (trot * dockpos.yaxis).Normalized();		// don't trust these enough
		if (type->dockMethod == SpaceStationType::ORBITAL) m_dockupdir = -m_dockupdir;
		m_state++;
	}

	if (m_state == 1) {			// fly to first docking waypoint
		m_child = new AICmdFlyTo(m_ship, m_target->GetFrame(), m_dockpos, 0.0, 0, false);
		return TimeStepUpdate();
	}
	// second docking waypoint
	m_ship->SetWheelState(true);
	if (m_ship->AIFaceOrient(m_dockdir, m_dockupdir) < 0.01 && m_state++ > 3) {		// second docking waypoint
		vector3d targpos = GetPosInFrame(m_ship->GetFrame(), m_target->GetFrame(), m_dockpos);
		vector3d relpos = targpos - m_ship->GetPosition();
		vector3d relvel = m_ship->GetVelocityRelTo(m_target);
		vector3d maxthrust = m_ship->GetMaxThrust(vector3d(1,1,1));
		maxthrust.y -= GetGravityAtPos(m_ship, m_target->GetFrame(), m_dockpos);
		m_ship->AIMatchPosVel(relpos, relvel, 0.0, maxthrust);
	}
	else m_ship->AIMatchVel(vector3d(0.0));
	return false;
}

bool AICmdHoldPosition::TimeStepUpdate()
{
	// XXX perhaps try harder to move back to the original position
	m_ship->AIMatchVel(vector3d(0,0,0));
	return false;
}
