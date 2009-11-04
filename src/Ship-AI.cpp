#include "libs.h"
#include "Ship.h"
#include "Pi.h"
#include "Player.h"
#include "perlin.h"

void Ship::AIBodyDeleted(const Body* const body)
{
	for (std::list<AIInstruction>::iterator i = m_todo.begin(); i != m_todo.end(); ) {
		switch ((*i).cmd) {
			case DO_KILL:
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
			Pi::RequestTimeAccel(1);
			Pi::player->SetFlightControlState(Player::CONTROL_MANUAL);
		}
	}
}

bool Ship::AICmdFlyTo(const Body *body)
{
	vector3d bodyPos = body->GetPositionRelTo(GetFrame());
	vector3d dir = bodyPos - GetPosition();
	double dist = dir.Length() - body->GetRadius();
	vector3d relVel = GetVelocityRelativeTo(body);
	double vel = relVel.Length();

	/* done? */
	if (dist < 2.0*body->GetRadius()) {
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

