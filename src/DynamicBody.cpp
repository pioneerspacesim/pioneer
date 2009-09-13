#include "libs.h"
#include "DynamicBody.h"
#include "Space.h"
#include "Frame.h"
#include "Serializer.h"
#include "ModelCollMeshData.h"

DynamicBody::DynamicBody(): ModelBody()
{
	m_flags = Body::FLAG_CAN_MOVE_FRAME;
	m_orient = matrix4x4d::Identity();
	m_oldOrient = m_orient;
	m_force = vector3d(0.0);
	m_torque = vector3d(0.0);
	m_vel = vector3d(0.0);
	m_angVel = vector3d(0.0);
	m_mass = 1;
	m_angInertia = 1;
	m_massRadius = 1;
	m_enabled = true;
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

void DynamicBody::Save()
{
	using namespace Serializer::Write;
	ModelBody::Save();
	for (int i=0; i<16; i++) wr_double(m_orient[i]);
	wr_vector3d(m_force);
	wr_vector3d(m_torque);
	wr_vector3d(m_vel);
	wr_vector3d(m_angVel);
	wr_double(m_mass);
	wr_double(m_massRadius);
	wr_double(m_angInertia);
	wr_bool(m_enabled);
}

void DynamicBody::Load()
{
	using namespace Serializer::Read;
	ModelBody::Load();
	for (int i=0; i<16; i++) m_orient[i] = rd_double();
	m_oldOrient = m_orient;
	m_force = rd_vector3d();
	m_torque = rd_vector3d();
	m_vel = rd_vector3d();
	m_angVel = rd_vector3d();
	m_mass = rd_double();
	m_massRadius = rd_double();
	m_angInertia = rd_double();
	m_enabled = rd_bool();
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

void DynamicBody::TimeStepUpdate(const float timeStep)
{
	if (m_enabled) {
		/* This shit is for rotating frames. It is a bit smelly */
		vector3d angRot = GetFrame()->GetAngVelocity();
		{
			double omega = angRot.Length();
			if (omega) {
			
				// centrifugal force
				vector3d perpend = vector3d::Cross(angRot, GetPosition());
				if (perpend != vector3d(0,0,0)) {
					perpend = vector3d::Cross(perpend, angRot).Normalized();
					double R = vector3d::Dot(perpend, GetPosition());
					double centrifugal = m_mass * omega * omega * R;
					m_force += centrifugal*perpend;
					// coriolis force
					m_force += -2*m_mass*vector3d::Cross(angRot, GetVelocity());
				}
			}

		}

		m_oldOrient = m_orient;
		m_vel += (double)timeStep * m_force * (1.0 / m_mass);
		m_angVel += (double)timeStep * m_torque * (1.0 / m_angInertia);
		vector3d consideredAngVel = m_angVel - angRot;
		
		vector3d pos = GetPosition();
		// applying angular velocity :-/
		{
			double len = consideredAngVel.Length();
			if (len != 0) {
				vector3d rotAxis = consideredAngVel * (1.0 / len);
				matrix4x4d rotMatrix = matrix4x4d::RotateMatrix(len * timeStep,
						rotAxis.x, rotAxis.y, rotAxis.z);
				m_orient = rotMatrix * m_orient;
			}
		}

		pos += m_vel * (double)timeStep;
		m_orient[12] = pos.x;
		m_orient[13] = pos.y;
		m_orient[14] = pos.z;
		TriMeshUpdateLastPos(m_orient);

		m_force = vector3d(0.0);
		m_torque = vector3d(0.0);
	}
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
	m_orient = r;
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
	const CollMesh *m = GetModelSBRECollMesh(GetSbreModel());
	// XXX this is stupid. the radius of mass distribution should be
	// defined on the model, not cooked up in some moronic way
	vector3d min = vector3d(FLT_MAX);
	vector3d max = vector3d(-FLT_MAX);
	for (int i=0; i<3*m->nv; i+=3) {
		min.x = MIN(m->pVertex[i], min.x);
		min.y = MIN(m->pVertex[i+1], min.y);
		min.z = MIN(m->pVertex[i+2], min.z);
		max.x = MAX(m->pVertex[i], max.x);
		max.y = MAX(m->pVertex[i+1], max.y);
		max.z = MAX(m->pVertex[i+2], max.z);
	}
	const vector3d size = max-min;
	m_massRadius = (size.x + size.y + size.z) * 0.333;
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
bool DynamicBody::OnCollision(Body *b, Uint32 flags, double relVel)
{
	float kineticEnergy = 0;
	if (b->IsType(Object::DYNAMICBODY)) {
		kineticEnergy = KINETIC_ENERGY_MULT * m_mass * relVel * relVel;
	} else {
		const float v = (float)GetVelocity().Length();
		kineticEnergy = KINETIC_ENERGY_MULT * m_mass * relVel * relVel;
	}
	if (kineticEnergy) OnDamage(b, kineticEnergy);
	return true;
}
