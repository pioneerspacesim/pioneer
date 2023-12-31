// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _FRAME_H
#define _FRAME_H

#include "FrameId.h"

#include "IterationProxy.h"
#include "JsonFwd.h"
#include "matrix3x3.h"
#include "matrix4x4.h"
#include "vector3.h"
#include <list>
#include <string>

class Body;
class CollisionSpace;
class Geom;
class SystemBody;
class SfxManager;
class Space;

struct CollisionContact;

// Frame of reference.

class Frame {
	// Used to avoid direct instantiation of Frames: use factory methods instead.
	// TODO: Find a better way, as checking it at compile time
	struct Dummy;

public:
	Frame() = delete;
	Frame(const Dummy &d, FrameId parent, const char *label, unsigned int flags = FLAG_DEFAULT, double radius = 0.0);
	// Used *only* for Camera frame:
	// it doesn't set up CollisionSpace, and use default values for label, flags and radius
	Frame(const Dummy &d, FrameId parent);

	Frame(const Frame &) = delete;
	Frame(Frame &&) noexcept;
	Frame &operator=(Frame &&);

	~Frame();

	enum { FLAG_DEFAULT = (0),
		FLAG_ROTATING = (1 << 1),
		FLAG_HAS_ROT = (1 << 2) };

	static FrameId CreateFrame(FrameId parent, const char *label, unsigned int flags = FLAG_DEFAULT, double radius = 0.0);
	static FrameId FromJson(const Json &jsonObj, Space *space, FrameId parent, double at_time);

	// Used to speed up creation/deletion of Frame for camera
	static FrameId CreateCameraFrame(FrameId parent);
	static void DeleteCameraFrame(FrameId camera);

	static void ToJson(Json &jsonObj, FrameId fId, Space *space);
	static void PostUnserializeFixup(FrameId fId, Space *space);

	static void DeleteFrames();

	static Frame *GetFrame(FrameId FId);

	FrameId GetId() const { return m_thisId; }

	const std::string &GetLabel() const { return m_label; }
	void SetLabel(const char *label) { m_label = label; }

	void SetPosition(const vector3d &pos) { m_pos = pos; }
	vector3d GetPosition() const { return m_pos; }
	void SetInitialOrient(const matrix3x3d &m, double time);
	void SetOrient(const matrix3x3d &m, double time);
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

	FrameId GetParent() const { return m_parent; }
	FrameId GetNonRotFrame() const { return IsRotFrame() ? m_parent : m_thisId; }
	FrameId GetRotFrame() const { return HasRotFrame() ? m_children.front() : m_thisId; }

	void SetBodies(SystemBody *s, Body *b)
	{
		m_sbody = s;
		m_astroBody = b;
	}
	SystemBody *GetSystemBody() const { return m_sbody; }
	Body *GetBody() const { return m_astroBody; }

	void AddChild(FrameId fId) { m_children.push_back(fId); }
	void RemoveChild(FrameId fId);
	bool HasChildren() const { return !m_children.empty(); }
	unsigned GetNumChildren() const { return static_cast<Uint32>(m_children.size()); }
	IterationProxy<std::vector<FrameId>> GetChildren() { return MakeIterationProxy(m_children); }
	const IterationProxy<const std::vector<FrameId>> GetChildren() const { return MakeIterationProxy(m_children); }

	void AddGeom(Geom *);
	void RemoveGeom(Geom *);
	void AddStaticGeom(Geom *);
	void RemoveStaticGeom(Geom *);
	// TODO: Should be a Planet or there's a needs for a Body?
	void SetPlanetGeom(double radius, Body *);
	CollisionSpace *GetCollisionSpace() const;

	static void UpdateOrbitRails(double time, double timestep);
	static void CollideFrames(void (*callback)(CollisionContact *));
	void UpdateInterpTransform(double alpha);
	void ClearMovement();

	// For an object in a rotating frame, relative to non-rotating frames it
	// must attain this velocity within rotating frame to be stationary.
	vector3d GetStasisVelocity(const vector3d &pos) const { return -vector3d(0, m_angSpeed, 0).Cross(pos); }

	vector3d GetPositionRelTo(FrameId relTo) const;
	vector3d GetVelocityRelTo(FrameId relTo) const;
	matrix3x3d GetOrientRelTo(FrameId relTo) const;
	matrix4x4d GetTransformRelTo(FrameId relTo) const;

	// Same as above except it does interpolation between
	// physics ticks so rendering is smooth above physics hz
	vector3d GetInterpPositionRelTo(FrameId relTo) const;
	matrix3x3d GetInterpOrientRelTo(FrameId relTo) const;
	matrix4x4d GetInterpTransformRelTo(FrameId relTo) const;

	static void GetFrameTransform(FrameId fFrom, FrameId fTo, matrix4x4d &m);

	std::unique_ptr<SfxManager> m_sfx; // the last survivor. actually m_children is pretty grim too.

private:
	FrameId m_thisId;

	void UpdateRootRelativeVars();

	FrameId m_parent;				 // if parent is null then frame position is absolute
	std::vector<FrameId> m_children; // child frames, first may be rotating
	SystemBody *m_sbody;			 // points to SBodies in Pi::current_system
	Body *m_astroBody;				 // if frame contains a star or planet or something

	vector3d m_pos;
	vector3d m_oldPos;
	vector3d m_interpPos;
	matrix3x3d m_initialOrient;
	matrix3x3d m_orient;
	matrix3x3d m_interpOrient;
	vector3d m_vel;	   // note we don't use this to move frame. rather,
					   // orbital rails determine velocity.
	double m_angSpeed; // this however *is* directly applied (for rotating frames)
	double m_oldAngDisplacement;
	std::string m_label;
	double m_radius;
	int m_flags;
	std::unique_ptr<CollisionSpace> m_collisionSpace;

	vector3d m_rootVel; // velocity, position and orient relative to root frame
	vector3d m_rootPos; // updated by UpdateOrbitRails
	matrix3x3d m_rootOrient;
	vector3d m_rootInterpPos;	   // interp position and orient relative to root frame
	matrix3x3d m_rootInterpOrient; // updated by UpdateInterpTransform

	int m_astroBodyIndex; // deserialisation

	static std::vector<Frame> s_frames;
	static std::vector<CollisionSpace *> s_collisionSpaces;

	// A trick in order to avoid a direct call of ctor or dtor: use factory methods instead
	struct Dummy {
		Dummy() :
			madeWithFactory(false)
		{
		}
		bool madeWithFactory;
	};

	Dummy d;
};

#endif /* _FRAME_H */
