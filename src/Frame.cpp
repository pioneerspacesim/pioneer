#include "Frame.h"
#include "Space.h"
#include "Serializer.h"
#include "collider/collider.h"

Frame::Frame()
{
	Init(NULL, "", 0);
}

Frame::Frame(Frame *parent, const char *label)
{
	Init(parent, label, 0);
}

Frame::Frame(Frame *parent, const char *label, unsigned int flags)
{
	Init(parent, label, flags);
}

void Frame::Serialize(Frame *f)
{
	using namespace Serializer::Write;
	wr_int(f->m_flags);
	wr_double(f->m_radius);
	wr_string(f->m_label);
	for (int i=0; i<16; i++) wr_double(f->m_orient[i]);
	wr_vector3d(f->m_angVel);
	wr_vector3d(f->m_pos);
	wr_int(Serializer::LookupSystemBody(f->m_sbody));
	wr_int(Serializer::LookupBody(f->m_astroBody));
	wr_int(f->m_children.size());
	for (std::list<Frame*>::iterator i = f->m_children.begin();
			i != f->m_children.end(); ++i) {
		Serialize(*i);
	}
}

Frame *Frame::Unserialize(Frame *parent)
{
	using namespace Serializer::Read;
	Frame *f = new Frame();
	f->m_parent = parent;
	f->m_flags = rd_int();
	f->m_radius = rd_double();
	f->m_label = rd_string();
	for (int i=0; i<16; i++) f->m_orient[i] = rd_double();
	f->m_angVel = rd_vector3d();
	f->m_pos = rd_vector3d();
	f->m_sbody = Serializer::LookupSystemBody(rd_int());
	f->m_astroBody = (Body*)rd_int();
	f->m_vel = vector3d(0.0);
	for (int i=rd_int(); i>0; --i) {
		f->m_children.push_back(Unserialize(f));
	}
	
	return f;
}

void Frame::PostUnserializeFixup(Frame *f)
{
	f->m_astroBody = Serializer::LookupBody((size_t)f->m_astroBody);
	for (std::list<Frame*>::iterator i = f->m_children.begin();
			i != f->m_children.end(); ++i) {
		PostUnserializeFixup(*i);
	}
}

void Frame::RemoveChild(Frame *f)
{
	m_children.remove(f);
}

void Frame::Init(Frame *parent, const char *label, unsigned int flags)
{
	m_sbody = 0;
	m_astroBody = 0;
	m_parent = parent;
	m_flags = flags;
	m_radius = 0;
	m_pos = vector3d(0.0f);
	m_vel = vector3d(0.0);
	m_angVel = vector3d(0.0);
	m_orient = matrix4x4d::Identity();
	m_collisionSpace = new CollisionSpace();
	if (m_parent) {
		m_parent->m_children.push_back(this);
	}
	if (label) m_label = label;
}

Frame::~Frame()
{
	delete m_collisionSpace;
	for (std::list<Frame*>::iterator i = m_children.begin(); i != m_children.end(); ++i) delete *i;
}

void Frame::AddGeom(Geom *g) { m_collisionSpace->AddGeom(g); }
void Frame::RemoveGeom(Geom *g) { m_collisionSpace->RemoveGeom(g); }
void Frame::SetPlanetGeom(double radius, Body *obj)
{
	m_collisionSpace->SetSphere(vector3d(0,0,0), radius, static_cast<void*>(obj));
}

void Frame::ApplyLeavingTransform(matrix4x4d &m) const
{
	m = matrix4x4d::Translation(m_pos) * m_orient * m;
}

void Frame::ApplyEnteringTransform(matrix4x4d &m) const
{
	m = m * m_orient.InverseOf() * matrix4x4d::Translation(-m_pos);
}

void Frame::GetFrameTransform(const Frame *fFrom, const Frame *fTo, matrix4x4d &m)
{
	matrix4x4d m2 = matrix4x4d::Identity();
	m = matrix4x4d::Identity();

	const Frame *f = fFrom;
	const Frame *root = Space::GetRootFrame();

	while ((f!=root) && (fTo != f)) {
		f->ApplyLeavingTransform(m);
		f = f->m_parent;
	}

	while (fTo != f) {
		fTo->ApplyEnteringTransform(m2);
		fTo = fTo->m_parent;
	}

	m = m2 * m;
}
	
void Frame::RotateInTimestep(double step)
{
	double ang = m_angVel.Length() * step;
	if (ang == 0) return;
	vector3d rotAxis = vector3d::Normalize(m_angVel);
	matrix4x4d rotMatrix = matrix4x4d::RotateMatrix(ang, rotAxis.x, rotAxis.y, rotAxis.z);

	m_orient = rotMatrix * m_orient;
}

/*
 * For an object in a rotating frame, relative to non-rotating frames it
 * must attain this velocity within rotating frame to be stationary.
 */
vector3d Frame::GetStasisVelocityAtPosition(const vector3d &pos) const
{
	const double omega = m_angVel.Length();
	if (omega) {
		vector3d perpend = vector3d::Cross(m_angVel, pos);
		perpend = vector3d::Normalize(vector3d::Cross(perpend, m_angVel));
		double R = vector3d::Dot(perpend, pos);
		perpend *= R;
		return -vector3d::Cross(m_angVel, perpend);
	} else {
		return vector3d(0,0,0);
	}
}
