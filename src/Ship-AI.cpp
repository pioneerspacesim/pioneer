#include "libs.h"
#include "Ship.h"
#include "Pi.h"
#include "Player.h"
#include "perlin.h"

static void path(double duration, const vector3d &startPos, const vector3d &controlPos, const vector3d &endPos,
		vector3d startVel, vector3d endVel, QuarticBezier &outPath)
{
	// How do you get derivative of a bezier line?
	// http://www.cs.mtu.edu/~shene/COURSES/cs3621/NOTES/spline/Bezier/bezier-der.html
	// path is a quartic bezier:
//	vector3d /*p0,*/ p1, p2, p3/*, p4*/;
	// velocity along this path is derivative of above bezier, and
	// is a cubic bezier:
	vector3d v0, v1, v2, v3;
	// acceleration is derivative of the velocity bezier and is a quadric bezier
	vector3d a0, a1, a2;
	startVel *= duration;
	endVel *= duration;

	outPath.p0 = startPos;
	outPath.p1 = startPos + 0.25*startVel;
	outPath.p2 = controlPos;
	outPath.p3 = endPos - 0.25*endVel;
	outPath.p4 = endPos;

	v0 = startVel;
	v1 = 4.0*(outPath.p2 - outPath.p1);
	v2 = 4.0*(outPath.p3 - outPath.p2);
	v3 = endVel;

	a0 = 3.0*(v1-v0);
	a1 = 3.0*(v2-v1);
	a2 = 3.0*(v3-v2);

	double max_accel = MAX(a0.Length(), MAX(a1.Length(), a2.Length()));
	printf("Path max accel is %f m.sec^-2\n", max_accel/(duration*duration));
}

void Ship::AIBodyDeleted(const Body* const body)
{
	for (std::list<AIInstruction>::iterator i = m_todo.begin(); i != m_todo.end(); ) {
		switch ((*i).cmd) {
			case DO_KILL:
			case DO_ORBIT:
			case DO_KAMIKAZE:
			case DO_FLY_TO:
				if (body == (Body*)(*i).arg) i = m_todo.erase(i);
				else i++;
				break;
			default:
				i++;
				break;
		}
	}
}

