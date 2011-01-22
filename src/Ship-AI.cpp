#include "libs.h"
#include "Ship.h"
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
		for (int j=0; j<accel.p.size(); j++) this_path_max_accel = MAX(this_path_max_accel, accel.p[j].Length());
		this_path_max_accel /= (outDuration*outDuration);
		if (this_path_max_accel < maxAccel) {
			printf("Path max accel is %f m.sec^-2, duration %f\n", this_path_max_accel, outDuration);
			return;
		}
	}
}

void Ship::AIBodyDeleted(const Body* const body)
{
	for (std::list<AIInstruction>::iterator i = m_todo.begin(); i != m_todo.end(); ) {
		switch ((*i).cmd) {
			case DO_KILL:
			case DO_LOW_ORBIT:
			case DO_MEDIUM_ORBIT:
			case DO_HIGH_ORBIT:
			case DO_KAMIKAZE:
			case DO_FLY_TO:
			case DO_DOCK:
				if (body == (*i).target) i = m_todo.erase(i);
				else i++;
				break;
			default:
				i++;
				break;
		}
	}
}

static bool FrameSphereIntersect(const vector3d &start, const vector3d &end, const double sphereRadius, double &outDist)
{
	outDist = 0;
	if (start.Length() <= sphereRadius) return true;
	if (end.Length() <= sphereRadius) return true;
	double length = (end-start).Length();
	double b = -vector3d::Dot(start, (end-start).Normalized());
	double det = (b*b) - vector3d::Dot(start, start) + sphereRadius*sphereRadius;
	if (det > 0.0) {
		det = sqrt(det);
		double i1 = b - det;
		double i2 = b + det;
		if (i2 > 0.0) {
			if (i1 < 0.0) {
				if (i2 < length) {
					outDist = i2;
					return true;
				}
			} else {
				if (i1 < length) {
					outDist = i1;
					return true;
				}
			}
		}
	}
	return false;
}

/**
 * Target should be in this->GetFrame() coordinates
 * If obstructor is not null then *obstructor will be set.
 */
bool Ship::AIArePlanetsInTheWayOfGettingTo(const vector3d &target, Body **obstructor, double &outDist)
{
	Body *body = Space::FindNearestTo(this, Object::PLANET);
	if (body) {
		Frame *frame = body->GetFrame();
		matrix4x4d m;
		Frame::GetFrameTransform(GetFrame(), frame, m);
		vector3d p2 = m * target;
		vector3d p1 = m * GetPosition();
		double rad = body->GetBoundingRadius();
		if (FrameSphereIntersect(p1, p2, rad, outDist)) {
			if (obstructor) *obstructor = body;
			return true;
		}
	}
	return false;
}

matrix4x4d Ship::g_orient;
double Ship::g_maxAccel = 0.0;
double Ship::g_invFrameAccel = 0.0;
double Ship::g_frameAccel = 0.0;

void Ship::AITimeStep(const float timeStep)
{
	bool done = false;
	
	// set up some handy vars 
	GetRotMatrix(g_orient);
	const ShipType &stype = GetShipType();
	g_maxAccel = stype.angThrust / GetAngularInertia();		// should probably be in stats anyway
	g_frameAccel = g_maxAccel * Pi::GetTimeAccel() / PHYSICS_HZ;
	g_invFrameAccel = 1.0 / g_frameAccel;

	// allow the launch thruster thing to happen
	if (m_launchLockTimeout != 0) return;
	
	if (m_todo.size() != 0) {
		AIInstruction &inst = m_todo.front();
		switch (inst.cmd) {
			case DO_DOCK:
				done = AICmdDock(inst, static_cast<SpaceStation*>(inst.target));
				break;
			case DO_KAMIKAZE:
				done = AICmdKamikaze(static_cast<const Ship*>(inst.target));
				break;
			case DO_KILL:
				done = AICmdKill(static_cast<const Ship*>(inst.target));
				break;
			case DO_LOW_ORBIT:
				done = AICmdOrbit(inst, 1.1);
				break;
			case DO_MEDIUM_ORBIT:
				done = AICmdOrbit(inst, 2.0);
				break;
			case DO_HIGH_ORBIT:
				done = AICmdOrbit(inst, 5.0);
				break;
			case DO_FLY_TO:
				done = AICmdFlyTo(inst);
				break;
			case DO_FOLLOW_PATH:
				done = AIFollowPath(inst, inst.frame, true);
				break;
			case DO_JOURNEY:
				done = AICmdJourney(inst);
				break;
			case DO_NOTHING: done = true; break;
		}
	}
	if (done) { 
		ClearThrusterState();
		printf("AI '%s' successfully executed %d\n", GetLabel().c_str(), m_todo.front().cmd);
		m_todo.pop_front();
		/* Finished autopilot program so fall out of time accel */
		if ((this == static_cast<Ship*>(Pi::player)) && (m_todo.size() == 0)) {
			// doesn't happen until next game tick, which is good
			// because AI will have set thrusters assuming a
			// particular timestep
			Pi::RequestTimeAccel(1);
			Pi::player->SetFlightControlState(Player::CONTROL_MANUAL);
		}
	}
}

