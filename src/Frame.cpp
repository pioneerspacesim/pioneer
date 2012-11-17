// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

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
	wr.Vector3d(f->m_pos);
	for (int i=0; i<9; i++) wr.Double(f->m_orient[i]);
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
	f->m_pos = rd.Vector3d();
	for (int i=0; i<9; i++) f->m_orient[i] = rd.Double();
	f->m_angVel = rd.Vector3d();
	f->m_sbody = space->GetSystemBodyByIndex(rd.Int32());
	f->m_astroBodyIndex = rd.Int32();
	f->m_vel = vector3d(0.0);
	for (int i=rd.Int32(); i>0; --i) {
		f->m_children.push_back(Unserialize(rd, space, f));
	}
	Sfx::Unserialize(rd, f);

	f->ClearMovement();
	return f;
}

void Frame::PostUnserializeFixup(Frame *f, Space *space)
{
	f->UpdateRootPosVel();
	f->m_astroBody = space->GetBodyByIndex(f->m_astroBodyIndex);
	for (std::list<Frame*>::iterator i = f->m_children.begin();
			i != f->m_children.end(); ++i) {
		PostUnserializeFixup(*i, space);
	}
}

void Frame::Init(Frame *parent, const char *label, unsigned int flags)
{
	m_sfx = 0;
	m_sbody = 0;
	m_astroBody = 0;
	m_parent = parent;
	m_flags = flags;
	m_radius = 0;
	m_pos = vector3d(0.0);
	m_vel = vector3d(0.0);
	m_angVel = vector3d(0.0);
	m_orient = matrix3x3d::Identity();
	UpdateRootPosVel();
	ClearMovement();
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


vector3d Frame::GetVelocityRelTo(const Frame *relTo) const
{
	if (this == relTo) return vector3d(0,0,0);		// these lines are not strictly necessary
	vector3d diff = m_rootVel - relTo->m_rootVel;
	if (relTo->IsRotFrame()) return diff * relTo->m_orient;
	else return diff;
}

vector3d Frame::GetPositionRelTo(const Frame *relTo) const
{
	if (this == relTo) return vector3d(0,0,0);
	vector3d diff = m_rootPos - relTo->m_rootPos;
	if (relTo->IsRotFrame()) return diff * relTo->m_orient;
	else return diff;
}

vector3d Frame::GetInterpPositionRelTo(const Frame *relTo) const
{
	if (this == relTo) return vector3d(0,0,0);
	vector3d diff = m_rootInterpPos - relTo->m_rootInterpPos;
	if (relTo->IsRotFrame()) return diff * relTo->m_orient;
	else return diff;
}

matrix3x3d Frame::GetOrientRelTo(const Frame *relTo) const
{
	if (this == relTo) return matrix3x3d::Identity();
	if (IsRotFrame()) {
		if (relTo->IsRotFrame()) return m_orient * relTo->m_orient.Transpose();
		else return m_orient;
	}
	if (relTo->IsRotFrame()) return relTo->m_orient.Transpose();
	else return matrix3x3d::Identity();
}

matrix3x3d Frame::GetInterpOrientRelTo(const Frame *relTo) const
{
	if (this == relTo) return matrix3x3d::Identity();
	if (IsRotFrame()) {
		if (relTo->IsRotFrame()) return m_interpOrient * relTo->m_interpOrient.Transpose();
		else return m_interpOrient;
	}
	if (relTo->IsRotFrame()) return relTo->m_interpOrient.Transpose();
	else return matrix3x3d::Identity();
}

void Frame::UpdateInterpTransform(double alpha)
{
	m_interpPos = alpha*m_pos + (1.0-alpha)*m_oldPos;
	m_interpOrient = m_oldOrient;

	double len = m_oldAngDisplacement.Length() * double(alpha);
	if (!is_zero_general(len)) {
		vector3d axis = m_oldAngDisplacement.Normalized();
		matrix3x3d rot = matrix3x3d::BuildRotate(len, axis);
		m_interpOrient = rot * m_interpOrient;
	}
	m_rootInterpPos = m_interpPos + m_parent->m_rootInterpPos;

	for (std::list<Frame*>::iterator i = m_children.begin(); i != m_children.end(); ++i) {
		(*i)->UpdateInterpTransform(alpha);
	}
}

void Frame::ClearMovement()
{
	m_rootInterpPos = m_rootPos;
	m_oldPos = m_interpPos = m_pos;
	m_oldOrient = m_interpOrient = m_orient;
	m_oldAngDisplacement = vector3d(0.0);
}

void Frame::UpdateOrbitRails(double time, double timestep)
{
	m_oldPos = m_pos;
	m_oldOrient = m_orient;
	m_oldAngDisplacement = m_angVel * timestep;

	// update rotation
	double ang = m_angVel.Length() * timestep;		// hmm. cumulative inaccuracy?
	if (!is_zero_general(ang)) {
		vector3d axis = m_angVel.Normalized();
		matrix3x3d rot = matrix3x3d::BuildRotate(ang, axis);
		m_orient = rot * m_orient;
	}
	UpdateRootPosVel();

	for (std::list<Frame*>::iterator i = m_children.begin(); i != m_children.end(); ++i) {
		(*i)->UpdateOrbitRails(time, timestep);
	}
}

void Frame::UpdateRootPosVel()
{
	// update pos & vel relative to parent frame
	if (!m_parent) m_rootPos = m_rootVel = vector3d(0,0,0);
	else {
		m_rootPos = m_pos + m_parent->m_rootPos;
		m_rootVel = m_vel + m_parent->m_rootVel;
	}
}