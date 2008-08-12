#include "SpaceStation.h"
#include "Ship.h"
#include "ModelCollMeshData.h"

#define STATION_SBRE_MODEL	65

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

SpaceStation::SpaceStation(): ModelBody()
{
	SetModel(STATION_SBRE_MODEL);
	matrix4x4d m = matrix4x4d::RotateYMatrix(M_PI-M_PI/6);
	SetRotation(m);

	CollMeshSet *mset = GetModelCollMeshSet(STATION_SBRE_MODEL);
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
			if ((!s->GetDockedWith()) && (s->GetDockingTimer()!=0.0f)) {
				s->Disable();
				s->SetDockedWith(this);
				printf("docking!\n");
			}
		}
		return false;
	} else {
		return true;
	}
}

void SpaceStation::Render(const Frame *camFrame)
{
	RenderSbreModel(camFrame, STATION_SBRE_MODEL, &params);
}