bool Ship::AIAddAvoidancePathOnWayTo(const Body *target)
{
	Body *obstructor;
	double distanceToHit;
	if (!AIArePlanetsInTheWayOfGettingTo(target->GetPositionRelTo(GetFrame()), &obstructor, distanceToHit)) {
		return false;
	}
	if (obstructor == target) return false;
	if (10.0*obstructor->GetBoundingRadius() < obstructor->GetPositionRelTo(this).Length()) {
		// if the obstructor is miles away then we don't give a shit
		return false;
	}
	printf("Adding avoidance path around %s\n", obstructor->GetLabel().c_str());

	Frame *frame = obstructor->GetFrame();
	// avoid using the rotating frame for the path unless the target is in it
	if (frame->IsRotatingFrame() && (target->GetFrame() != frame)) frame = frame->m_parent;
	// hm. need to avoid some obstacle.
	const vector3d a = GetPositionRelTo(frame);
	const vector3d b = target->GetPositionRelTo(frame);
	const vector3d c = obstructor->GetPositionRelTo(frame);

	// if we are really close to the target then don't bother with this shit
	if ((a-b).Length() < 10000.0) return false;

	// so we try to avoid hitting innerRadius, and we try to
	// circumnavigate obstructor along outerRadius
	const double innerRadius = 1.8 * obstructor->GetBoundingRadius();
	const double outerRadius = 2.0 * obstructor->GetBoundingRadius();

	const vector3d fromCToHitPos = ((a + distanceToHit*((b-a).Normalized())) - c).Normalized();

	const vector3d perpendicularToHit = vector3d::Cross(vector3d::Cross(fromCToHitPos, vector3d(0.0,1.0,0.0)),
			a).Normalized();

	// So there is a circle around obstructor with radius 'outerRadius'.
	// Try to find a line from 'a' to the point on this circle nearest to
	// target position 'b'.
	// This becomes a bezier control point, and then we repeat the process
	// until we have unobstructed line of sight on 'b'.
	std::vector<vector3d> pos;
	pos.push_back(a);
	//printf("hitpos:\n");

	vector3d p = a;
	// special case, starting too near the planet */
	if (p.Length() < outerRadius) {
		p = a.Normalized() * outerRadius;
		// start by flying at zenith to get height
		pos.push_back(p);
	}

	double dummy;
	int max_to_do = 8;
	do {
		double minDistToTarget = FLT_MAX;
		double minDistToTargetAng = 0;

		for (double ang=0; ang<2.0*M_PI; ang += 0.1*M_PI) {
			const vector3d testPos = outerRadius * (sin(ang)*perpendicularToHit + cos(ang)*fromCToHitPos);
			if (!FrameSphereIntersect(p, testPos, innerRadius, dummy)) {
				//printf("hm. ang %f might work\n", ang);
				double distToTarget = (testPos - b).Length();
				if (distToTarget < minDistToTarget) {
					minDistToTarget = distToTarget;
					minDistToTargetAng = ang;
				}
			}
		}
		const vector3d nextPos = outerRadius * (sin(minDistToTargetAng)*perpendicularToHit + cos(minDistToTargetAng)*fromCToHitPos);
		
		if ((b.Length() < innerRadius) && ((b.Normalized()*outerRadius - b).Length() < minDistToTarget) &&
				!FrameSphereIntersect(p, b.Normalized()*outerRadius, innerRadius, dummy)) {
			// target is on the obstructing planet. flying to
			// point above target is closer than the chosen
			// minDist point so do that and finish the path with a
			// vertical descent
			pos.push_back(nextPos);
			pos.push_back(b.Normalized() * outerRadius);
			p = nextPos;
			break;
		} else {
			// use the selected minDist point along outerRadius circle
			assert(minDistToTarget != FLT_MAX);
			pos.push_back(nextPos);
			p = nextPos;
		}
		if (!max_to_do--) {
			/// XXX should really figure out why it happens...
			fprintf(stderr, "Hm. Ship::AIAddAvoidancePathOnWayTo() made a dog's arse of that one...\n");
			break;
		}
	} while (FrameSphereIntersect(p, b, innerRadius, dummy));

	if (b.Length() < innerRadius) {
		// target on planet. get close enough that this function won't
		// make another avoidance path...
		pos.push_back(b + 5000.0*b.Normalized());
	} else {
		// we only make a path round the side of the obstructor, not going on
		// to the target since some other AI function probably wants to do
		// this its own way.
		// same as last step but a bit further...
		pos.push_back(p + (pos[pos.size()-1] - pos[pos.size()-2]));
	}

	const vector3d ourVelocity = GetVelocityRelativeTo(frame);
	const vector3d endVelocity = vector3d(0.0);

	double duration;
	const ShipType &type = GetShipType();
	const double maxAccel = fabs(type.linThrust[ShipType::THRUSTER_FORWARD] / GetMass());
	AIInstruction &inst = AIPrependInstruction(DO_FOLLOW_PATH, obstructor);
	path(pos.size(), &pos[0], ourVelocity, endVelocity, maxAccel*.75, duration, inst.path);
	int i = 0;
	for (float t=0.0; i<=10; i++, t+=0.1) {
		vector3d v = inst.path.DerivativeOf().Eval(t) * (1.0/duration);
		//printf("%.1f: %f,%f,%f\n", t, v.x,v.y,v.z);
	}

	printf("duration %f\n", duration);
	inst.startTime = Pi::GetGameTime();
	inst.endTime = inst.startTime + duration;
	inst.frame = frame;
	return true;
}

