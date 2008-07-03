#include "libs.h"
#include "RigidBody.h"
#include "Space.h"
#include "objimport.h"
#include "Frame.h"

RigidBody::RigidBody(): StaticRigidBody()
{
	m_flags = Body::FLAG_CAN_MOVE_FRAME;
	m_body = dBodyCreate(Space::world);
	dMassSetBox(&m_mass, 1,50,50,50);
	dMassAdjust(&m_mass, 1.0f);

	dBodySetMass(m_body, &m_mass);
}

void RigidBody::SetMassDistributionFromCollMesh(const CollMesh *m)
{
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
	dMassSetBox(&m_mass, 1, max.x-min.x, max.y-min.y, max.z-min.z);
	dBodySetMass(m_body, &m_mass);
}

vector3d RigidBody::GetAngularMomentum()
{
	matrix4x4d I;
	I.LoadFromOdeMatrix(m_mass.I);
	return I * vector3d(dBodyGetAngularVel(m_body));
}

RigidBody::~RigidBody()
{
	dBodyDestroy(m_body);
}

void RigidBody::SetVelocity(vector3d v)
{
	dBodySetLinearVel(m_body, v.x, v.y, v.z);
}

void RigidBody::SetAngVelocity(vector3d v)
{
	dBodySetAngularVel(m_body, v.x, v.y, v.z);
}
