#include "SpaceStation.h"
#include "Ship.h"
#include "objimport.h"

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

SpaceStation::SpaceStation(): StaticRigidBody()
{
	SetGeomFromSBREModel(STATION_SBRE_MODEL, &params);
	matrix4x4d m = matrix4x4d::RotateYMatrix(-M_PI/4);
	dMatrix3 _m;
	m.SaveToOdeMatrix(_m);
//	dGeomSetRotation(m_geom, _m);
//	dGeomSetBody(m_geom, 0);
}

SpaceStation::~SpaceStation()
{
}

bool SpaceStation::OnCollision(Body *b)
{
	return true;

	if (b->GetType() == Object::SHIP) {
		Ship *s = static_cast<Ship*>(b);
		if (!s->GetDockedWith()) {
			s->SetDockedWith(this);
			printf("docking!\n");
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
