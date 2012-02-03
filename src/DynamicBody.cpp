#include "libs.h"
#include "DynamicBody.h"
#include "Space.h"
#include "Frame.h"
#include "Serializer.h"
#include "Planet.h"
#include "Pi.h"

DynamicBody::DynamicBody(): ModelBody()
{
	m_flags = Body::FLAG_CAN_MOVE_FRAME;
	m_orient = matrix4x4d::Identity();
	m_oldOrient = m_orient;
	m_oldAngDisplacement = vector3d(0.0);
	m_force = vector3d(0.0);
	m_torque = vector3d(0.0);
	m_vel = vector3d(0.0);
	m_angVel = vector3d(0.0);
	m_mass = 1;
	m_angInertia = 1;
	m_massRadius = 1;
	m_enabled = true;
	m_atmosForce = vector3d(0.0);
	m_gravityForce = vector3d(0.0);
	m_externalForce = vector3d(0.0);		// do external forces calc instead?
	m_lastForce = vector3d(0.0);
	m_lastTorque = vector3d(0.0);
}

void DynamicBody::SetForce(const vector3d f)
{
	m_force = f;
}

void DynamicBody::AddForce(const vector3d f)
{
	m_force += f;
}

void DynamicBody::AddTorque(const vector3d t)
{
	m_torque += t;
}

void DynamicBody::AddRelForce(const vector3d f)
{
	m_force += m_orient.ApplyRotationOnly(f);
}

void DynamicBody::AddRelTorque(const vector3d t)
{
	m_torque += m_orient.ApplyRotationOnly(t);
}

void DynamicBody::Save(Serializer::Writer &wr, Space *space)
{
	ModelBody::Save(wr, space);
	for (int i=0; i<16; i++) wr.Double(m_orient[i]);
	wr.Vector3d(m_force);
	wr.Vector3d(m_torque);
	wr.Vector3d(m_vel);
	wr.Vector3d(m_angVel);
	wr.Double(m_mass);
	wr.Double(m_massRadius);
	wr.Double(m_angInertia);
	wr.Bool(m_enabled);
}

void DynamicBody::Load(Serializer::Reader &rd, Space *space)
{
	ModelBody::Load(rd, space);
	for (int i=0; i<16; i++) m_orient[i] = rd.Double();
	m_oldOrient = m_orient;
	m_force = rd.Vector3d();
	m_torque = rd.Vector3d();
	m_vel = rd.Vector3d();
	m_angVel = rd.Vector3d();
	m_mass = rd.Double();
	m_massRadius = rd.Double();
	m_angInertia = rd.Double();
	m_enabled = rd.Bool();
}

void DynamicBody::PostLoadFixup(Space *space)
{
	CalcExternalForce();
}

void DynamicBody::SetTorque(const vector3d t)
{
	m_torque = t;
}

void DynamicBody::SetMass(double mass)
{
	m_mass = mass;
	// This is solid sphere mass distribution, my friend
	m_angInertia = (2/5.0)*m_mass*m_massRadius*m_massRadius;
}

void DynamicBody::SetPosition(vector3d p)
{
	m_orient[12] = p.x;
	m_orient[13] = p.y;
	m_orient[14] = p.z;
	ModelBody::SetPosition(p);
}

vector3d DynamicBody::GetPosition() const
{
	return vector3d(m_orient[12], m_orient[13], m_orient[14]);
}

matrix4x4d DynamicBody::GetTransformRelTo(const Frame* relTo) const
{
	matrix4x4d m;
	Frame::GetFrameTransform(GetFrame(), relTo, m);
	return m * m_orient;
}

