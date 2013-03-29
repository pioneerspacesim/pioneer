// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _FRAME_H
#define _FRAME_H

#include "libs.h"
#include "Serializer.h"
#include <string>
#include <list>

class Body;
class CollisionSpace;
class Geom;
class SystemBody;
class Sfx;
class Space;

// Frame of reference.

class Frame {
public:
	enum { FLAG_ROTATING=(1<<1), FLAG_HAS_ROT=(1<<2) };

	Frame();
	Frame(Frame *parent, const char *label);
	Frame(Frame *parent, const char *label, unsigned int flags);
	~Frame();
	static void Serialize(Serializer::Writer &wr, Frame *f, Space *space);
	static void PostUnserializeFixup(Frame *f, Space *space);
	static Frame *Unserialize(Serializer::Reader &rd, Space *space, Frame *parent);
	const std::string &GetLabel() const { return m_label; }
	void SetLabel(const char *label) { m_label = label; }

	void SetPosition(const vector3d &pos) { m_pos = pos; }
	vector3d GetPosition() const { return m_pos; }
	void SetOrient(const matrix3x3d &m) { m_orient = m; }
	const matrix3x3d &GetOrient() const { return m_orient; }
	const matrix3x3d &GetInterpOrient() const { return m_interpOrient; }
	void SetVelocity(const vector3d &vel) { m_vel = vel; }
	vector3d GetVelocity() const { return m_vel; }
	void SetAngSpeed(const double angspeed) { m_angSpeed = angspeed; }
	double GetAngSpeed() const { return m_angSpeed; }
	void SetRadius(double radius) { m_radius = radius; }
	double GetRadius() const { return m_radius; }
	bool IsRotFrame() const { return m_flags & FLAG_ROTATING; }
	bool HasRotFrame() const { return m_flags & FLAG_HAS_ROT; }

	Frame *GetParent() const { return m_parent; }
	Frame *GetNonRotFrame() { return IsRotFrame() ? m_parent : this; }
	Frame *GetRotFrame() { return HasRotFrame() ? m_children.front() : this; }

	void SetBodies(SystemBody *s, Body *b) { m_sbody = s; m_astroBody = b; }
	SystemBody *GetSystemBody() const { return m_sbody; }
	Body *GetBody() const { return m_astroBody; }

	void AddChild(Frame *f) { m_children.push_back(f); }
	void RemoveChild(Frame *f);

	typedef std::vector<Frame*>::const_iterator ChildIterator;
	ChildIterator BeginChildren() const { return m_children.begin(); }
	ChildIterator EndChildren() const { return m_children.end(); }

	void AddGeom(Geom *);
	void RemoveGeom(Geom *);
	void AddStaticGeom(Geom *);
	void RemoveStaticGeom(Geom *);
	void SetPlanetGeom(double radius, Body *);
	CollisionSpace *GetCollisionSpace() const { return m_collisionSpace; }

	void UpdateOrbitRails(double time, double timestep);
	void UpdateInterpTransform(double alpha);
	void ClearMovement();

	// For an object in a rotating frame, relative to non-rotating frames it
	// must attain this velocity within rotating frame to be stationary.
	vector3d GetStasisVelocity(const vector3d &pos) const { return -vector3d(0,m_angSpeed,0).Cross(pos); }

	vector3d GetPositionRelTo(const Frame *relTo) const;
	vector3d GetVelocityRelTo(const Frame *relTo) const;
	matrix3x3d GetOrientRelTo(const Frame *relTo) const;

	// Same as above except it does interpolation between
	// physics ticks so rendering is smooth above physics hz
	vector3d GetInterpPositionRelTo(const Frame *relTo) const;
	matrix3x3d GetInterpOrientRelTo(const Frame *relTo) const;

	static void GetFrameTransform(const Frame *fFrom, const Frame *fTo, matrix4x4d &m);
	static void GetFrameRenderTransform(const Frame *fFrom, const Frame *fTo, matrix4x4d &m);

	Sfx *m_sfx;			// the last survivor. actually m_children is pretty grim too.

private:
	void Init(Frame *parent, const char *label, unsigned int flags);
	void UpdateRootRelativeVars();

	Frame *m_parent;				// if parent is null then frame position is absolute
	std::vector<Frame*> m_children;	// child frames, first may be rotating
	SystemBody *m_sbody; 			// points to SBodies in Pi::current_system
	Body *m_astroBody; 				// if frame contains a star or planet or something

	vector3d m_pos;
	vector3d m_oldPos;
	vector3d m_interpPos;
	matrix3x3d m_orient;
	matrix3x3d m_interpOrient;
	vector3d m_vel; // note we don't use this to move frame. rather,
			// orbital rails determine velocity.
	double m_angSpeed; // this however *is* directly applied (for rotating frames)
	double m_oldAngDisplacement;
	std::string m_label;
	double m_radius;
	int m_flags;
	CollisionSpace *m_collisionSpace;

	vector3d m_rootVel;			// velocity, position and orient relative to root frame
	vector3d m_rootPos;			// updated by UpdateOrbitRails
	matrix3x3d m_rootOrient;
	vector3d m_rootInterpPos;		// interp position and orient relative to root frame
	matrix3x3d m_rootInterpOrient;	// updated by UpdateInterpTransform

	int m_astroBodyIndex; // deserialisation
};

#endif /* _FRAME_H */
