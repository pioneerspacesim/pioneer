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


/**
 * Should be at least 3 points (start, 1 control point, and endpoint). This
 * will give a bezier curve with 5 points, because the velocity constraints at
 * either end introduce a control point each. */
static void path(int numPoints, const vector3d *points,
		const vector3d &startVel, const vector3d &endVel, const double maxAccel,
		double &outDuration, BezierCurve &outPath)
{
	assert(numPoints >= 3);
	// How do you get derivative of a bezier line?
	// http://www.cs.mtu.edu/~shene/COURSES/cs3621/NOTES/spline/Bezier/bezier-der.html
	// path is a quartic bezier:
//	vector3d /*p0,*/ p1, p2, p3/*, p4*/;
	// velocity along this path is derivative of above bezier, and
	// is a cubic bezier:
	// acceleration is derivative of the velocity bezier and is a quadric bezier

	outDuration = 1.0;
	
	// max journey length 2^30 seconds
	for (int i=0; i<30; i++, outDuration *= 2.0) {
		const vector3d _startVel = startVel * outDuration;
		const vector3d _endVel = endVel * outDuration;

		/* comes from differentiation. see BezierCurve.h */
		const double d = 1.0/(double)(numPoints+1);
		outPath = BezierCurve(numPoints+2);
		outPath.p[0] = points[0];
		outPath.p[1] = points[0] + d*_startVel;
		for (int j=2; j<numPoints; j++) outPath.p[j] = points[j-1];
		outPath.p[numPoints] = points[numPoints-1] - d*_endVel;
		outPath.p[numPoints+1] = points[numPoints-1];

		BezierCurve vel = outPath.DerivativeOf();
		BezierCurve accel = vel.DerivativeOf();

		double this_path_max_accel = 0;
		for (int j=0; j<accel.p.size(); j++) this_path_max_accel = std::max(this_path_max_accel, accel.p[j].Length());
		this_path_max_accel /= (outDuration*outDuration);
		if (this_path_max_accel < maxAccel) {
			printf("Path max accel is %f m.sec^-2, duration %f\n", this_path_max_accel, outDuration);
			return;
		}
	}
}