void DynamicBody::CalcExternalForce()
{
	// gravity
	Body *body = GetFrame()->GetBodyFor();
	if (body && !body->IsType(Object::SPACESTATION)) {	// they ought to have mass though...
		vector3d b1b2 = GetPosition();
		double m1m2 = GetMass() * body->GetMass();
		double invrsqr = 1.0 / b1b2.LengthSqr();
		double force = G*m1m2 * invrsqr;
		m_externalForce = -b1b2 * sqrt(invrsqr) * force;
	}
	else m_externalForce = vector3d(0.0);
	m_gravityForce = m_externalForce;

	// atmospheric drag
	const double speed = m_vel.Length();
	if ((speed > 0) && body && body->IsType(Object::PLANET))
	{
		Planet *planet = static_cast<Planet*>(body);
		double dist = GetPosition().Length();
		double pressure, density;
		planet->GetAtmosphericState(dist, &pressure, &density);
		const double radius = GetBoundingRadius();
		const double AREA = radius;
		// ^^^ yes that is as stupid as it looks
		const double DRAG_COEFF = 0.1; // 'smooth sphere'
		vector3d dragDir = -m_vel.Normalized();
		vector3d fDrag = 0.5*density*speed*speed*AREA*DRAG_COEFF*dragDir;

		// make this a bit less daft at high time accel
		// only allow atmosForce to increase by .1g per frame
		vector3d f1g = m_atmosForce + dragDir * GetMass();
		if (fDrag.LengthSqr() > f1g.LengthSqr()) m_atmosForce = f1g;
		else m_atmosForce = fDrag;

		m_externalForce += m_atmosForce;
	}

	// centrifugal and coriolis forces for rotating frames
	vector3d angRot = GetFrame()->GetAngVelocity();
	if (angRot.LengthSqr() > 0.0) {
		m_externalForce -= m_mass * angRot.Cross(angRot.Cross(GetPosition()));	// centrifugal
		m_externalForce -= 2 * m_mass * angRot.Cross(GetVelocity());			// coriolis
	}

}

void DynamicBody::TimeStepUpdate(const float timeStep)
{
	if (m_enabled) {
		m_force += m_externalForce;

		m_oldOrient = m_orient;
		m_vel += double(timeStep) * m_force * (1.0 / m_mass);
		m_angVel += double(timeStep) * m_torque * (1.0 / m_angInertia);
		// angvel is always relative to non-rotating frame, so need to counter frame angvel
		vector3d consideredAngVel = m_angVel - GetFrame()->GetAngVelocity();
		
		vector3d pos = GetPosition();
		// applying angular velocity :-/
		{
			double len = consideredAngVel.Length();
			if (len > 1e-16) {
				vector3d rotAxis = consideredAngVel * (1.0 / len);
				matrix4x4d rotMatrix = matrix4x4d::RotateMatrix(len * timeStep,
						rotAxis.x, rotAxis.y, rotAxis.z);
				m_orient = rotMatrix * m_orient;
			}
		}
		m_oldAngDisplacement = consideredAngVel * timeStep;

		pos += m_vel * double(timeStep);
		m_orient[12] = pos.x;
		m_orient[13] = pos.y;
		m_orient[14] = pos.z;
		TriMeshUpdateLastPos(m_orient);

//printf("vel = %.1f,%.1f,%.1f, force = %.1f,%.1f,%.1f, external = %.1f,%.1f,%.1f\n",
//	m_vel.x, m_vel.y, m_vel.z, m_force.x, m_force.y, m_force.z,
//	m_externalForce.x, m_externalForce.y, m_externalForce.z);

		m_lastForce = m_force;
		m_lastTorque = m_torque;
		m_force = vector3d(0.0);
		m_torque = vector3d(0.0);
		CalcExternalForce();			// regenerate for new pos/vel
	} else {
		m_oldOrient = m_orient;
		m_oldAngDisplacement = vector3d(0.0);
	}
}

// for timestep changes, to stop autopilot overshoot
// either adds half of current accel or removes all of current accel 
void DynamicBody::ApplyAccel(const float timeStep)
{
	vector3d vdiff = double(timeStep) * m_lastForce * (1.0 / m_mass);
	double spd = m_vel.LengthSqr();
	if ((m_vel-2.0*vdiff).LengthSqr() < spd) m_vel -= 2.0*vdiff;
	else if ((m_vel+vdiff).LengthSqr() < spd) m_vel += vdiff;

	vector3d avdiff = double(timeStep) * m_lastTorque * (1.0 / m_angInertia);
	double aspd = m_angVel.LengthSqr();
	if ((m_angVel-2.0*avdiff).LengthSqr() < aspd) m_angVel -= 2.0*avdiff;
	else if ((m_angVel+avdiff).LengthSqr() < aspd) m_angVel += avdiff;
}

