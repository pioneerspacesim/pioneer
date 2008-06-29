#include "SpaceStation.h"
#include "Ship.h"
#include "objimport.h"

SpaceStation::SpaceStation(): StaticRigidBody()
{
	dGeomSphereSetRadius(m_geom, 100.0);
	dGeomSetData(m_geom, static_cast<Body*>(this));
	matrix4x4d m = matrix4x4d::RotateYMatrix(M_PI);
	dMatrix3 _m;
	m.SaveToOdeMatrix(_m);
	dGeomSetRotation(m_geom, _m);
	m_mesh = 0;
}

SpaceStation::~SpaceStation()
{
//	dGeomDestroy(m_geom);
}

bool SpaceStation::OnCollision(Body *b)
{
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
void SpaceStation::SetMesh(ObjMesh *m)
{
	m_mesh = m;
}

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

void SpaceStation::Render(const Frame *camFrame)
{
	RenderSbreModel(camFrame, 65, &params);
}