bool Ship::AICmdDock(AIInstruction &inst, SpaceStation *target)
{
	Frame *frame = target->GetFrame();

	// Ship::DOCKING for example...
	if (GetFlightState() != Ship::FLYING) return true;

	// is there planets in our way that we need to avoid?
	if (AIAddAvoidancePathOnWayTo(target)) {
		// since we have added a new path, the current one will be invalid
		// and must be rebuilt:
		inst.endTime = 0;
		return false;
	}

	if (target->GetPositionRelTo(this).Length() > 100000.0) {
		AIPrependInstruction(DO_FLY_TO, target);
		return false;
	}

	if (frame->IsRotatingFrame() && !target->IsGroundStation()) {
		// can't autopilot by the rotating frame of a space station
		// (it rotates too fast and it would be incorrect anyway)
		frame = frame->m_parent;
	}

	int port = target->GetMyDockingPort(this);
	if (port == -1) {
		std::string msg;
		target->GetDockingClearance(this, msg);
		port = target->GetMyDockingPort(this);
		if (port == -1) {
			// hm should give a failure message, that we couldn't
			// get docking
			return true;
		}
	}

	if (inst.startTime == 0) {
		inst.startTime = Pi::GetGameTime();
		
		const double maxAccel = GetWeakestThrustersForce() / GetMass();
		vector3d pos[3];
		pos[0] = GetPositionRelTo(frame);
		const vector3d ourVelocity = GetVelocityRelativeTo(frame);
		const vector3d endVelocity = vector3d(0.0,-25.0,0.0);

		SpaceStationType::positionOrient_t shipOrientStartDocking, shipOrientEndDocking;
		bool good = target->GetSpaceStationType()->GetShipApproachWaypoints(port, 1, shipOrientStartDocking);
		good = good && target->GetSpaceStationType()->GetShipApproachWaypoints(port, 2, shipOrientEndDocking);

		assert(good);
		double duration;

		matrix4x4d target_rot;
		target->GetRotMatrix(target_rot);
		pos[1] = target->GetPosition() + target_rot * shipOrientStartDocking.pos;
		pos[2] = target->GetPosition() + target_rot * shipOrientEndDocking.pos;
	
		path(3, pos, ourVelocity, endVelocity, maxAccel*.5, duration, inst.path);
		inst.endTime = inst.startTime + duration;
	}
	if (inst.endTime > 0) {
		bool done = AIFollowPath(inst, frame, true);
		
		if (inst.endTime - Pi::GetGameTime() < 30.0) {
			// orient the ship in the last 30 seconds
			// should maybe cache this final orientation to avoid
			// the lua calls
			SpaceStationType::positionOrient_t shipOrientEndDocking;
			target->GetSpaceStationType()->GetShipApproachWaypoints(port, 2, shipOrientEndDocking);

			matrix4x4d orient;
			target->GetRotMatrix(orient);
			orient = orient * matrix4x4d::MakeRotMatrix(shipOrientEndDocking.xaxis, shipOrientEndDocking.yaxis,
					vector3d::Cross(shipOrientEndDocking.xaxis, shipOrientEndDocking.yaxis));
			SetAngThrusterState(0, 0);
			SetAngThrusterState(1, 0);
			SetAngThrusterState(2, 0);
			AISlowOrient(orient);
	
			if (GetWheelState() == 0) SetWheelState(true);
		}
		return done;
	} else {
		return true;
	}
}

bool Ship::AIFollowPath(AIInstruction &inst, Frame *frame, bool pointShipAtVelocityVector)
{
	const vector3d ourPosition = GetPositionRelTo(frame);
	const vector3d ourVelocity = GetVelocityRelativeTo(frame);
	double dur = inst.endTime - inst.startTime;
	// instead of trying to get to desired location on path curve
	// within a game tick, try adopting acceleration necessary to
	// get to desired point some fraction of remaining journey time in the
	// future, within that time. this avoids oscillation around a perfect position
	double reactionTime = CLAMP((inst.endTime-Pi::GetGameTime())*0.01, Pi::GetTimeStep(), 200.0);
	reactionTime = MIN(reactionTime, inst.endTime - Pi::GetGameTime());
	double t = (Pi::GetGameTime()+reactionTime - inst.startTime) / dur;
	vector3d wantVel;
	//printf("rtime: %f t %f\n", reactionTime, t);
	if (Pi::GetGameTime()+Pi::GetTimeStep() >= inst.endTime) {
		printf("end time %f, current time %f\n", inst.endTime, Pi::GetGameTime());
		return true;
	} else {
		vector3d wantPos = inst.path.Eval(t);
		vector3d diffPos = wantPos - ourPosition;
		wantVel = diffPos / reactionTime;
	}
	{
//		vector3d perfectPos = inst.path.Eval((Pi::GetGameTime()-inst.startTime)/dur);
//		printf("perfect deviation %f m\n", (perfectPos-ourPosition).Length());
	}
	{
	//	vector3d perfectVel = inst.path.DerivativeOf().Eval((Pi::GetGameTime()-inst.startTime)/dur) / dur;
	//	printf("perfect vel deviation %f m/s\n", (perfectVel-ourVelocity).Length());
	}
	vector3d diffVel = wantVel - ourVelocity;
	//printf("diff vel %f\n", diffVel.Length());
	vector3d accel = diffVel / Pi::GetTimeStep();
	//printf("%f m/sec/sec\n", accel.Length());
	vector3d force = GetMass() * accel;
	{
		matrix4x4d tran;
		// need rotation between target frame and ship, so force is in
		// correct model coords.
		Frame::GetFrameTransform(frame, GetFrame(), tran);
		tran.ClearToRotOnly();
		// make body-relative and apply force using thrusters
		matrix4x4d rot;
		GetRotMatrix(rot);
		force = rot.InverseOf() * tran * force;
		ClearThrusterState();
		AITrySetBodyRelativeThrust(force);
		if (pointShipAtVelocityVector) {
			if (wantVel.Length()) AISlowFaceDirection((tran * wantVel).Normalized());
		} else {
			// orient so main engines can be used most effectively
			vector3d perfectForce = inst.path.DerivativeOf().DerivativeOf().Eval(t);
			if (perfectForce.Length()) AISlowFaceDirection(perfectForce.Normalized());
		}
	}
	return false;
	//SetForce(force);
}

