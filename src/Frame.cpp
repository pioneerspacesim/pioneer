#include "Frame.h"
#include "Body.h"
#include "Space.h"
#include "collider/collider.h"
#include "Sfx.h"
#include "galaxy/StarSystem.h"
#include "Pi.h"
#include "Game.h"

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

void Frame::Serialize(Serializer::Writer &wr, Frame *f, Space *space)
{
	wr.Int32(f->m_flags);
	wr.Double(f->m_radius);
	wr.String(f->m_label);
	for (int i=0; i<16; i++) wr.Double(f->m_orient[i]);
	wr.Vector3d(f->m_angVel);
	wr.Int32(space->GetIndexForSystemBody(f->m_sbody));
	wr.Int32(space->GetIndexForBody(f->m_astroBody));
	wr.Int32(f->m_children.size());
	for (std::list<Frame*>::iterator i = f->m_children.begin();
			i != f->m_children.end(); ++i) {
		Serialize(wr, *i, space);
	}
	Sfx::Serialize(wr, f);
}

Frame *Frame::Unserialize(Serializer::Reader &rd, Space *space, Frame *parent)
{
	Frame *f = new Frame();
	f->m_parent = parent;
	f->m_flags = rd.Int32();
	f->m_radius = rd.Double();
	f->m_label = rd.String();
	for (int i=0; i<16; i++) f->m_orient[i] = rd.Double();
	f->m_angVel = rd.Vector3d();
	if (rd.StreamVersion() < 20) {
		vector3d pos = rd.Vector3d();
		f->m_orient.SetTranslate(pos);
	}
	f->m_sbody = space->GetSystemBodyByIndex(rd.Int32());
	f->m_astroBodyIndex = rd.Int32();
	f->m_vel = vector3d(0.0);
	for (int i=rd.Int32(); i>0; --i) {
		f->m_children.push_back(Unserialize(rd, space, f));
	}
	Sfx::Unserialize(rd, f);

	f->m_oldOrient = f->m_orient;
	f->m_oldAngDisplacement = vector3d(0.0);
	
	return f;
}

void Frame::PostUnserializeFixup(Frame *f, Space *space)
{
	f->m_astroBody = space->GetBodyByIndex(f->m_astroBodyIndex);
	for (std::list<Frame*>::iterator i = f->m_children.begin();
			i != f->m_children.end(); ++i) {
		PostUnserializeFixup(*i, space);
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
	m_vel = vector3d(0.0);
	m_angVel = vector3d(0.0);
	m_orient = matrix4x4d::Identity();
	m_oldOrient = matrix4x4d::Identity();
	m_oldAngDisplacement = vector3d(0.0);
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
	m = m_orient * m;
}

void Frame::ApplyEnteringTransform(matrix4x4d &m) const
{
	m = m * m_orient.InverseOf();
}

vector3d Frame::GetFrameRelativeVelocity(const Frame *fFrom, const Frame *fTo)
{
	if (fFrom == fTo) return vector3d(0,0,0);
	vector3d v1 = vector3d(0,0,0);
	vector3d v2 = vector3d(0,0,0);
	
	matrix4x4d m = matrix4x4d::Identity();

	const Frame *f = fFrom;

	// move forwards from origin to root
	while (f->m_parent && fTo != f) {
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

	while (f->m_parent && fTo != f) {
		f->ApplyLeavingTransform(m);
		f = f->m_parent;
	}

	while (fTo != f) {
		fTo->ApplyEnteringTransform(m2);
		fTo = fTo->m_parent;
	}

	m = m2 * m;
}
	
void Frame::GetFrameRenderTransform(const Frame *fFrom, const Frame *fTo, matrix4x4d &m)
{
	matrix4x4d m2 = matrix4x4d::Identity();
	m = matrix4x4d::Identity();

	const Frame *f = fFrom;

	while (f->m_parent && fTo != f) {
		m = f->m_interpolatedTransform * m;
		f = f->m_parent;
	}

	while (fTo != f) {
		m2 = m2 * fTo->m_interpolatedTransform.InverseOf();
		fTo = fTo->m_parent;
	}

	m = m2 * m;
}
	
void Frame::RotateInTimestep(double step)
{
	double ang = m_angVel.Length() * step;
	if (is_zero_general(ang)) return;
	vector3d rotAxis = m_angVel.Normalized();
	matrix4x4d rotMatrix = matrix4x4d::RotateMatrix(ang, rotAxis.x, rotAxis.y, rotAxis.z);

	const vector3d pos = m_orient.GetTranslate();
	m_orient = m_orient * rotMatrix;
	m_orient.SetTranslate(pos);
}

// For an object in a rotating frame, relative to non-rotating frames it
// must attain this velocity within rotating frame to be stationary.

vector3d Frame::GetStasisVelocityAtPosition(const vector3d &pos) const
{
	return -m_angVel.Cross(pos);
}

bool Frame::IsStationRotFrame() const
{
	return (m_astroBody && m_astroBody->IsType(Object::SPACESTATION));
}

// Find system body this frame is for.

SystemBody *Frame::GetSystemBodyFor() const
{
	if (m_sbody) return m_sbody;
	if (m_parent) return m_parent->m_sbody; // rotating frame of planet
	else return 0;
}

// Find body this frame is for

Body *Frame::GetBodyFor() const
{
	if (m_astroBody) return m_astroBody;
	if (m_sbody && m_sbody->type != SystemBody::TYPE_GRAVPOINT && !m_children.empty())
		return (*m_children.begin())->m_astroBody;
	return 0;
}

void Frame::UpdateInterpolatedTransform(double alpha)
{
	vector3d outPos = alpha*vector3d(m_orient[12], m_orient[13], m_orient[14]) +
			(1.0-alpha)*vector3d(m_oldOrient[12], m_oldOrient[13], m_oldOrient[14]);

	m_interpolatedTransform = m_oldOrient;
	{
		double len = m_oldAngDisplacement.Length() * double(alpha);
		if (! is_zero_general(len)) {
			vector3d rotAxis = m_oldAngDisplacement.Normalized();
			matrix4x4d rotMatrix = matrix4x4d::RotateMatrix(len,
					rotAxis.x, rotAxis.y, rotAxis.z);
			m_interpolatedTransform = rotMatrix * m_interpolatedTransform;
		}
	}
	m_interpolatedTransform[12] = outPos.x;
	m_interpolatedTransform[13] = outPos.y;
	m_interpolatedTransform[14] = outPos.z;
	
	for (std::list<Frame*>::iterator i = m_children.begin(); i != m_children.end(); ++i) {
		(*i)->UpdateInterpolatedTransform(alpha);
	}

}

void Frame::UpdateOrbitRails(double time, double timestep)
{
	m_oldOrient = m_orient;
	m_oldAngDisplacement = m_angVel * timestep;
	if (!m_parent) {
		m_orient = matrix4x4d::Identity();
	} else if (m_sbody) {
		// this isn't very smegging efficient
		vector3d pos = m_sbody->orbit.OrbitalPosAtTime(time);
		vector3d pos2 = m_sbody->orbit.OrbitalPosAtTime(time+1.0);
		vector3d vel = pos2 - pos;
		SetPosition(pos);
		SetVelocity(vel);
	}
	RotateInTimestep(timestep);

	for (std::list<Frame*>::iterator i = m_children.begin(); i != m_children.end(); ++i) {
		(*i)->UpdateOrbitRails(time, timestep);
	}
}

