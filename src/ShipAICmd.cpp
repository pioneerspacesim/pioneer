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


AICommand *AICommand::Load(Serializer::Reader &rd)
{
	CmdName name = (CmdName)rd.Int32();
	switch (name) {
		case CMD_NONE: default: return 0;
//		case CMD_JOURNEY: return new AICmdJourney(rd);
//		case CMD_DOCK: return new AICmdDock(rd);
//		case CMD_ORBIT: return new AICmdOrbit(rd);
		case CMD_FLYTO: return new AICmdFlyTo(rd);
		case CMD_KILL: return new AICmdKill(rd);
		case CMD_KAMIKAZE: return new AICmdKamikaze(rd);
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
	m_ship = (Ship*)rd.Int32();
	m_child = Load(rd);
}

void AICommand::PostLoadFixup()
{
	m_ship = (Ship *)Serializer::LookupBody((size_t)m_ship);
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
bool AICmdDock::TimeStepUpdate()
{
	if (!ProcessChild()) return false;

	// Ship::DOCKING for example...
	if (m_ship->GetFlightState() != Ship::FLYING) return true;

	AIPath newPath;
	if (m_ship->AIAddAvoidancePathOnWayTo(m_target, newPath)) {
		m_child = new AICmdFlyTo(m_ship, m_target, newPath);
		m_path.endTime = 0;
		return false;
	}
	else if (m_target->GetPositionRelTo(m_ship).Length() > 100000.0) {
		m_child = new AICmdFlyTo(m_ship, m_target);
		m_path.endTime = 0;
		return false;
	}

	int port = m_target->GetMyDockingPort(m_ship);
	if (port == -1) {
		std::string msg;
		m_target->GetDockingClearance(m_ship, msg);
		port = m_target->GetMyDockingPort(m_ship);
		if (port == -1) {
			// todo: should give a failure message, that we couldn't
			// get docking
			return true;
		}
	}

	if (m_path.endTime == 0) {
		Frame *frame = m_target->GetFrame();
		if (frame->IsRotatingFrame() && !m_target->IsGroundStation()) {
			// can't autopilot by the rotating frame of a space station
			// (it rotates too fast and it would be incorrect anyway)
			frame = frame->m_parent;
		}
		
		const double maxAccel = m_ship->GetWeakestThrustersForce() / m_ship->GetMass();
		vector3d pos[3];
		pos[0] = m_ship->GetPositionRelTo(frame);
		const vector3d ourVelocity = m_ship->GetVelocityRelativeTo(frame);
		const vector3d endVelocity = vector3d(0.0,-25.0,0.0);

		SpaceStationType::positionOrient_t shipOrientStartDocking, shipOrientEndDocking;
		bool good = m_target->GetSpaceStationType()->GetShipApproachWaypoints(port, 1, shipOrientStartDocking);
		good = good && m_target->GetSpaceStationType()->GetShipApproachWaypoints(port, 2, shipOrientEndDocking);

		assert(good);
		double duration;

		matrix4x4d target_rot;
		m_target->GetRotMatrix(target_rot);
		pos[1] = m_target->GetPosition() + target_rot * shipOrientStartDocking.pos;
		pos[2] = m_target->GetPosition() + target_rot * shipOrientEndDocking.pos;
	
		path(3, pos, ourVelocity, endVelocity, maxAccel*.5, duration, m_path.path);
		m_path.startTime = Pi::GetGameTime();
		m_path.endTime = m_path.startTime + duration;
		m_path.frame = frame;
	}
	if (m_path.endTime > 0) {
		bool done = m_ship->AIFollowPath(m_path, true);
		
		if (m_path.endTime - Pi::GetGameTime() < 30.0) {
			// orient the ship in the last 30 seconds
			// todo: should maybe cache this final orientation to avoid
			// the lua calls
			SpaceStationType::positionOrient_t shipOrientEndDocking;
			m_target->GetSpaceStationType()->GetShipApproachWaypoints(port, 2, shipOrientEndDocking);

			matrix4x4d orient;
			m_target->GetRotMatrix(orient);
			orient = orient * matrix4x4d::MakeRotMatrix(shipOrientEndDocking.xaxis, shipOrientEndDocking.yaxis,
					shipOrientEndDocking.xaxis.Cross(shipOrientEndDocking.yaxis));
			m_ship->SetAngThrusterState(0, 0);
			m_ship->SetAngThrusterState(1, 0);
			m_ship->SetAngThrusterState(2, 0);
			m_ship->AISlowOrient(orient);
	
			if (m_ship->GetWheelState() == 0) m_ship->SetWheelState(true);
		}
		return done;
	} else {
		return true;
	}
}


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

bool AICmdKamikaze::TimeStepUpdate()
{
	m_ship->SetGunState(0,0);
	// needs to deal with frames, large distances, and success
	if (m_ship->GetFrame() == m_target->GetFrame()) {
		double dist = (m_target->GetPosition() - m_ship->GetPosition()).Length();
		vector3d vRel = m_ship->GetVelocityRelativeTo(m_target);
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


bool AICmdKill::TimeStepUpdate()
{
return false;

	if (!m_target) return true;

	matrix4x4d rot; m_ship->GetRotMatrix(rot);				// some world-space params
	const ShipType &stype = m_ship->GetShipType();
	vector3d targpos = m_target->GetPositionRelTo(m_ship);
	vector3d targvel = m_target->GetVelocityRelativeTo(m_ship);		
	vector3d targdir = targpos.Normalized();
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
	}
	m_leadOffset += m_leadDrift * Pi::GetTimeStep();
	double leadAV = (leaddir-targdir).Dot((leaddir-heading).Normalized());	// leaddir angvel
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

		if (skillEvade < 1.0 && targhead.z < 0.0) {		// smart chase
			vector3d objvel = targvel * rot;		// obj space targvel
			evadethrust.x = objvel.x > 0.0 ? 1.0 : -1.0;
			evadethrust.y = objvel.y > 0.0 ? 1.0 : -1.0;
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
		// v = sqrt(2as), positive => towards
		if (dist > reqdist) ispeed = sqrt(2.0 * rearaccel * (dist - reqdist));
		else ispeed = -sqrt(2.0 * rearaccel * (reqdist - dist));
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


// gets position of (target + offset in target's frame) in frame
// if object has its own rotational frame, ignores it
static vector3d GetPosInFrame(Frame *frame, Body *target, vector3d &offset)
{
	matrix4x4d trot, tran; // todo: add a thing to body? or frame?
	if (target->HasDoubleFrame())				// planet or space station
		Frame::GetFrameTransform(target->GetFrame()->m_parent, frame, tran);
	else Frame::GetFrameTransform(target->GetFrame(), frame, tran);
	return tran * (offset + target->GetPosition());
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
	
	// ignore if targpos is closer than body surface
	if ((p2-p1).Length() < p1.Length() - r) return false;

	// check if direct escape is safe (30 degree limit)
	if (p2p1dir.Dot(p1n) > 0.5) return false;

	// add velocity / distance modifier to radius if body is ahead
	if (p2p1dir.Dot(p1n) < -0.5) {
		vector3d v1 = obj1->GetVelocityRelativeTo(obj2);
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
static bool TargetWellTest(Ship *obj1, Body *obj2)
{
	if (!obj2->IsType(Object::SPACESTATION)) return false;
	if (!((SpaceStation *)obj2)->IsGroundStation()) return false;

	vector3d ourdir = obj1->GetPositionRelTo(obj2).NormalizedSafe();
	vector3d targupdir = obj2->GetPosition().NormalizedSafe();
	return (ourdir.Dot(targupdir) > 0.5) ? true : false;		// 30 degree escape
}

// generates from (0,0,0) to spos, in plane of target 
// formula uses similar triangles
// shiptarg in ship's frame
// output in body's non-rotating frame
static vector3d GenerateTangent(Ship *ship, Body *body, vector3d &shiptarg)
{
	matrix4x4d tran;
	if (body->HasDoubleFrame())				// planet or space station
		Frame::GetFrameTransform(ship->GetFrame(), body->GetFrame()->m_parent, tran);
	else Frame::GetFrameTransform(ship->GetFrame(), body->GetFrame(), tran);
	
	vector3d spos = tran * ship->GetPosition();
	vector3d targ = tran * shiptarg;
	double a = spos.Length(), b = body->GetBoundingRadius();
	while (b > a)
		a *= 1.02;	// fudge if ship gets under radius
	double c = sqrt(a*a - b*b);
	return spos.Normalized()*b*b/a + spos.Cross(targ).Cross(spos).Normalized()*b*c/a;
}

// obj1 is ship, obj2 is target body, targpos is destination in obj1's frame
static Body *FindNearestObstructor(Ship *obj1, Body *obj2, vector3d &targpos)
{
	Body *body = obj2->GetFrame()->GetBodyFor();
	if (body && CheckCollision(obj1, body, targpos)) return body;
	Frame *parent = obj2->GetFrame()->m_parent;
	if (!parent || !parent->m_parent) return 0;
	return FindNearestObstructor(obj1, parent->m_parent->GetBodyFor(), targpos);
}

static int GetFlipMode(Ship *ship, Body *target, vector3d &posoff)
{
	vector3d targpos = GetPosInFrame(ship->GetFrame(), target, posoff);
	return (targpos.Length() > 100000000.0) ? 1 : 0;		// arbitrary
}

void AICmdFlyTo::GetAwayFromBody(Body *body, vector3d &targpos)
{
	if (TargetWellTest(m_ship, m_target)) return;

	printf("Flying away from body: %s\n", body->GetLabel().c_str());
	fflush(stdout);

	// build escape vector	
	vector3d pos = m_ship->GetPosition();
	vector3d targdir = (targpos-pos).Normalized();
	vector3d ourdir = pos.Normalized();
	vector3d sidedir = ourdir.Cross(targdir).Cross(ourdir).Normalized();
	vector3d newdir = ourdir + 1.74 * sidedir;				// 30 degree escape angle
	newdir = pos + newdir * (body->GetBoundingRadius()*1.02 - pos.Length());

	// offsets to rotating objects are relative to the non-rotating frame
	if (body->HasDoubleFrame()) {				// needed for stars
		matrix4x4d tran;
		Frame::GetFrameTransform(body->GetFrame(), body->GetFrame()->m_parent, tran);
		newdir = tran * newdir;
	}

	// todo: terminal velocity

	// ignore further collisions, forward only
	m_child = new AICmdFlyTo(m_ship, body, newdir, 0.0, 0, false);
	m_frame = 0;		// trigger rebuild when child finishes
}

int g_navbodycount = 0;

void AICmdFlyTo::NavigateAroundBody(Body *body, vector3d &targpos)
{
	if (TargetWellTest(m_ship, m_target)) return;

	printf("Flying to tangent of body: %s\n", body->GetLabel().c_str());
	fflush(stdout);

	// offset tangent by a bit
	vector3d newpos = 1.02 * GenerateTangent(m_ship, body, targpos);

	// should already be a position in non-rotating frame
	// todo: terminal velocity

	// ignore further collisions
	int newmode = GetFlipMode(m_ship, body, newpos) ? 1 : 3;
	m_child = new AICmdFlyTo(m_ship, body, newpos, 0.0, newmode, false);
	m_frame = 0;		// trigger rebuild when child finishes
}


void AICmdFlyTo::CheckCollisions()
{
	// first get target pos in our frame
	vector3d targpos = GetPosInFrame(m_ship->GetFrame(), m_target, m_posoff);
	m_relpos = targpos - m_ship->GetPosition();
	m_frame = m_ship->GetFrame();				// set up the termination test

	// check collisions
	if (!m_coll) return;
	Body *body = FindNearestObstructor(m_ship, m_ship->GetFrame()->GetBodyFor(), targpos);
	if (!body) body = FindNearestObstructor(m_ship, m_target, targpos);
	if (!body) return;

	// todo: check bounding radius vs rotating frame
	// if inside bounding radius of body, head out unless target is in same well
	if (body->GetBoundingRadius() > m_ship->GetPositionRelTo(body).Length())
		GetAwayFromBody(body, targpos);
			
	// Else fly to tangent (todo: or well if visible? hopefully don't need with 30 degree escape)
	else if (body->GetBoundingRadius() * 8 > m_ship->GetPositionRelTo(body).Length())
		NavigateAroundBody(body, targpos);
	
	// if far enough away, just head to vicinity of obstructor unless we were already
	else if (body != m_target || m_posoff.Length() < body->GetBoundingRadius() * 2)
		m_child = new AICmdFlyTo(m_ship, body);
}

// Fly to "vicinity" of body
AICmdFlyTo::AICmdFlyTo(Ship *ship, Body *target) : AICommand (ship, CMD_FLYTO)
{
	m_target = target;

	double dist = std::max(500.0, 4.0 * m_target->GetBoundingRadius());
	if (m_target->IsType(Object::SPACESTATION) && ((SpaceStation *)m_target)->IsGroundStation()) {
		matrix4x4d rot; m_target->GetRotMatrix(rot);
		m_posoff = dist * vector3d(rot[4], rot[5], rot[6]);		// up vector for starport
	}
	else if(m_target->HasDoubleFrame()) 		// get offset to non-rot frame
		m_posoff = dist * m_ship->GetPositionRelTo(m_target->GetFrame()->m_parent).Normalized();
	else m_posoff = dist * m_ship->GetPositionRelTo(m_target).Normalized();

	m_endvel = 0;
	m_orbitrad = 0;
	m_frame = m_ship->GetFrame();
	m_state = GetFlipMode(m_ship, m_target, m_posoff) | 0x10;
	m_coll = true;

	// check if we're already close enough
	if (dist > m_ship->GetPositionRelTo(m_target).Length()) m_state = 6 | 0x10;
	else CheckCollisions();
}

// Orbit
AICmdFlyTo::AICmdFlyTo(Ship *ship, Body *target, double alt) : AICommand (ship, CMD_FLYTO)
{
	m_target = target;
	m_orbitrad = target->GetSBody()->GetRadius() * alt;
	m_endvel = sqrt(target->GetMass() * G / m_orbitrad);

	matrix4x4d rot; m_ship->GetRotMatrix(rot);
	vector3d heading(-rot[8], -rot[9], -rot[10]);
	m_posoff = GenerateTangent(m_ship, m_target, heading);
	m_posoff *= m_orbitrad / m_target->GetBoundingRadius();
	m_frame = m_ship->GetFrame();
	m_state = GetFlipMode(m_ship, m_target, m_posoff) | 0x10;
	m_coll = true;

	CheckCollisions();
}

// Specified pos, endvel should be > 0
AICmdFlyTo::AICmdFlyTo(Ship *ship, Body *target, vector3d &posoff, double endvel, int headmode, bool coll)
	: AICommand (ship, CMD_FLYTO)
{
	m_target = target;
	m_posoff = posoff;
	m_frame = m_ship->GetFrame();
	m_endvel = endvel;
	m_orbitrad = 0;
	m_state = headmode | 0x10;
	m_coll = coll;

	CheckCollisions();
}


// m_state values:
// 0, head towards
// 1, head towards unless flip conditions pass, flip++
// 2, head away
// 3, head towards tangent in direction of target
// 4, don't change heading, one timestep before termination
// 5, applying final velocity cancellation hopefully 
// 10, terminal orbital adjustment mode
// 0x10, flag: heading is far off, 

bool AICmdFlyTo::TimeStepUpdate()
{
	if (!ProcessChild()) return false;		// child not finished
	if (!m_target) return true;
	if (m_frame != m_ship->GetFrame()) {
		CheckCollisions();
		return TimeStepUpdate();		// recurse, no reason that it shouldn't work second time...
	}

	vector3d targpos = GetPosInFrame(m_ship->GetFrame(), m_target, m_posoff);
	vector3d relpos = targpos - m_ship->GetPosition();
	vector3d reldir = relpos.NormalizedSafe();
	vector3d relvel = m_ship->GetVelocityRelativeTo(m_target);
	double targdist = relpos.Length(), timestep = Pi::GetTimeStep(), ang;
	double sideacc = m_ship->GetMaxThrust(vector3d(0.0)).x / m_ship->GetMass();

	// terminal orbit mode: force into proper orbit at current alt
	if (m_state == 10) {
		vector3d pos = m_ship->GetPositionRelTo(m_target);
		double orbspd = sqrt(m_target->GetMass() * G / pos.Length());
		vector3d targvel = orbspd * pos.Cross(relvel).Cross(pos).Normalized();
		m_ship->AIMatchAngVelObjSpace(vector3d(0.0));		// face towards target at end maybe?
		if (m_ship->AIMatchVel(targvel)) return true;
		return false;				// applied thrust this frame, wait until next to bail
	}

	// termination conditions
	if (m_state == 4) m_state = 5;					// finished last adjustment, hopefully
	else if (m_endvel <= 0) if (targdist < 0.5*sideacc*timestep*timestep) m_state = 4;
	else if (relpos.Dot(m_relpos) <= 0) m_state = (m_orbitrad > 0) ? 10 : 5;

	// check for uncorrectable side velocity
	vector3d perpvel = relvel - reldir * relvel.Dot(reldir);
	if (m_state < 4 && perpvel.LengthSqr() > 2.0 * sideacc * targdist) {
		m_ship->AIFaceDirection(-perpvel.Normalized());
		m_ship->AIMatchPosVel(relpos, relvel, m_endvel, false);
		return false;
	}

	// linear thrust
	if (m_state & 0x10) { m_ship->AIMatchVel(vector3d(0.0)); m_state &= 0xf; }
	else {
		double decel = m_ship->AIMatchPosVel(relpos, relvel, m_endvel, (m_state == 1));
		if (m_state == 1 && decel < 0) m_state = 2;		// time to flip
	}
	
	// set heading according to current state
	vector3d nextpos = m_ship->AIGetNextFramePos();			// position next frame before atmos/grav
	bool facing;
	if (m_state < 2) facing = m_ship->AIFaceDirection(targpos-nextpos);
	if (m_state == 2) facing = m_ship->AIFaceDirection(-reldir);
	if (m_state == 3) {
		vector3d newhead = 1.02 * GenerateTangent(m_ship, m_target, targpos);
		newhead = GetPosInFrame(m_ship->GetFrame(), m_target, newhead);
		facing = m_ship->AIFaceDirection(newhead-nextpos);
	}
	if (m_state > 3) m_ship->AIMatchAngVelObjSpace(vector3d(0.0));
	if (m_state < 4 && !facing) m_state |= 0x10;		// don't accel next frame if facing wrong way

	if (m_state == 5) return true;
	return false;
}