void DynamicBody::UpdateInterpolatedTransform(double alpha)
{
	// interpolating matrices like this is a sure sign of madness
	vector3d outPos = alpha*vector3d(m_orient[12], m_orient[13], m_orient[14]) +
			(1.0-alpha)*vector3d(m_oldOrient[12], m_oldOrient[13], m_oldOrient[14]);

	m_interpolatedTransform = m_oldOrient;
	{
		double len = m_oldAngDisplacement.Length() * double(alpha);
		if (! float_is_zero_general(len)) {
			vector3d rotAxis = m_oldAngDisplacement.Normalized();
			matrix4x4d rotMatrix = matrix4x4d::RotateMatrix(len,
					rotAxis.x, rotAxis.y, rotAxis.z);
			m_interpolatedTransform = rotMatrix * m_interpolatedTransform;
		}
	}
	m_interpolatedTransform[12] = outPos.x;
	m_interpolatedTransform[13] = outPos.y;
	m_interpolatedTransform[14] = outPos.z;
}

void DynamicBody::UndoTimestep()
{
	m_orient = m_oldOrient;
	TriMeshUpdateLastPos(m_orient);
	TriMeshUpdateLastPos(m_orient);
}

void DynamicBody::Enable()
{
	ModelBody::Enable();
	m_enabled = true;
}

void DynamicBody::Disable()
{
	ModelBody::Disable();
	m_enabled = false;
}

void DynamicBody::SetRotMatrix(const matrix4x4d &r)
{
	vector3d pos = GetPosition();
	m_oldOrient = m_orient;
	m_orient = r;
	m_oldAngDisplacement = vector3d(0.0);
	SetPosition(pos);
}

void DynamicBody::GetRotMatrix(matrix4x4d &m) const
{
	m = m_orient;
	m[12] = 0;
	m[13] = 0;
	m[14] = 0;
}

void DynamicBody::SetMassDistributionFromModel()
{
	LmrCollMesh *m = GetLmrCollMesh();
	// XXX totally arbitrarily pick to distribute mass over a half
	// bounding sphere area
	m_massRadius = m->GetBoundingRadius()*0.5f;
	SetMass(m_mass);
}

vector3d DynamicBody::GetAngularMomentum() const
{
	return m_angInertia * m_angVel;
}

DynamicBody::~DynamicBody()
{
}

vector3d DynamicBody::GetAngVelocity() const
{
	return m_angVel;
}

vector3d DynamicBody::GetVelocity() const
{
	return m_vel;
}

void DynamicBody::SetVelocity(vector3d v)
{
	m_vel = v;
}

void DynamicBody::SetAngVelocity(vector3d v)
{
	m_angVel = v;
}

#define KINETIC_ENERGY_MULT	0.00001f
bool DynamicBody::OnCollision(Object *o, Uint32 flags, double relVel)
{
	// don't bother doing collision damage from a missile that will now explode, or may have already
	// also avoids an occasional race condition where destruction event of this could be queued twice
	// returning true to insure that the missile can react to the collision
	if (o->IsType(Object::MISSILE)) return true;

	double kineticEnergy = 0;
	if (o->IsType(Object::DYNAMICBODY)) {
		kineticEnergy = KINETIC_ENERGY_MULT * static_cast<DynamicBody*>(o)->GetMass() * relVel * relVel;
	} else {
		kineticEnergy = KINETIC_ENERGY_MULT * m_mass * relVel * relVel;
	}
	// damage (kineticEnergy is being passed as a damage value) is measured in kilograms
	// ignore damage less than a gram
	if (kineticEnergy > 1e-3) OnDamage(o, float(kineticEnergy));
	return true;
}
