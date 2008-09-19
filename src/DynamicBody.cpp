#include "libs.h"
#include "DynamicBody.h"
#include "Space.h"
#include "Frame.h"
#include "Serializer.h"

DynamicBody::DynamicBody(): ModelBody()
{
	m_flags = Body::FLAG_CAN_MOVE_FRAME;
	m_body = dBodyCreate(Space::world);
	dMassSetBox(&m_mass, 1,50,50,50);
	dMassAdjust(&m_mass, 1.0f);

	dBodySetMass(m_body, &m_mass);
}

void DynamicBody::Save()
{
	using namespace Serializer::Write;
	ModelBody::Save();
	wr_vector3d(GetAngVelocity());
}

void DynamicBody::Load()
{
	using namespace Serializer::Read;
	ModelBody::Load();
	SetAngVelocity(rd_vector3d());
}

void DynamicBody::Enable()
{
	ModelBody::Enable();
	dBodyEnable(m_body);
}

void DynamicBody::Disable()
{
	ModelBody::Disable();
	dBodyDisable(m_body);
}

void DynamicBody::SetRotMatrix(const matrix4x4d &r)
{
	dMatrix3 _m;
	r.SaveToOdeMatrix(_m);
	dBodySetRotation(m_body, _m);
}

void DynamicBody::GetRotMatrix(matrix4x4d &m)
{
	m.LoadFromOdeMatrix(dBodyGetRotation(m_body));
}

void DynamicBody::SetMassDistributionFromCollMesh(const CollMesh *m)
{
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
	float size = ((max.x-min.x) + (max.y-min.y) + (max.z-min.z)) / 6.0f;
	dMassSetSphere(&m_mass, 1, size);
	// boxes go mental after a while due to inertia tensor being fishy
//	dMassSetBox(&m_mass, 1, max.x-min.x, max.y-min.y, max.z-min.z);
	dBodySetMass(m_body, &m_mass);
}

vector3d DynamicBody::GetAngularMomentum()
{
	matrix4x4d I;
	I.LoadFromOdeMatrix(m_mass.I);
	return I * vector3d(dBodyGetAngularVel(m_body));
}

DynamicBody::~DynamicBody()
{
	dBodyDestroy(m_body);
}

vector3d DynamicBody::GetAngVelocity()
{
	return vector3d(dBodyGetAngularVel(m_body));
}

vector3d DynamicBody::GetVelocity()
{
	const dReal *v = dBodyGetLinearVel(m_body);
	return vector3d(v[0], v[1], v[2]);
}

void DynamicBody::SetVelocity(vector3d v)
{
	dBodySetLinearVel(m_body, v.x, v.y, v.z);
}

void DynamicBody::SetAngVelocity(vector3d v)
{
	dBodySetAngularVel(m_body, v.x, v.y, v.z);
}
