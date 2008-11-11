#ifndef _FRAME_H
#define _FRAME_H

#include "libs.h"
#include <string>
#include <list>
#include "StarSystem.h"

class Body;
class CollisionSpace;
class Geom;

/*
 * Frame of reference.
 */
class Frame {
public:
	Frame();
	Frame(Frame *parent, const char *label);
	Frame(Frame *parent, const char *label, unsigned int flags);
	~Frame();
	static void Serialize(Frame *);
	static void PostUnserializeFixup(Frame *f);
	static Frame *Unserialize(Frame *parent);
	const char *GetLabel() const { return m_label.c_str(); }
	void SetLabel(const char *label) { m_label = label; }
	void SetPosition(const vector3d &pos) { m_pos = pos; }
	vector3d GetPosition() const { return m_pos; }
	void SetVelocity(const vector3d &vel) { m_vel = vel; }
	vector3d GetVelocity() const { return m_vel; }
	void SetAngVelocity(const vector3d &angvel) { m_angVel = angvel; }
	vector3d GetAngVelocity() const { return m_angVel; }
	const matrix4x4d &GetOrientation() const { return m_orient; }
	void SetOrientation(const matrix4x4d &m) { m_orient = m; }
	void SetRadius(double radius) { m_radius = radius; }
	void RemoveChild(Frame *f);
	void AddGeom(Geom *);
	void RemoveGeom(Geom *);
	void SetPlanetGeom(double radius, Body *);
	CollisionSpace *GetCollisionSpace() const { return m_collisionSpace; }
	void RotateInTimestep(double step);

	void ApplyLeavingTransform(matrix4x4d &m) const;
	void ApplyEnteringTransform(matrix4x4d &m) const;

	static void GetFrameTransform(const Frame *fFrom, const Frame *fTo, matrix4x4d &m);

	bool IsLocalPosInFrame(const vector3d &pos) {
		return (pos.Length() < m_radius);
	}
	/* if parent is null then frame position is absolute */
	Frame *m_parent;
	std::list<Frame*> m_children;
	StarSystem::SBody *m_sbody; // points to SBodies in Pi::current_system
	Body *m_astroBody; // if frame contains a star or planet or something
	
	enum { TEMP_VIEWING=1 };
private:
	void Init(Frame *parent, const char *label, unsigned int flags);
	vector3d m_pos;
	vector3d m_vel; // note we don't use this to move frame. rather,
			// orbital rails determine velocity.
	vector3d m_angVel; // this however *is* directly applied (for rotating frames)
	matrix4x4d m_orient;
	std::string m_label;
	double m_radius;
	int m_flags;
	CollisionSpace *m_collisionSpace;
};

#endif /* _FRAME_H */
