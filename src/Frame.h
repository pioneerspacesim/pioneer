#ifndef _FRAME_H
#define _FRAME_H

#include "libs.h"
#include "Serializer.h"
#include <string>
#include <list>

class Body;
class CollisionSpace;
class Geom;
class SBody;
class Sfx;
class Space;

/*
 * Frame of reference.
 */
class Frame {
public:
	Frame();
	Frame(Frame *parent, const char *label);
	Frame(Frame *parent, const char *label, unsigned int flags);
	~Frame();
	static void Serialize(Serializer::Writer &wr, Frame *f, Space *space);
	static void PostUnserializeFixup(Frame *f, Space *space);
	static Frame *Unserialize(Serializer::Reader &rd, Space *space, Frame *parent);
	// XXX this should return a std::string
	const char *GetLabel() const { return m_label.c_str(); }
	void SetLabel(const char *label) { m_label = label; }
	void SetPosition(const vector3d &pos) { m_orient.SetTranslate(pos); }
	vector3d GetPosition() const { return m_orient.GetTranslate(); }
	void SetRotationOnly(const matrix4x4d &m) { for (int i=0; i<12; i++) m_orient[i] = m[i]; }
	void SetTransform(const matrix4x4d &m) { m_orient = m; }
	const matrix4x4d &GetTransform() const { return m_orient; }
	void SetVelocity(const vector3d &vel) { m_vel = vel; }
	vector3d GetVelocity() const { return m_vel; }
	void SetAngVelocity(const vector3d &angvel) { m_angVel = angvel; }
	vector3d GetAngVelocity() const { return m_angVel; }
	vector3d GetStasisVelocityAtPosition(const vector3d &pos) const;
	void SetRadius(double radius) { m_radius = radius; }
	double GetRadius() const { return m_radius; }
	void RemoveChild(Frame *f);
	void AddGeom(Geom *);
	void RemoveGeom(Geom *);
	void AddStaticGeom(Geom *);
	void RemoveStaticGeom(Geom *);
	void SetPlanetGeom(double radius, Body *);
	CollisionSpace *GetCollisionSpace() const { return m_collisionSpace; }
	void RotateInTimestep(double step);
	bool IsRotatingFrame() const { return !is_zero_general(m_angVel.Length()); }
	bool IsStationRotFrame() const;
	// snoops into parent frames so beware
	SBody *GetSBodyFor() const;
	Body *GetBodyFor() const;
	void UpdateOrbitRails(double time, double timestep);

	void ApplyLeavingTransform(matrix4x4d &m) const;
	void ApplyEnteringTransform(matrix4x4d &m) const;

	static void GetFrameTransform(const Frame *fFrom, const Frame *fTo, matrix4x4d &m);
	static vector3d GetFrameRelativeVelocity(const Frame *fFrom, const Frame *fTo);
	/** Same as GetFrameTransform except it does interpolation between
	  * physics ticks so rendering is smooth above physics hz */
	static void GetFrameRenderTransform(const Frame *fFrom, const Frame *fTo, matrix4x4d &m);
	void UpdateInterpolatedTransform(double alpha);
	void ClearMovement() {
		m_oldOrient = m_interpolatedTransform = m_orient;
		m_oldAngDisplacement = vector3d(0.0);
	}


	bool IsLocalPosInFrame(const vector3d &pos) {
		return (pos.Length() < m_radius);
	}
	/* if parent is null then frame position is absolute */
	Frame *m_parent;
	std::list<Frame*> m_children;
	SBody *m_sbody; // points to SBodies in Pi::current_system
	Body *m_astroBody; // if frame contains a star or planet or something
	Sfx *m_sfx;
	
	enum { TEMP_VIEWING=1 };
private:
	void Init(Frame *parent, const char *label, unsigned int flags);
	vector3d m_vel; // note we don't use this to move frame. rather,
			// orbital rails determine velocity.
	vector3d m_angVel; // this however *is* directly applied (for rotating frames)
	vector3d m_oldAngDisplacement;
	matrix4x4d m_orient;
	matrix4x4d m_oldOrient;
	matrix4x4d m_interpolatedTransform;
	std::string m_label;
	double m_radius;
	int m_flags;
	CollisionSpace *m_collisionSpace;

	int m_astroBodyIndex; // deserialisation
};

#endif /* _FRAME_H */