AICommand *AICommand::Load(Serializer::Reader &rd)
{
	CmdName name = (CmdName)rd.Int32();
	switch (name) {
		case CMD_NONE: return 0;
		case CMD_JOURNEY: return new AICmdJourney(rd);
		case CMD_DOCK: return new AICmdDock(rd);
		case CMD_ORBIT: return new AICmdOrbit(rd);
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
					vector3d::Cross(shipOrientEndDocking.xaxis, shipOrientEndDocking.yaxis));
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

bool AICmdOrbit::TimeStepUpdate()
{
	if (!ProcessChild()) return false;

	// don't think about it
	if (!m_target->IsType(Object::PLANET)) return true;
			
	if (m_ship->GetFlightState() == Ship::LANDED) {
		if (m_ship->GetDockedWith()) {
			m_ship->Undock();
		} else {
			m_ship->Blastoff();
		}
		return false;
	}

	// is there planets in our way that we need to avoid?
	AIPath newPath;
	if (m_ship->AIAddAvoidancePathOnWayTo(m_target, newPath)) {
		m_child = new AICmdFlyTo(m_ship, m_target, newPath);
		m_path.endTime = 0;
		return false;
	}

	if (m_path.endTime == 0) {
		Frame *frame = m_target->GetFrame()->m_parent;
		PiVerify(frame);

		Planet *planet = static_cast<Planet*>(m_target);
		const ShipType &type = m_ship->GetShipType();
		const vector3d ourPosition = m_ship->GetPositionRelTo(frame);
		const vector3d ourVelocity = m_ship->GetVelocityRelativeTo(frame);
		// XXX nice naming inconsistency ^^
		const double orbitalRadius = planet->GetSBody()->GetRadius() * m_orbitHeight;
		const double orbitalSpeed = sqrt(planet->GetMass() * G / orbitalRadius);

		vector3d midpos, endpos, endVel;
		// approach differently if we are currently above or below orbit
		if (ourPosition.Length() < orbitalRadius) {
			endpos = vector3d::Cross(vector3d(0.0,1.0,0.0), ourPosition).Normalized();
			midpos = ourPosition.Normalized() * orbitalRadius*1.1;
			endVel = endpos * orbitalSpeed;
			endpos = (midpos + endpos*(midpos-ourPosition).Length()).Normalized() * orbitalRadius;
		} else {
			endpos = vector3d::Cross(vector3d(0.0,1.0,0.0), ourPosition);
			endpos = endpos.Normalized() * orbitalRadius;
			midpos = (ourPosition.Normalized() + endpos.Normalized())
					.Normalized()* orbitalRadius*1.1;
			endVel = ourPosition.Normalized()*-orbitalSpeed;
		}
		printf("Pos %f,%f,%f\n",
				ourPosition.x,
				ourPosition.y,
				ourPosition.z);
		printf("Endpos %f,%f,%f (%f)\n", endpos.x, endpos.y, endpos.z, endpos.Length());
		// generate path
		double duration;
		// assumption that rear thruster is most powerful
		const double maxAccel = fabs(type.linThrust[ShipType::THRUSTER_FORWARD] / m_ship->GetMass());
		printf("max accel %f m/sec/sec\n",maxAccel);
		vector3d pos[3];
		pos[0] = ourPosition;
		pos[1] = midpos;
		pos[2] = endpos;
		path(3, pos, ourVelocity, endVel, maxAccel*.75, duration, m_path.path);
		m_path.startTime = Pi::GetGameTime();
		m_path.endTime = m_path.startTime + duration;
		m_path.frame = frame;

		{
			double invD = 1.0/duration;
			vector3d p1 = m_path.path.Eval(0.0);
			vector3d p2 = m_path.path.Eval(invD);
			printf("starts at %f m/sec\n", (p1-p2).Length());
			p1 = m_path.path.Eval(1.0-invD);
			p2 = m_path.path.Eval(1.0);
			printf("ends at %f m/sec\n", (p1-p2).Length());

		}
	}

	if (m_path.endTime <= 0) return true;
	return m_ship->AIFollowPath(m_path);
}

bool AICmdFlyTo::TimeStepUpdate()
{
	if (!ProcessChild()) return false;

	if (m_path.endTime == 0) {
		AIPath newPath;
		if (m_ship->AIAddAvoidancePathOnWayTo(m_target, newPath)) {
			m_child = new AICmdFlyTo(m_ship, m_target, newPath);
			m_path.endTime = 0;
			return false;
		}

		Frame *frame = m_target->GetFrame();
		if (frame->IsRotatingFrame()) frame = frame->m_parent;
		assert(frame);

		const ShipType &type = m_ship->GetShipType();
		const vector3d ourPosition = m_ship->GetPositionRelTo(frame);
		const vector3d ourVelocity = m_ship->GetVelocityRelativeTo(frame);
		// bug: broken
		const vector3d endPosition = std::max(50000.0, 4.0 * m_target->GetBoundingRadius()) * (ourPosition - m_target->GetPosition()).Normalized();

		// generate path
		double duration;
		// assumption that rear thruster is most powerful
		const double maxAccel = fabs(type.linThrust[ShipType::THRUSTER_FORWARD] / m_ship->GetMass());
		vector3d pos[3];
		pos[0] = ourPosition;
		pos[1] = 0.5*(ourPosition+endPosition);
		pos[2] = endPosition;
		path(3, pos, ourVelocity, m_target->GetVelocity(), maxAccel*.75, duration, m_path.path);
		m_path.startTime = Pi::GetGameTime();
		m_path.endTime = m_path.startTime + duration;
		m_path.frame = frame;

		{
			double invD = 1.0/duration;
			vector3d p1 = m_path.path.Eval(0.0);
			vector3d p2 = m_path.path.Eval(invD);
			printf("starts at %f m/sec\n", (p1-p2).Length());
			p1 = m_path.path.Eval(1.0-invD);
			p2 = m_path.path.Eval(1.0);
			printf("ends at %f m/sec\n", (p1-p2).Length());

		}
	}

	return m_ship->AIFollowPath(m_path);
}

bool AICmdKamikaze::TimeStepUpdate()
{
	m_ship->SetGunState(0,0);
	/* needs to deal with frames, large distances, and success */
	if (m_ship->GetFrame() == m_target->GetFrame()) {
		const float dist = (m_target->GetPosition() - m_ship->GetPosition()).Length();
		vector3d vRel = m_ship->GetVelocityRelativeTo(m_target);
		vector3d dir = (m_target->GetPosition() - m_ship->GetPosition()).Normalized();

		const double eta = Clamp(dist / vector3d::Dot(vRel, dir), 0.0, 10.0);
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
bool Ship::AICmdKill(const Ship *enemy)
{
	SetGunState(0,0);
	// launch if docked
	if (GetDockedWith()) Undock();
	// needs to deal with frames, large distances, and success
	if (GetFrame() == enemy->GetFrame()) {
		const float dist = (enemy->GetPosition() - GetPosition()).Length();
		vector3d dir = (enemy->GetPosition() - GetPosition()).Normalized();
		ClearThrusterState();
//		if (dist > 500.0) {
//			AIFaceDirection(dir);
			AIFaceTargetLead(enemy);
			// thunder at player at 400m/sec
//			if (dist > 5000.0) AIModelCoordsMatchSpeedRelTo(vector3d(0,0,-400), enemy);
			// fire guns if aiming well enough	
			matrix4x4d rot;
			GetRotMatrix(rot);
			const vector3d zaxis = vector3d(-rot[8], -rot[9], -rot[10]);
			const float dot = vector3d::Dot(dir, vector3d(-rot[8], -rot[9], -rot[10]));
			if (dot > 0.95f) {
				SetGunState(0,1);
	//const SBodyPath *path = Pi::player->GetHyperspaceTarget();
	//TryHyperspaceTo(path);
			}
//		} else {
//			// if too close turn away!
//			AIFaceDirection(-dir);
//			AIModelCoordsMatchSpeedRelTo(vector3d(0,0,-1000), enemy);
//		}
	}
	return false;
}
*/

/*
// principle? Use linear thrusters to dodge and close or open range
// Use angular thrusters either to get on target or increase evasion ability
// 

void Ship::AIFight()
{
	// determine approximate time-to-threat and damage potential

		

	// determine approximate time-to-threaten and damage potential


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


bool AICmdKill::TimeStepUpdate()
{
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