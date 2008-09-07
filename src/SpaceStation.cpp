#include "SpaceStation.h"
#include "Ship.h"
#include "ModelCollMeshData.h"
#include "gameconsts.h"

struct SpaceStationType {
	Uint32 sbreModel;
	enum { ORBITAL, SURFACE } dockMethod;
};

struct SpaceStationType stationTypes[SpaceStation::TYPE_MAX] = {
	{ 65, SpaceStationType::ORBITAL },
	{ 90, SpaceStationType::SURFACE },
};

static ObjParams params = {
	{ 0.5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f },

	{	// pColor[3]
	{ { 1.0f, 0.0f, 1.0f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 },
	{ { 0.8f, 0.6f, 0.5f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 },
	{ { 0.5f, 0.5f, 0.5f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 } },

	// pText[3][256]	
	{ "Hello old bean", "DIET STEAKETTE" },
};

void SpaceStation::GetDockingSurface(CollMeshSet *mset, int midx)
{
	meshinfo_t *minfo = &mset->meshInfo[midx];
	assert(minfo->flags == 0x1);
	assert(minfo->numTris);
	port.center = vector3d(0.0);
	const int t = minfo->triStart;
	float *const vts = mset->sbreCollMesh->pVertex;
	for (int pos=0; pos<minfo->numTris; pos++) {
		vector3d v1(vts + 3*mset->triIndices[t+pos].v1);
		vector3d v2(vts + 3*mset->triIndices[t+pos].v2);
		vector3d v3(vts + 3*mset->triIndices[t+pos].v3);
		// use first tri to get docking port normal (which points out of the
		// docking port)
		if (pos == 0) {
			port.normal = vector3d::Cross(v2-v1,v2-v3);
			port.normal.Normalize();
			port.horiz = vector3d::Normalize(v1-v2);
		}
		port.center += v1+v2+v3;
	}
	port.center *= 1.0/(3.0*minfo->numTris);
/*	printf("Docking port center %f,%f,%f, normal %f,%f,%f, horiz %f,%f,%f\n",
		port.center.x,
		port.center.y,
		port.center.z,
		port.normal.x,
		port.normal.y,
		port.normal.z,
		port.horiz.x,
		port.horiz.y,
		port.horiz.z); */
}

SpaceStation::SpaceStation(TYPE type): ModelBody()
{
	const Uint32 sbreModel = stationTypes[type].sbreModel;
	m_type = type;
	SetModel(sbreModel);

	CollMeshSet *mset = GetModelCollMeshSet(sbreModel);
	for (unsigned int i=0; i<geomColl.size(); i++) {
		if (geomColl[i].flags == 0x1) {
			// docking surface
			GetDockingSurface(mset, i);
	//		mset->meshInfo[i]
/*struct meshinfo_t {
	int flags;
	int triStart; // into triIndices
	int numTris;
};*/

		}
	}
}

SpaceStation::~SpaceStation()
{
}

bool SpaceStation::IsGroundStation() const
{
	return (stationTypes[m_type].dockMethod ==
	        SpaceStationType::SURFACE);
}

void SpaceStation::OrientDockedShip(Ship *ship) const
{
	const int dockMethod = stationTypes[m_type].dockMethod;
	if (dockMethod == SpaceStationType::SURFACE) {
		matrix4x4d stationRot;
		GetRotMatrix(stationRot);
		vector3d port_y = vector3d::Cross(-port.horiz, port.normal);
		matrix4x4d rot = stationRot * matrix4x4d::MakeRotMatrix(-port.horiz, -port.normal, -port_y);
		vector3d pos = GetPosition() + stationRot*port.center;
		ship->SetPosition(pos - stationRot*port.normal);
		ship->SetRotMatrix(rot);
	}
}

void SpaceStation::OrientLaunchingShip(Ship *ship) const
{
	const int dockMethod = stationTypes[m_type].dockMethod;
	if (dockMethod == SpaceStationType::ORBITAL) {
		// position ship in middle of docking bay, pointing out of it
		// XXX need to do forced thrusting thingy...
		// XXX ang vel not zeroed for some reason...
		matrix4x4d stationRot;
		GetRotMatrix(stationRot);
		vector3d port_y = vector3d::Cross(-port.horiz, port.normal);
		matrix4x4d rot = stationRot * matrix4x4d::MakeRotMatrix(port.horiz, port_y, port.normal);
		vector3d pos = GetPosition() + stationRot*port.center;
		ship->SetPosition(pos);
		ship->SetRotMatrix(rot);
		ship->SetVelocity(vector3d(0,0,0));
		ship->SetAngVelocity(vector3d(0,0,0));
	}
	else if (dockMethod == SpaceStationType::SURFACE) {
		ship->Blastoff();

	/*	not necessary, since for the time being 'SURFACE' starports are on planets
	 *	so the positioning Blastoff does is fine
	 *
	 *	matrix4x4d stationRot;
		GetRotMatrix(stationRot);
		vector3d port_y = vector3d::Cross(-port.horiz, port.normal);
		matrix4x4d rot = stationRot * matrix4x4d::MakeRotMatrix(-port.horiz, -port.normal, -port_y);
		vector3d pos = GetPosition() + stationRot*port.center;
		ship->SetPosition(pos - stationRot*(10*port.normal));
		ship->SetRotMatrix(rot);
		ship->SetVelocity(vector3d(0,0,0));
		ship->SetAngVelocity(vector3d(0,0,0)); */
	} else {
		assert(0);
	}
}

bool SpaceStation::GetDockingClearance(Ship *s)
{
	s->SetDockingTimer(60*10);
	return true;
}

bool SpaceStation::OnCollision(Body *b, Uint32 flags)
{
	if (flags == 1) {
		// hitting docking area of a station
		if (b->GetType() == Object::SHIP) {
			Ship *s = static_cast<Ship*>(b);
		
			const dReal *vel = dBodyGetLinearVel(s->m_body);
			double speed = vector3d(vel[0], vel[1], vel[2]).Length();
			
			// must be oriented sensibly and have wheels down
			if (IsGroundStation()) {
				matrix4x4d rot;
				s->GetRotMatrix(rot);
				matrix4x4d invRot = rot.InverseOf();
				
				matrix4x4d stationRot;
				GetRotMatrix(stationRot);
				vector3d dockingNormal = stationRot*port.normal;

				// check player is sortof sensibly oriented for landing
				const double dot = vector3d::Dot(vector3d(-invRot[1], -invRot[5], -invRot[9]), dockingNormal);
				if ((dot < 0.99) || (s->GetWheelState() != 1.0)) return false;
			}
			
			if ((speed < MAX_LANDING_SPEED) &&
			    (!s->GetDockedWith()) &&
			    (s->GetDockingTimer()!=0.0f)) {
				s->SetDockedWith(this);
			}
		}
		return false;
	} else {
		return true;
	}
}

void SpaceStation::Render(const Frame *camFrame)
{
	RenderSbreModel(camFrame, stationTypes[m_type].sbreModel, &params);
}
