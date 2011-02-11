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


// Target should be in this->GetFrame() coordinates
// If obstructor is not null then *obstructor will be set.

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

// returns false if path is clear, otherwise generates new AI path and returns it
bool Ship::AIAddAvoidancePathOnWayTo(const Body *target, AIPath &newPath)
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
	path(pos.size(), &pos[0], ourVelocity, endVelocity, maxAccel*.75, duration, newPath.path);

	int i = 0;
	for (float t=0.0; i<=10; i++, t+=0.1) {
		vector3d v = newPath.path.DerivativeOf().Eval(t) * (1.0/duration);
		//printf("%.1f: %f,%f,%f\n", t, v.x,v.y,v.z);
	}
	printf("duration %f\n", duration);

	newPath.startTime = Pi::GetGameTime();
	newPath.endTime = newPath.startTime + duration;
	newPath.frame = frame;

	return true;
}

// returns true if path is complete
bool Ship::AIFollowPath(AIPath &path, bool pointShipAtVelocityVector)
{
	const vector3d ourPosition = GetPositionRelTo(path.frame);
	const vector3d ourVelocity = GetVelocityRelativeTo(path.frame);
	double dur = path.endTime - path.startTime;
	// instead of trying to get to desired location on path curve
	// within a game tick, try adopting acceleration necessary to
	// get to desired point some fraction of remaining journey time in the
	// future, within that time. this avoids oscillation around a perfect position
	double reactionTime = Clamp<double>((path.endTime-Pi::GetGameTime())*0.01, Pi::GetTimeStep(), 200.0);
	reactionTime = std::min(reactionTime, path.endTime - Pi::GetGameTime());
	double t = (Pi::GetGameTime()+reactionTime - path.startTime) / dur;
	vector3d wantVel;
	//printf("rtime: %f t %f\n", reactionTime, t);
	if (Pi::GetGameTime()+Pi::GetTimeStep() >= path.endTime) {
		printf("end time %f, current time %f\n", path.endTime, Pi::GetGameTime());
		return true;
	} else {
		vector3d wantPos = path.path.Eval(t);
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
		Frame::GetFrameTransform(path.frame, GetFrame(), tran);
		tran.ClearToRotOnly();
		// make body-relative and apply force using thrusters
		matrix4x4d rot;
		GetRotMatrix(rot);
		force = rot.InverseOf() * tran * force;
		ClearThrusterState();

		vector3d maxThrust = GetMaxThrust(force);
		vector3d relThrust(force.x/maxThrust.x, force.y/maxThrust.y, force.z/maxThrust.z);
		SetThrusterState(relThrust);
	//	AITrySetBodyRelativeThrust(force);
		if (pointShipAtVelocityVector) {
			if (wantVel.Length()) AISlowFaceDirection((tran * wantVel).Normalized());
		} else {
			// orient so main engines can be used most effectively
			vector3d perfectForce = path.path.DerivativeOf().DerivativeOf().Eval(t);
			if (perfectForce.Length()) AISlowFaceDirection(perfectForce.Normalized());
		}
	}
	return false;
	//SetForce(force);
}



void Ship::AITrySetBodyRelativeThrust(const vector3d &force)
{
	const ShipType &type = GetShipType();

	vector3d thrust;
	if (force.x > 0.0) thrust.x = force.x / type.linThrust [ShipType::THRUSTER_RIGHT];
	else thrust.x = -force.x / type.linThrust [ShipType::THRUSTER_LEFT];
	if (force.y > 0.0) thrust.y = force.y / type.linThrust [ShipType::THRUSTER_UP];
	else thrust.y = -force.y / type.linThrust [ShipType::THRUSTER_DOWN];
	if (force.z > 0.0) thrust.z = force.z / type.linThrust [ShipType::THRUSTER_REVERSE];
	else thrust.z = -force.z / type.linThrust [ShipType::THRUSTER_FORWARD];
}

// Orient so our -ve z axis == dir. ie so that dir points forwards
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

	double ang = acos (Clamp(-head.z, -1.0, 1.0));		// scalar angle from head to curhead
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
//		double acc2;
//		if (head.x*head.x > head.y*head.y) acc2 = maxAccel * head.y / head.x;
//		else acc2 = maxAccel * head.x / head.y;
//		double maxAccDir = sqrt(maxAccel*maxAccel + acc2*acc2);		// maximum accel in desired direction
//		double iangvel = sqrt (2.0 * maxAccDir * ang);			// ideal angvel at current time

// Applies thrust directly

// Added input of target angular velocity in direction of dir-heading
// god knows whether this works for negative iangvel

void Ship::AIFaceDirection(const vector3d &dir, double av)
{
	double timeStep = Pi::GetTimeStep();
	matrix4x4d rot; GetRotMatrix(rot);
	double maxAccel = GetShipType().angThrust / GetAngularInertia();		// should probably be in stats anyway
	double frameAccel = maxAccel * timeStep;
	double invFrameAccel = 1.0 / frameAccel;

	vector3d head = dir * rot;		// create desired object-space heading
	vector3d dav(0.0, 0.0, 0.0);	// desired angular velocity

	if (head.z > -0.99999999f)
	{
		double ang = acos (Clamp(-head.z, -1.0, 1.0));		// scalar angle from head to curhead
		double iangvel = av + sqrt (2.0 * maxAccel * ang);	// ideal angvel at current time

		double frameEndAV = iangvel - frameAccel;
		if (frameEndAV <= 0.0) iangvel = ang / timeStep;	// last frame discrete correction
		else iangvel = (iangvel + frameEndAV) * 0.5;		// discrete overshoot correction

		// Normalize (head.x, head.y) to give desired angvel direction
		double head2dnorm = head.x*head.x + head.y*head.y;
		if (head2dnorm == 0.0) { head.y = 1.0; head2dnorm = 1.0; }	// NaN fix
		else head2dnorm = 1.0 / sqrt(head2dnorm);
		dav.x = head.y * head2dnorm * iangvel;
		dav.y = -head.x * head2dnorm * iangvel;
	}
	vector3d cav = GetAngVelocity() * rot;		// current obj-rel angvel
	vector3d diff = dav - cav;					// find diff between current & desired angvel

	SetAngThrusterState(diff * invFrameAccel);
}

// returns direction in world space from this ship to target lead position
vector3d Ship::AIGetLeadDir(Body *target, vector3d& targaccel, int gunindex)
{
	vector3d targpos = target->GetPositionRelTo(this);
	vector3d targvel = target->GetVelocityRelativeTo(this);
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

	vector3d difVel = v - relVel;		// required change in velocity

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

	if (!m_curAICmd) return true;
	if (m_curAICmd->TimeStepUpdate()) {
		AIClearInstructions();
		ClearThrusterState();
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

void Ship::AIJourney(SBodyPath &dest)
{
	AIClearInstructions();
	m_curAICmd = new AICmdJourney(this, dest);
}

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
	m_curAICmd = new AICmdOrbit(this, target, alt);
}