bool Ship::AICmdJourney(AIInstruction &inst)
{
	if (Pi::currentSystem->GetLocation() != (SysLoc)inst.journeyDest) {
		// need to hyperspace there
		int fuelRequired;
		double duration;
		enum Ship::HyperjumpStatus jumpStatus;
		CanHyperspaceTo(&inst.journeyDest, fuelRequired, duration, &jumpStatus);
		if (jumpStatus == Ship::HYPERJUMP_OK) {
			switch (GetFlightState()) {
			case FLYING:
				TryHyperspaceTo(&inst.journeyDest);
				break;
			case DOCKING:
				// just wait
				break;
			case LANDED:
				if (GetDockedWith()) {
					Undock();
				} else {
					Blastoff();
				}
				break;
			}
		} else {
			printf("AICmdJourney() can't get to destination (reason %d) :-(\n", (int)jumpStatus);
			if (!GetDockedWith()) {
				// if we aren't docked then there is no point trying to
				// buy fuel, etc. just give up
				printf("AICmdJourney() failed (not docked, HyperjumpStatus=%d)\n", (int)jumpStatus);
				return true;
			}

			switch (jumpStatus) {
			case Ship::HYPERJUMP_INSUFFICIENT_FUEL:
				{
					Equip::Type fuelType = GetHyperdriveFuelType();

					if (BuyFrom(GetDockedWith(), fuelType, false)) {
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
					const Equip::Type fuelType = GetHyperdriveFuelType();
					const Equip::Type driveType = m_equipment.Get(Equip::SLOT_ENGINE);
					const Equip::Type laserType = m_equipment.Get(Equip::SLOT_LASER, 0);
					// preserve money
					Sint64 oldMoney = GetMoney();
					SetMoney(10000000);
					MarketAgent *trader = GetDockedWith();
					// need to lose some equipment and see if we get light enough
					Equip::Type t = (Equip::Type)Pi::rng.Int32(Equip::TYPE_MAX);
					if ((EquipType::types[t].slot == Equip::SLOT_ENGINE) && trader->CanSell(t)) {
						// try a different hyperdrive
						SellTo(trader, driveType);
						if (!BuyFrom(trader, t)) {
							BuyFrom(trader, driveType);
						}
						printf("Switched drive to a %s\n", EquipType::types[t].name);
					} else if ((t != fuelType) && (t != driveType) && (t != laserType)) {
						SellTo(trader, t);
						printf("Removed a %s\n", EquipType::types[t].name);
					}
					SetMoney(oldMoney);
				}
				break;
			case Ship::HYPERJUMP_OK:
				break; // shouldn't reach this though
			}
		}
	} else {
		// we are in the desired system. fly to the target and dock
		// first remove the 'DO_JOURNEY' command
		m_todo.pop_front();
		// then specific instructions to get us there
		Body *b = Space::FindBodyForSBodyPath(&inst.journeyDest);
		AIPrependInstruction(DO_DOCK, b);
		AIPrependInstruction(DO_FLY_TO, b);
	}
	return false;
}

bool Ship::AICmdOrbit(AIInstruction &inst, double orbitHeight)
{
	bool done = false;
	Body *body = inst.target;

	// don't think about it
	if (!body->IsType(Object::PLANET)) return true;
			
	if (GetFlightState() == LANDED) {
		if (GetDockedWith()) {
			Undock();
		} else {
			Blastoff();
		}
		return false;
	}

	// is there planets in our way that we need to avoid?
	if (AIAddAvoidancePathOnWayTo(body)) {
		// since we have added a new path, the current one will be invalid
		// and must be rebuilt:
		inst.endTime = 0;
		return false;
	}

	Frame *frame = body->GetFrame()->m_parent;
	PiVerify(frame);

	if (inst.endTime == 0) {
		Planet *planet = static_cast<Planet*>(body);
		const ShipType &type = GetShipType();
		const vector3d ourPosition = GetPositionRelTo(frame);
		const vector3d ourVelocity = GetVelocityRelativeTo(frame);
		// XXX nice naming inconsistency ^^
		const double orbitalRadius = planet->GetSBody()->GetRadius() * orbitHeight;
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
		const double maxAccel = fabs(type.linThrust[ShipType::THRUSTER_FORWARD] / GetMass());
		printf("max accel %f m/sec/sec\n",maxAccel);
		vector3d pos[3];
		pos[0] = ourPosition;
		pos[1] = midpos;
		pos[2] = endpos;
		path(3, pos,
				ourVelocity, endVel, maxAccel*.75, duration, inst.path);
		inst.startTime = Pi::GetGameTime();
		inst.endTime = inst.startTime + duration;

		{
			double invD = 1.0/duration;
			vector3d p1 = inst.path.Eval(0.0);
			vector3d p2 = inst.path.Eval(invD);
			printf("starts at %f m/sec\n", (p1-p2).Length());
			p1 = inst.path.Eval(1.0-invD);
			p2 = inst.path.Eval(1.0);
			printf("ends at %f m/sec\n", (p1-p2).Length());

		}
	}

	if (inst.endTime > 0) {
		if (AIFollowPath(inst, frame)) {
			done = true;
		}
	}

	return done;
}

void Ship::AITrySetBodyRelativeThrust(const vector3d &force)
{
	const ShipType &type = GetShipType();

	double state[ShipType::THRUSTER_MAX];
	state[ShipType::THRUSTER_FORWARD] = MAX(force.z / type.linThrust[ShipType::THRUSTER_FORWARD], 0.0);
	state[ShipType::THRUSTER_REVERSE] = MAX(force.z / type.linThrust[ShipType::THRUSTER_REVERSE], 0.0);
	state[ShipType::THRUSTER_UP] = MAX(force.y / type.linThrust[ShipType::THRUSTER_UP], 0.0);
	state[ShipType::THRUSTER_DOWN] = MAX(force.y / type.linThrust[ShipType::THRUSTER_DOWN], 0.0);
	state[ShipType::THRUSTER_LEFT] = MAX(force.x / type.linThrust[ShipType::THRUSTER_LEFT], 0.0);
	state[ShipType::THRUSTER_RIGHT] = MAX(force.x / type.linThrust[ShipType::THRUSTER_RIGHT], 0.0);
	bool engines_not_powerful_enough = false;
	for (int i=0; i<(int)ShipType::THRUSTER_MAX; i++) {
		if (state[i] > 1.0) engines_not_powerful_enough = true;
		SetThrusterState((ShipType::Thruster)i, state[i]);
	}
#ifdef DEBUG
	if (engines_not_powerful_enough) {
		printf("AI: Crud. thrusters insufficient: ");
		for (int i=0; i<(int)ShipType::THRUSTER_MAX; i++) printf("%f ", state[i]);
		printf("\n");
	}
#endif
}

bool Ship::AICmdFlyTo(AIInstruction &inst)
{
	bool done = false;
	Body *body = inst.target;

	Frame *frame = body->GetFrame();
	if (frame->IsRotatingFrame()) frame = frame->m_parent;
	assert(frame);
	
	// is there planets in our way that we need to avoid?
	if (AIAddAvoidancePathOnWayTo(body)) {
		// since we have added a new path, the current one will be invalid
		// and must be rebuilt:
		inst.endTime = 0;
		return false;
	}

	if (inst.endTime == 0) {
		const ShipType &type = GetShipType();
		const vector3d ourPosition = GetPositionRelTo(frame);
		const vector3d ourVelocity = GetVelocityRelativeTo(frame);
		const vector3d endPosition = MAX(50000.0, 4.0 * body->GetBoundingRadius()) * (ourPosition - body->GetPosition()).Normalized();

		// generate path
		double duration;
		// assumption that rear thruster is most powerful
		const double maxAccel = fabs(type.linThrust[ShipType::THRUSTER_FORWARD] / GetMass());
		vector3d pos[3];
		pos[0] = ourPosition;
		pos[1] = 0.5*(ourPosition+endPosition);
		pos[2] = endPosition;
		path(3, pos,
				ourVelocity, body->GetVelocity(), maxAccel*.75, duration, inst.path);
		inst.startTime = Pi::GetGameTime();
		inst.endTime = inst.startTime + duration;

		{
			double invD = 1.0/duration;
			vector3d p1 = inst.path.Eval(0.0);
			vector3d p2 = inst.path.Eval(invD);
			printf("starts at %f m/sec\n", (p1-p2).Length());
			p1 = inst.path.Eval(1.0-invD);
			p2 = inst.path.Eval(1.0);
			printf("ends at %f m/sec\n", (p1-p2).Length());

		}
	}

	if (inst.endTime > 0) {
		if (AIFollowPath(inst, frame)) {
			done = true;
		}
	}

	return done;
}

bool Ship::AICmdKamikaze(const Ship *enemy)
{
	SetGunState(0,0);
	/* needs to deal with frames, large distances, and success */
	if (GetFrame() == enemy->GetFrame()) {
		const float dist = (enemy->GetPosition() - GetPosition()).Length();
		vector3d vRel = GetVelocityRelativeTo(enemy);
		vector3d dir = (enemy->GetPosition() - GetPosition()).Normalized();

		const double eta = CLAMP(dist / vector3d::Dot(vRel, dir), 0.0, 10.0);
		const vector3d enemyProjectedPos = enemy->GetPosition() + eta*enemy->GetVelocity() - eta*GetVelocity();
		dir = (enemyProjectedPos - GetPosition()).Normalized();

		ClearThrusterState();
		AIFaceDirection(dir);

		// thunder at target at 400m/sec
		AIModelCoordsMatchSpeedRelTo(vector3d(0,0,-400), enemy);
	}
	return false;
}


bool Ship::AICmdKill(const Ship *enemy)
{
	SetGunState(0,0);
	// launch if docked
	if (GetDockedWith()) Undock();
	/* needs to deal with frames, large distances, and success */
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



void Ship::AIInstruct(enum AICommand cmd, void *arg)
{
	AIInstruction inst(cmd);
	inst.target = (Body*)arg;
	m_todo.push_back(inst);
}

void Ship::AIInstructJourney(const SBodyPath &path)
{
	AIInstruction inst(DO_JOURNEY);
	inst.journeyDest = path;
	m_todo.push_back(inst);
}

Ship::AIInstruction &Ship::AIPrependInstruction(enum AICommand cmd, void *arg)
{
	AIInstruction inst(cmd);
	inst.target = (Body*)arg;
	m_todo.push_front(inst);
	return m_todo.front();
}

/* Orient so our -ve z axis == dir. ie so that dir points forwards */
void Ship::AISlowFaceDirection(const vector3d &dir)
{
	vector3d zaxis = -dir;
	vector3d xaxis = vector3d::Cross(vector3d(0.0,1.0,0.0), zaxis).Normalized();
	vector3d yaxis = vector3d::Cross(zaxis, xaxis).Normalized();
	matrix4x4d wantOrient = matrix4x4d::MakeRotMatrix(xaxis, yaxis, zaxis).InverseOf();
	AISlowOrient(wantOrient);
}

void Ship::AISlowOrient(const matrix4x4d &wantedOrient)
{
	double timeAccel = Pi::GetTimeAccel();
	if (timeAccel > 11.0) {
		SetRotMatrix(wantedOrient);
	} else {
		matrix4x4d rot;
		GetRotMatrix(rot);
		Quaterniond q = (~Quaterniond::FromMatrix4x4(rot)) * Quaterniond::FromMatrix4x4(wantedOrient);
		double angle;
		vector3d rotaxis;
		q.GetAxisAngle(angle, rotaxis);
		AIModelCoordsMatchAngVel(angle * rotaxis.Normalized() * (1.0/timeAccel), 10.0);
	}
}

/*

// First half of FaceDirection(dir)
// input dir = direction to face
// output dav = desired angular velocity at current frame
// output time = time in seconds to 

void Ship::AIFaceDirection(const vector3d &dir, vector3d &dav, double time)
{
	matrix4x4d rot;	GetRotMatrix(rot);
	vector3d head = dir * rot;		// create desired object-space heading
	dav(0.0, 0.0, 0.0);				// desired angular velocity

	if (head.z < -0.99999999f) { time = 0.0; return; }

	double ang = acos (CLAMP(-head.z, -1.0, 1.0));		// scalar angle from head to curhead
	double iangvel = sqrt (2.0 * g_maxAccel * ang);		// ideal angvel at current time

	// probably shouldn't have this in combat AI? put inaccuracy somewhere else maybe
	double frameEndAV = iangvel - g_frameAccel;
	if (frameEndAV <= 0.0) iangvel = iangvel * 0.5;		// last frame discrete correction
	iangvel = (iangvel + frameEndAV) * 0.5;				// discrete overshoot correction


	// Normalize (head.x, head.y) to give desired angvel direction
	double head2dnorm = head.x*head.x + head.y*head.y;
	if (head2dnorm == 0.0) { head.y = 1.0; head2dnorm = 1.0; }	// NaN fix
	else head2dnorm = 1.0 / sqrt(head2dnorm);
	dav.x = head.y * head2dnorm * iangvel;
	dav.y = -head.x * head2dnorm * iangvel;
}

// how to generate time to target?
// actually quite tricky
// given zero start/end, time = root2 * iav / maxaccel
// given iav start, zero end, time = iav / maxaccel
// use proper integral maybe?



// Second half of FaceDirection(dir), essentially
// input: dav = desired angular velocity
// output: Sets angular thruster states 

void Ship::AIMatchAngVel(const vector3d &dav)
{
	vector3d cav = GetAngVelocity() * g_orient;		// current obj-rel angvel
	vector3d diff = dav - cav;					// find diff between current & desired angvel

	SetAngThrusterState(0, diff.x * g_invFrameAccel);
	SetAngThrusterState(1, diff.y * g_invFrameAccel);
	SetAngThrusterState(2, diff.z * g_invFrameAccel);
}

*/
// Does some kind some kind of v*v = 2as
// where:
//		- v = maximum angvel around arc to slow to zero by desired heading
//		- s = angle between current and desired heading
//		- a = maximum acceleration along desired heading

// Fancier code to use both x and y angthrust if possible
/*		double acc2;
		if (head.x*head.x > head.y*head.y) acc2 = maxAccel * head.y / head.x;
		else acc2 = maxAccel * head.x / head.y;
		double maxAccDir = sqrt(maxAccel*maxAccel + acc2*acc2);		// maximum accel in desired direction
		double iangvel = sqrt (2.0 * maxAccDir * ang);			// ideal angvel at current time
*/

// Applies thrust directly

void Ship::AIFaceDirection(const vector3d &dir)
{
	vector3d head = dir * g_orient;		// create desired object-space heading
	vector3d dav(0.0, 0.0, 0.0);	// desired angular velocity

	if (head.z > -0.99999999f)
	{
		double ang = acos (CLAMP(-head.z, -1.0, 1.0));		// scalar angle from head to curhead
		double iangvel = sqrt (2.0 * g_maxAccel * ang);		// ideal angvel at current time

		double frameEndAV = iangvel - g_frameAccel;
		if (frameEndAV <= 0.0) iangvel = iangvel * 0.5;		// last frame discrete correction
		iangvel = (iangvel + frameEndAV) * 0.5;				// discrete overshoot correction

		// Normalize (head.x, head.y) to give desired angvel direction
		double head2dnorm = head.x*head.x + head.y*head.y;
		if (head2dnorm == 0.0) { head.y = 1.0; head2dnorm = 1.0; }	// NaN fix
		else head2dnorm = 1.0 / sqrt(head2dnorm);
		dav.x = head.y * head2dnorm * iangvel;
		dav.y = -head.x * head2dnorm * iangvel;
	}
	vector3d cav = GetAngVelocity() * g_orient;		// current obj-rel angvel
	vector3d diff = dav - cav;					// find diff between current & desired angvel

	SetAngThrusterState(0, diff.x * g_invFrameAccel);
	SetAngThrusterState(1, diff.y * g_invFrameAccel);
	SetAngThrusterState(2, diff.z * g_invFrameAccel);
}


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
// need following results:
// "optimal" angular thrust to get sights on target
// approximate time required

//void Ship::CombatTurn(const Ship *enemy)
void Ship::AIFaceTargetLead(const Ship *enemy)
{
	// target leading approximation?
	// - take time-to-target using current distance between ships
	// - don't worry about second-order issues like time required to get on target 
	// changing end point

	// So, basic target-leading...
	// do this all in object space?

	// get some object space relative values
	vector3d targpos = enemy->GetPositionRelTo(this) * g_orient;
vector3d targpos2 = (enemy->GetPosition() - GetPosition()) * g_orient;
	vector3d targvel = enemy->GetVelocityRelativeTo(this) * g_orient;
vector3d targvel2 = (enemy->GetVelocity() - GetVelocity()) * g_orient;
	// later: should adjust targpos for gunmount offset lol

	int laser = Equip::types[m_equipment.Get(Equip::SLOT_LASER, 0)].tableIndex;
	double projspeed = Equip::lasers[laser].speed;

	vector3d leadpos = targpos + targvel*(targpos.Length()/projspeed);
	leadpos = targpos + targvel*(leadpos.Length()/projspeed); 	// second order approx
	vector3d leaddir = leadpos.Normalized();
	// later: could repeat this with new targdist

	// and approximate target angular velocity at leaddir
	vector3d leaddir2 = (leadpos + targvel*0.01).Normalized();
	vector3d leadav = leaddir.Cross(leaddir2) * 100.0;
	// does this really give a genuine angvel? Probably

	// TEST: ok, let's throw this at FaceDirection for the moment

	AIFaceDirection(g_orient * leaddir);
	return;

	// so have target heading and target angvel at that heading
	// can now use modified version of FaceDirection?
	// not really: direction of leaddir and leadangvel may be different
	// so blend two results: thrust to reach leaddir and thrust to attain leadangvel
	// bias towards leadangvel as heading approaches leaddir

}

/* Orient so our -ve z axis == dir. ie so that dir points forwards */
/*void Ship::AIFaceDirection(const vector3d &dir)
{
	double invTimeAccel = 1.0 / Pi::GetTimeAccel();
	matrix4x4d rot;
	GetRotMatrix(rot);
	rot = rot.InverseOf();
	vector3d zaxis = vector3d(-rot[2], -rot[6], -rot[10]);
	if (Pi::GetTimeAccel() > 11.0) {
		// fake it
		zaxis = -dir;
		vector3d yaxis(rot[1], rot[5], rot[9]);
		vector3d xaxis = vector3d::Cross(yaxis, zaxis).Normalized();
		yaxis = vector3d::Cross(zaxis, xaxis);
		SetRotMatrix(matrix4x4d::MakeRotMatrix(xaxis, yaxis, zaxis).InverseOf());
	} else {
		vector3d rotaxis = rot * vector3d::Cross(zaxis, dir);
		vector3d angVel = rot * GetAngVelocity();
		const float dot = vector3d::Dot(dir, zaxis);
		// if facing > 90 degrees away then max turn rate
		rotaxis = rotaxis.Normalized();
//		if (dot < 0) rotaxis = -rotaxis;
		double angToGo = acos(CLAMP(dot, -1.0, 1.0));
		// agreement between angVel and rotAxis
		double goodAngVel = vector3d::Dot(angVel, rotaxis);

		if (dot > 0.99999) {
			angVel *= -invTimeAccel;
			SetAngThrusterState(0, angVel.x);
			SetAngThrusterState(1, angVel.y);
			SetAngThrusterState(2, angVel.z);
			return;
		}

		const ShipType &stype = GetShipType();
		double angAccel = stype.angThrust / GetAngularInertia();
		double timeToStop = goodAngVel / angAccel;
		// angle travelled until rotation can be stopped by thrusters
		double stoppingAng = 0.5 * angAccel * timeToStop * timeToStop;

		vector3d desiredAngVelChange = (rotaxis - angVel) * invTimeAccel;
		if (dot < 0.95) {
			// weirdness!
		//	desiredAngVelChange += vector3d(noise(angVel), noise(angVel+vector3d(1,0,0)), noise(2.0*angVel+vector3d(2,0,0)));
		}
		if (stoppingAng < 0.8*angToGo) {
			SetAngThrusterState(0, desiredAngVelChange.x);
			SetAngThrusterState(1, desiredAngVelChange.y);
			SetAngThrusterState(2, desiredAngVelChange.z);
		}
		else if (stoppingAng > 0.9*angToGo) {
			if (timeToStop > 0.0) {
				SetAngThrusterState(0, -desiredAngVelChange.x);
				SetAngThrusterState(1, -desiredAngVelChange.y);
				SetAngThrusterState(2, -desiredAngVelChange.z);
			} else {
				SetAngThrusterState(0, desiredAngVelChange.x);
				SetAngThrusterState(1, desiredAngVelChange.y);
				SetAngThrusterState(2, desiredAngVelChange.z);
			}
		}
	}
}
*/

void Ship::AIModelCoordsMatchAngVel(vector3d desiredAngVel, float softness)
{
	const ShipType &stype = GetShipType();
	double angAccel = stype.angThrust / GetAngularInertia();
	const double softTimeStep = Pi::GetTimeStep() * (double)softness;

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
	SetAngThrusterState(0, thrust.x);
	SetAngThrusterState(1, thrust.y);
	SetAngThrusterState(2, thrust.z);
}


void Ship::AIModelCoordsMatchSpeedRelTo(const vector3d v, const Ship *other)
{
	matrix4x4d m; GetRotMatrix(m);
	vector3d relToVel = m.InverseOf() * other->GetVelocity() + v;
	AIAccelToModelRelativeVelocity(relToVel);
}

#include "Frame.h"
/* Try to reach this model-relative velocity.
 * (0,0,-100) would mean going 100m/s forward.
 */
void Ship::AIAccelToModelRelativeVelocity(const vector3d v)
{
	const ShipType &stype = GetShipType();
	
	// OK. For rotating frames linked to space stations we want to set
	// speed relative to non-rotating frame (so we apply Frame::GetStasisVelocityAtPosition.
	// For rotating frames linked to planets we want to set velocity relative to
	// surface, so we do not apply Frame::GetStasisVelocityAtPosition
	vector3d relVel = GetVelocity();
	if (!GetFrame()->m_astroBody) {
		relVel -= GetFrame()->GetStasisVelocityAtPosition(GetPosition());
	}
	matrix4x4d m; GetRotMatrix(m);
	relVel = m.InverseOf() * relVel;

	vector3d difVel = v - relVel;
	// want to change velocity by difVel...
//	SetVelocity(m * (relVel + difVel));
	const float invMass = 1.0 / GetMass();

	if (difVel.x > 0) {
		// figure out biggest accel can get, and then what we need this timestep.
		float velChange = Pi::GetTimeStep() * stype.linThrust[ShipType::THRUSTER_RIGHT] * invMass;
		float thrust;
		if (velChange < difVel.x) thrust = 1.0;
		else thrust = difVel.x / velChange;
		thrust *= thrust; // this is just to hide control jiggle
		SetThrusterState(ShipType::THRUSTER_RIGHT, thrust);
	} else {
		float velChange = Pi::GetTimeStep() * stype.linThrust[ShipType::THRUSTER_LEFT] * invMass;
		float thrust;
		if (velChange > difVel.x) thrust = 1.0;
		else thrust = difVel.x / velChange;
		thrust *= thrust;
		SetThrusterState(ShipType::THRUSTER_LEFT, thrust);
	}

	if (difVel.y > 0) {
		float velChange = Pi::GetTimeStep() * stype.linThrust[ShipType::THRUSTER_UP] * invMass;
		float thrust;
		if (velChange < difVel.y) thrust = 1.0;
		else thrust = difVel.y / velChange;
		thrust *= thrust;
		SetThrusterState(ShipType::THRUSTER_UP, thrust);
	} else {
		float velChange = Pi::GetTimeStep() * stype.linThrust[ShipType::THRUSTER_DOWN] * invMass;
		float thrust;
		if (velChange > difVel.y) thrust = 1.0;
		else thrust = difVel.y / velChange;
		thrust *= thrust;
		SetThrusterState(ShipType::THRUSTER_DOWN, thrust);
	}

	if (difVel.z > 0) {
		float velChange = Pi::GetTimeStep() * stype.linThrust[ShipType::THRUSTER_REVERSE] * invMass;
		float thrust;
		if (velChange < difVel.z) thrust = 1.0;
		else thrust = difVel.z / velChange;
		thrust *= thrust;
		SetThrusterState(ShipType::THRUSTER_REVERSE, thrust);
	} else {
		float velChange = Pi::GetTimeStep() * stype.linThrust[ShipType::THRUSTER_FORWARD] * invMass;
		float thrust;
		if (velChange > difVel.z) thrust = 1.0;
		else thrust = difVel.z / velChange;
		thrust *= thrust;
		SetThrusterState(ShipType::THRUSTER_FORWARD, thrust);
	}
}