void Ship::AITimeStep(const float timeStep)
{
	bool done = false;

	if (m_todo.size() != 0) {
		AIInstruction &inst = m_todo.front();
		switch (inst.cmd) {
			case DO_KAMIKAZE:
				done = AICmdKamikaze(static_cast<const Ship*>(inst.arg));
				break;
			case DO_KILL:
				done = AICmdKill(static_cast<const Ship*>(inst.arg));
				break;
			case DO_ORBIT:
				done = AICmdOrbit(inst);
				break;
			case DO_FLY_TO:
				done = AICmdFlyTo(static_cast<const Body*>(inst.arg));
				break;
			case DO_NOTHING: done = true; break;
		}
	}
	if (done) { 
		printf("AI '%s' successfully executed %d:'%s'\n", GetLabel().c_str(), m_todo.front().cmd,
				static_cast<Ship*>(m_todo.front().arg)->GetLabel().c_str());
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

bool Ship::AIFollowPath(AIInstruction &inst)
{
	double dur = inst.endTime - inst.startTime;
	// instead of trying to get to desired location on path curve
	// within a game tick, try adopting acceleration necessary to
	// get to desired point 60 seconds in the future, within 60
	// seconds
	double reactionTime = Pi::GetTimeStep();//CLAMP((inst.endTime-Pi::GetGameTime())*0.01, Pi::GetTimeStep(), 60.0);
	reactionTime = MIN(reactionTime, inst.endTime - Pi::GetGameTime());
//	printf("%f\n", reactionTime);
	// XXX subtly wrong -- misses time at end of journey
	double t = (Pi::GetGameTime()+reactionTime - inst.startTime) / dur;
	vector3d wantVel;
//	printf("t %f\n", t);
	if (Pi::GetGameTime()+Pi::GetTimeStep() >= inst.endTime) {
		return true;
	} else {
		vector3d wantPos = inst.path.Eval(t);
		vector3d diffPos = wantPos - GetPosition();
		wantVel = diffPos / reactionTime;
	}
	{
//		vector3d perfectPos = inst.path.Eval((Pi::GetGameTime()-inst.startTime)/dur);
//		printf("perfect deviation %f m\n", (perfectPos-GetPosition()).Length());
	}
	{
	//	vector3d perfectVel = inst.path.DerivativeOf().Eval((Pi::GetGameTime()-inst.startTime)/dur) / dur;
	//	printf("perfect vel deviation %f m/s\n", (perfectVel-GetVelocity()).Length());
	}
	vector3d diffVel = wantVel - GetVelocity();
	//printf("diff vel %f\n", diffVel.Length());
	vector3d accel = diffVel / Pi::GetTimeStep();
	printf("%f m/sec/sec\n", accel.Length());
	vector3d force = GetMass() * accel;
	{
		// make body-relative and apply force using thrusters
		matrix4x4d rot;
		GetRotMatrix(rot);
		force = rot.InverseOf() * force;
		ClearThrusterState();
		AITrySetBodyRelativeThrust(force);
		// orient so main engines can be used most effectively
		vector3d perfectForce = inst.path.DerivativeOf().DerivativeOf().Eval(t);
		if (perfectForce.Length()) AISlowFaceDirection(perfectForce.Normalized());
	}
	return false;
	//SetForce(force);
}

bool Ship::AICmdOrbit(AIInstruction &inst)
{
	bool done = false;
	Body *body = (Body*)inst.arg;

//XXX warning must consider frames of reference
	if (inst.endTime == 0) {
		vector3d endpos = vector3d::Cross(vector3d(0.0,1.0,0.0), GetPosition());
		endpos = endpos.Normalized() * 42164000.0; // geostationary
		vector3d midpos = (GetPosition().Normalized() + endpos.Normalized())
				.Normalized()* 42164000.0;
		vector3d endVel = GetPosition().Normalized()*-3070.0;
		printf("Pos %f,%f,%f\n",
				GetPosition().x,
				GetPosition().y,
				GetPosition().z);
		printf("Endpos %f,%f,%f (%f)\n", endpos.x, endpos.y, endpos.z, endpos.Length());
		// generate path
		const double duration = 60.0*60.0;
		path(duration, GetPosition(), midpos, endpos,
				GetVelocity(), endVel, inst.path);
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
		if (AIFollowPath(inst)) {
			done = true;
		}
	}

	return done;
}

void Ship::AITrySetBodyRelativeThrust(const vector3d &force)
{
	double f;
	const ShipType &type = GetShipType();

	double state[ShipType::THRUSTER_MAX];
	state[ShipType::THRUSTER_REAR] = MAX(force.z / type.linThrust[ShipType::THRUSTER_REAR], 0.0);
	state[ShipType::THRUSTER_FRONT] = MAX(force.z / type.linThrust[ShipType::THRUSTER_FRONT], 0.0);
	state[ShipType::THRUSTER_TOP] = MAX(force.y / type.linThrust[ShipType::THRUSTER_TOP], 0.0);
	state[ShipType::THRUSTER_BOTTOM] = MAX(force.y / type.linThrust[ShipType::THRUSTER_BOTTOM], 0.0);
	state[ShipType::THRUSTER_LEFT] = MAX(force.x / type.linThrust[ShipType::THRUSTER_LEFT], 0.0);
	state[ShipType::THRUSTER_RIGHT] = MAX(force.x / type.linThrust[ShipType::THRUSTER_RIGHT], 0.0);
	bool engines_not_powerful_enough = false;
	for (int i=0; i<(int)ShipType::THRUSTER_MAX; i++) {
		if (state[i] > 1.0) engines_not_powerful_enough = true;
		SetThrusterState((ShipType::Thruster)i, state[i]);
	}
	if (engines_not_powerful_enough) {
		printf("AI: Crud. thrusters insufficient: ");
		for (int i=0; i<(int)ShipType::THRUSTER_MAX; i++) printf("%f ", state[i]);
		printf("\n");
	}
}

bool Ship::AICmdFlyTo(const Body *body)
{
	vector3d bodyPos = body->GetPositionRelTo(GetFrame());
	vector3d dir = bodyPos - GetPosition();
	double dist = dir.Length() - body->GetBoundingRadius();
	vector3d relVel = GetVelocityRelativeTo(body);
	double vel = relVel.Length();

	/* done? */
	if (dist < 2.0*body->GetBoundingRadius()) {
		return true;
	}

	// work out stopping distance at current vel
	const ShipType &stype = GetShipType();
	double revAccel = stype.linThrust[ShipType::THRUSTER_FRONT] / (1000.0*m_stats.total_mass);
	double timeToStop = vel / revAccel;
	double stoppingDist = 0.5 * revAccel * timeToStop * timeToStop;
	
	ClearThrusterState();
	AISlowFaceDirection(dir.Normalized());
//	AIFaceDirection(dir.Normalized());
	
	if (stoppingDist < 0.8*dist) {
		AIAccelToModelRelativeVelocity(vector3d(0,0,-100000000000.0));
	} else if (stoppingDist > 0.9*dist) {
		AIAccelToModelRelativeVelocity(vector3d(0,0,0));
	}


	return false;
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

#include "Space.h"
bool Ship::AICmdKill(const Ship *enemy)
{
	SetGunState(0,0);
	// launch if docked
	if (GetDockedWith()) SetDockedWith(0,0);
	/* needs to deal with frames, large distances, and success */
	if (GetFrame() == enemy->GetFrame()) {
		const float dist = (enemy->GetPosition() - GetPosition()).Length();
		vector3d dir = (enemy->GetPosition() - GetPosition()).Normalized();
		ClearThrusterState();
		if (dist > 500.0) {
			AIFaceDirection(dir);
			// thunder at player at 400m/sec
			AIModelCoordsMatchSpeedRelTo(vector3d(0,0,-400), enemy);
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
		} else {
			// if too close turn away!
			AIFaceDirection(-dir);
			AIModelCoordsMatchSpeedRelTo(vector3d(0,0,-1000), enemy);
		}
	}
	return false;
}

void Ship::AIInstruct(enum AICommand cmd, void *arg)
{
	m_todo.push_back(AIInstruction(cmd, arg));
}

/* Orient so our -ve z axis == dir. ie so that dir points forwards */
void Ship::AISlowFaceDirection(const vector3d &dir)
{
	double timeAccel = Pi::GetTimeAccel();
	matrix4x4d rot;
	GetRotMatrix(rot);
	rot = rot.InverseOf();
	vector3d zaxis = vector3d(-rot[2], -rot[6], -rot[10]);
	if (timeAccel > 11.0) {
		// fake it
		zaxis = -dir;
		vector3d yaxis(rot[1], rot[5], rot[9]);
		vector3d xaxis = vector3d::Cross(yaxis, zaxis).Normalized();
		yaxis = vector3d::Cross(zaxis, xaxis);
		SetRotMatrix(matrix4x4d::MakeRotMatrix(xaxis, yaxis, zaxis).InverseOf());
	} else {
		vector3d rotaxis = vector3d::Cross(zaxis, dir);
		vector3d angVel = rot * GetAngVelocity();
		const float dot = vector3d::Dot(dir, zaxis);
		// if facing > 90 degrees away then max turn rate
		if (dot < 0) rotaxis = rotaxis.Normalized();
		rotaxis = rot*rotaxis;
		vector3d desiredAngVelChange = 4.0*(rotaxis - angVel);
		desiredAngVelChange *= 1.0 / timeAccel;
		SetAngThrusterState(0, desiredAngVelChange.x);
		SetAngThrusterState(1, desiredAngVelChange.y);
		SetAngThrusterState(2, desiredAngVelChange.z);
	}
}

/* Orient so our -ve z axis == dir. ie so that dir points forwards */
void Ship::AIFaceDirection(const vector3d &dir)
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
		float velChange = Pi::GetTimeStep() * stype.linThrust[ShipType::THRUSTER_TOP] * invMass;
		float thrust;
		if (velChange < difVel.y) thrust = 1.0;
		else thrust = difVel.y / velChange;
		thrust *= thrust;
		SetThrusterState(ShipType::THRUSTER_TOP, thrust);
	} else {
		float velChange = Pi::GetTimeStep() * stype.linThrust[ShipType::THRUSTER_BOTTOM] * invMass;
		float thrust;
		if (velChange > difVel.y) thrust = 1.0;
		else thrust = difVel.y / velChange;
		thrust *= thrust;
		SetThrusterState(ShipType::THRUSTER_BOTTOM, thrust);
	}

	if (difVel.z > 0) {
		float velChange = Pi::GetTimeStep() * stype.linThrust[ShipType::THRUSTER_FRONT] * invMass;
		float thrust;
		if (velChange < difVel.z) thrust = 1.0;
		else thrust = difVel.z / velChange;
		thrust *= thrust;
		SetThrusterState(ShipType::THRUSTER_FRONT, thrust);
	} else {
		float velChange = Pi::GetTimeStep() * stype.linThrust[ShipType::THRUSTER_REAR] * invMass;
		float thrust;
		if (velChange > difVel.z) thrust = 1.0;
		else thrust = difVel.z / velChange;
		thrust *= thrust;
		SetThrusterState(ShipType::THRUSTER_REAR, thrust);
	}
}

