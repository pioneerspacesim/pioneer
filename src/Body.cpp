#include "libs.h"
#include "Body.h"
#include "Frame.h"
#include "Star.h"
#include "Planet.h"
#include "CargoBody.h"
#include "SpaceStation.h"
#include "Ship.h"
#include "Player.h"
#include "Projectile.h"
#include "Missile.h"
#include "HyperspaceCloud.h"
#include "Pi.h"

Body::Body()
{
	m_frame = 0;
	m_flags = 0;
	m_hasDoubleFrame = false;
	m_projectedPos = vector3d(0.0f, 0.0f, 0.0f);
	m_onscreen = false;
	m_dead = false;
}

Body::~Body()
{
}

void Body::Save(Serializer::Writer &wr)
{
	wr.Int32(Serializer::LookupFrame(m_frame));
	wr.String(m_label);
	wr.Bool(m_onscreen);
	wr.Vector3d(m_projectedPos);
	wr.Bool(m_dead);
	wr.Bool(m_hasDoubleFrame);
}

void Body::Load(Serializer::Reader &rd)
{
	m_frame = Serializer::LookupFrame(rd.Int32());
	m_label = rd.String();
	m_onscreen = rd.Bool();
	m_projectedPos = rd.Vector3d();
	m_dead = rd.Bool();
	m_hasDoubleFrame = rd.Bool();
}	

void Body::Serialize(Serializer::Writer &_wr)
{
	Serializer::Writer wr;
	wr.Int32(int(GetType()));
	switch (GetType()) {
		case Object::STAR:
		case Object::PLANET:
		case Object::SPACESTATION:
		case Object::SHIP:
		case Object::PLAYER:
		case Object::MISSILE:
		case Object::CARGOBODY:
		case Object::PROJECTILE:
		case Object::HYPERSPACECLOUD:
			Save(wr);
			break;
		default:
			assert(0);
	}
	wr.Vector3d(GetPosition());
	matrix4x4d m;
	GetRotMatrix(m);
	for (int i=0; i<16; i++) wr.Double(m[i]);
	_wr.WrSection("Body", wr.GetData());
}

Body *Body::Unserialize(Serializer::Reader &_rd)
{
	Serializer::Reader rd = _rd.RdSection("Body");
	Body *b = 0;
	Object::Type type = Object::Type(rd.Int32());
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
		case Object::MISSILE:
			b = new Missile(); break;
		case Object::PROJECTILE:
			b = new Projectile(); break;
		case Object::CARGOBODY:
			b = new CargoBody(); break;
		case Object::HYPERSPACECLOUD:
			b = new HyperspaceCloud(); break;
		default:
			assert(0);
	}
	b->Load(rd);
	// must SetFrame() correctly so ModelBodies can add geom to space
	Frame *f = b->m_frame;
	b->m_frame = 0;
	b->SetFrame(f);
	//
	b->SetPosition(rd.Vector3d());
	matrix4x4d m;
	for (int i=0; i<16; i++) m[i] = rd.Double();
	b->SetRotMatrix(m);
	return b;
}

/* f == NULL, then absolute position within system */
vector3d Body::GetPositionRelTo(const Frame *relTo) const
{
	matrix4x4d m;
	Frame::GetFrameTransform(m_frame, relTo, m);
	return m * GetPosition();
}

vector3d Body::GetInterpolatedPositionRelTo(const Frame *relTo) const
{
	matrix4x4d m;
	Frame::GetFrameRenderTransform(m_frame, relTo, m);
	return m * GetInterpolatedPosition();
}

vector3d Body::GetPositionRelTo(const Body *relTo) const
{
	return GetPositionRelTo(relTo->GetFrame()) - relTo->GetPosition();
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
	vector3d other = up.Cross(forward).Normalized();
	forward = other.Cross(up);

	matrix4x4d rot = matrix4x4d::MakeRotMatrix(other, up, forward);
	rot = rot.InverseOf();
	SetRotMatrix(rot);
}

vector3d Body::GetVelocityRelTo(const Frame *f) const
{
	matrix4x4d m;
	Frame::GetFrameTransform(GetFrame(), f, m);
	vector3d vel = GetVelocity();
	if (f != GetFrame() || f->IsStationRotFrame())
		vel -= GetFrame()->GetStasisVelocityAtPosition(GetPosition());
	return (m.ApplyRotationOnly(vel) + Frame::GetFrameRelativeVelocity(f, GetFrame()));
}

vector3d Body::GetVelocityRelTo(const Body *other) const
{
	return GetVelocityRelTo(GetFrame()) - other->GetVelocityRelTo(GetFrame());
}

void Body::UpdateFrame()
{
	if (!(GetFlags() & Body::FLAG_CAN_MOVE_FRAME)) return;

	// falling out of frames
	if (!GetFrame()->IsLocalPosInFrame(GetPosition())) {
		printf("%s leaves frame %s\n", GetLabel().c_str(), GetFrame()->GetLabel());

		Frame *new_frame = GetFrame()->m_parent;
		if (new_frame) { // don't let fall out of root frame
			matrix4x4d m = matrix4x4d::Identity();
			GetFrame()->ApplyLeavingTransform(m);

			vector3d new_pos = m * GetPosition();

			matrix4x4d rot;
			GetRotMatrix(rot);
			SetRotMatrix(m * rot);
			
			m.ClearToRotOnly();
			SetVelocity(GetFrame()->GetVelocity() + m*(GetVelocity() - 
				GetFrame()->GetStasisVelocityAtPosition(GetPosition())));

			SetFrame(new_frame);
			SetPosition(new_pos);

			Pi::luaOnFrameChanged.Queue(this);
			
			return;
		}
	}

	// entering into frames
	for (std::list<Frame*>::iterator j = GetFrame()->m_children.begin(); j != GetFrame()->m_children.end(); ++j) {
		Frame *kid = *j;
		matrix4x4d m;
		Frame::GetFrameTransform(GetFrame(), kid, m);
		vector3d pos = m * GetPosition();
		if (!kid->IsLocalPosInFrame(pos)) continue;
		
		printf("%s enters frame %s\n", GetLabel().c_str(), kid->GetLabel());

		SetPosition(pos);
		SetFrame(kid);

		matrix4x4d rot;
		GetRotMatrix(rot);
		SetRotMatrix(m * rot);
				
		// get rid of transforms
		m.ClearToRotOnly();
		SetVelocity(m*(GetVelocity() - kid->GetVelocity())
			+ kid->GetStasisVelocityAtPosition(pos));

		Pi::luaOnFrameChanged.Queue(this);

		break;
	}
}

