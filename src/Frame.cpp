#include "Frame.h"
#include "Space.h"
#include "collider/collider.h"
#include "Sfx.h"

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

void Frame::Serialize(Serializer::Writer &wr, Frame *f)
{
	wr.Int32(f->m_flags);
	wr.Double(f->m_radius);
	wr.String(f->m_label);
	for (int i=0; i<16; i++) wr.Double(f->m_orient[i]);
	wr.Vector3d(f->m_angVel);
	wr.Vector3d(f->m_pos);
	wr.Int32(Serializer::LookupSystemBody(f->m_sbody));
	wr.Int32(Serializer::LookupBody(f->m_astroBody));
	wr.Int32(f->m_children.size());
	for (std::list<Frame*>::iterator i = f->m_children.begin();
			i != f->m_children.end(); ++i) {
		Serialize(wr, *i);
	}
	Sfx::Serialize(wr, f);
}

Frame *Frame::Unserialize(Serializer::Reader &rd, Frame *parent)
{
	Frame *f = new Frame();
	f->m_parent = parent;
	f->m_flags = rd.Int32();
	f->m_radius = rd.Double();
	f->m_label = rd.String();
	for (int i=0; i<16; i++) f->m_orient[i] = rd.Double();
	f->m_angVel = rd.Vector3d();
	f->m_pos = rd.Vector3d();
	f->m_sbody = Serializer::LookupSystemBody(rd.Int32());
	f->m_astroBody = (Body*)rd.Int32();
	f->m_vel = vector3d(0.0);
	for (int i=rd.Int32(); i>0; --i) {
		f->m_children.push_back(Unserialize(rd, f));
	}
	Sfx::Unserialize(rd, f);
	
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
	m_sfx = 0;
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
	if (m_sfx) delete [] m_sfx;
	delete m_collisionSpace;
	for (std::list<Frame*>::iterator i = m_children.begin(); i != m_children.end(); ++i) delete *i;
}

void Frame::AddGeom(Geom *g) { m_collisionSpace->AddGeom(g); }
void Frame::RemoveGeom(Geom *g) { m_collisionSpace->RemoveGeom(g); }
void Frame::AddStaticGeom(Geom *g) { m_collisionSpace->AddStaticGeom(g); }
void Frame::RemoveStaticGeom(Geom *g) { m_collisionSpace->RemoveStaticGeom(g); }
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

vector3d Frame::GetFrameRelativeVelocity(const Frame *fFrom, const Frame *fTo)
{
	if (fFrom == fTo) return vector3d(0,0,0);
	vector3d v1 = vector3d(0,0,0);
	vector3d v2 = vector3d(0,0,0);
	
	matrix4x4d m = matrix4x4d::Identity();

	const Frame *f = fFrom;
	const Frame *root = Space::rootFrame;

	// move forwards from origin to root
	while ((f!=root) && (fTo != f)) {
		v1 += m.ApplyRotationOnly(-f->GetVelocity());
		m = m * f->m_orient.InverseOf();
		f = f->m_parent;
	}

	// move backwards from target to root
	while (fTo != f) {
		v2 = fTo->m_orient.ApplyRotationOnly(fTo->GetVelocity() + v2);
		fTo = fTo->m_parent;
	}

	vector3d out = v1 + m.ApplyRotationOnly(v2);
//	printf("v1: %.2f,%.2f,%.2f  v2: %.2f,%.2f,%.2f ~= %.2f,%.2f,%.2f\n", v1.x, v1.y, v1.z, v2.x, v2.y, v2.z, out.x, out.y, out.z);
	return out;
}

void Frame::GetFrameTransform(const Frame *fFrom, const Frame *fTo, matrix4x4d &m)
{
	matrix4x4d m2 = matrix4x4d::Identity();
	m = matrix4x4d::Identity();

	const Frame *f = fFrom;
	const Frame *root = Space::rootFrame;

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
	vector3d rotAxis = m_angVel.Normalized();
	matrix4x4d rotMatrix = matrix4x4d::RotateMatrix(ang, rotAxis.x, rotAxis.y, rotAxis.z);

	m_orient = m_orient * rotMatrix;
}

/*
 * For an object in a rotating frame, relative to non-rotating frames it
 * must attain this velocity within rotating frame to be stationary.
 */
vector3d Frame::GetStasisVelocityAtPosition(const vector3d &pos) const
{
	const double omega = m_angVel.Length();
	vector3d vzero(0,0,0);
	if (omega) {
		vector3d perpend = vector3d::Cross(m_angVel, pos);
		if (perpend == vzero) return vzero;
		perpend = vector3d::Cross(perpend, m_angVel).Normalized();
		double R = vector3d::Dot(perpend, pos);
		perpend *= R;
		return -vector3d::Cross(m_angVel, perpend);
	} else {
		return vzero;
	}
}

/*
 * Find system body this frame is for.
 */
SBody *Frame::GetSBodyFor()
{
	if (m_sbody) return m_sbody;
	if (m_parent) return m_parent->m_sbody; // rotating frame of planet
	else return 0;
}
