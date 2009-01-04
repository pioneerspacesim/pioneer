#include "libs.h"
#include "Body.h"
#include "Frame.h"
#include "Serializer.h"
#include "Star.h"
#include "Planet.h"
#include "SpaceStation.h"
#include "Ship.h"
#include "Player.h"
#include "Sfx.h"

Body::Body()
{
	m_frame = 0;
	m_flags = 0;
	m_projectedPos = vector3d(0.0f, 0.0f, 0.0f);
	m_onscreen = false;
	m_dead = false;
}

Body::~Body()
{
	// Do not call delete body. Call Space::KillBody(body).
	assert(m_dead);
}

void Body::Save()
{
	using namespace Serializer::Write;
	wr_int(Serializer::LookupFrame(m_frame));
	wr_string(m_label);
	wr_bool(m_onscreen);
	wr_vector3d(m_projectedPos);
	wr_bool(m_dead);
}

void Body::Load()
{
	using namespace Serializer::Read;
	m_frame = Serializer::LookupFrame(rd_int());
	m_label = rd_string();
	m_onscreen = rd_bool();
	m_projectedPos = rd_vector3d();
	m_dead = rd_bool();
}	

void Body::Serialize()
{
	using namespace Serializer::Write;
	wr_int((int)GetType());
	switch (GetType()) {
		case Object::STAR:
		case Object::PLANET:
		case Object::SPACESTATION:
		case Object::SHIP:
		case Object::PLAYER:
		case Object::SFX:
			Save();
			break;
		default:
			assert(0);
	}
	wr_vector3d(GetPosition());
	matrix4x4d m;
	GetRotMatrix(m);
	for (int i=0; i<16; i++) wr_double(m[i]);
}

Body *Body::Unserialize()
{
	Body *b = 0;
	using namespace Serializer::Read;
	Object::Type type = (Object::Type)rd_int();
	switch (type) {
		case Object::STAR:
			b = new Star(); break;
		case Object::PLANET:
			b = new Planet(); break;
		case Object::SPACESTATION:
			b = new SpaceStation(); break;
		case Object::SHIP:
			b = new Ship(); break;
		case Object::PLAYER:
			b = new Player(); break;
		case Object::SFX:
			b = new Sfx(); break;
		default:
			// XXX bad. should assert
			return 0;
	}
	b->Load();
	// must SetFrame() correctly so ModelBodies can add geom to space
	Frame *f = b->m_frame;
	b->m_frame = 0;
	b->SetFrame(f);
	//
	b->SetPosition(rd_vector3d());
	matrix4x4d m;
	for (int i=0; i<16; i++) m[i] = rd_double();
	b->SetRotMatrix(m);
	return b;
}

/* f == NULL, then absolute position within system */
vector3d Body::GetPositionRelTo(const Frame *relTo)
{
	matrix4x4d m;
	Frame::GetFrameTransform(m_frame, relTo, m);
	return m * GetPosition();
}

const vector3d& Body::GetProjectedPos() const
{
	assert(IsOnscreen());
	return m_projectedPos;
}

void Body::OrientOnSurface(double radius, double latitude, double longitude)
{
	vector3d pos = vector3d(radius*cos(latitude)*cos(longitude), radius*sin(latitude)*cos(longitude), radius*sin(longitude));
	vector3d up = pos.Normalized();
	SetPosition(pos);

	vector3d forward = vector3d(0,0,1);
	vector3d other = vector3d::Cross(up, forward).Normalized();
	forward = vector3d::Cross(other, up);

	matrix4x4d rot = matrix4x4d::MakeRotMatrix(other, up, forward);
	rot = rot.InverseOf();
	SetRotMatrix(rot);
}
