// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _BODY_H
#define _BODY_H

#include "vector3.h"
#include "matrix4x4.h"
#include "Object.h"
#include "Frame.h"
#include "Serializer.h"
#include <string>

class ObjMesh;
class Space;
class Camera;
namespace Graphics { class Renderer; }

class Body: public Object {
public:
	OBJDEF(Body, Object, BODY);
	Body();
	virtual ~Body();
	void Serialize(Serializer::Writer &wr, Space *space);
	static Body *Unserialize(Serializer::Reader &rd, Space *space);
	virtual void PostLoadFixup(Space *space) {};

	virtual void SetPosition(const vector3d &p) { m_pos = p; }
	vector3d GetPosition() const { return m_pos; }
	virtual void SetOrient(const matrix3x3d &r) { m_orient = r; }
	const matrix3x3d &GetOrient() const { return m_orient; }
	virtual void SetVelocity(const vector3d &v) { assert(0); }
	virtual vector3d GetVelocity() const { return vector3d(0.0); }

	void SetPhysRadius(double r) { m_physRadius = r; }
	double GetPhysRadius() const { return m_physRadius; }
	void SetClipRadius(double r) { m_clipRadius = r; }
	double GetClipRadius() const { return m_clipRadius; }
	virtual double GetMass() const { assert(0); return 0; }
	
	// return true if to do collision response and apply damage
	virtual bool OnCollision(Object *o, Uint32 flags, double relVel) { return false; }
	// Attacker may be null
	virtual bool OnDamage(Object *attacker, float kgDamage) { return false; }
	// Override to clear any pointers you hold to the body
	virtual void NotifyRemoved(const Body* const removedBody) {}

	// before all bodies have had TimeStepUpdate (their moving step),
	// StaticUpdate() is called. Good for special collision testing (Projectiles)
	// as you can't test for collisions if different objects are on different 'steps'
	virtual void StaticUpdate(const float timeStep) {}
	virtual void TimeStepUpdate(const float timeStep) {}
	virtual void Render(Graphics::Renderer *r, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform) = 0;

	virtual void SetFrame(Frame *f) { m_frame = f; }
	Frame *GetFrame() const { return m_frame; }
	void SwitchToFrame(Frame *newFrame);
	void UpdateFrame();				// check for frame switching

	vector3d GetVelocityRelTo(const Body *) const;
	vector3d GetVelocityRelTo(const Frame *) const;
	vector3d GetPositionRelTo(const Frame *) const;
	vector3d GetPositionRelTo(const Body *) const;
	matrix3x3d GetOrientRelTo(const Frame *) const;

	// Should return pointer in Pi::currentSystem
	virtual const SystemBody *GetSystemBody() const { return 0; }
	// for putting on planet surface, oriented +y up
	void OrientOnSurface(double radius, double latitude, double longitude);

	void SetLabel(const std::string &label) { m_label = label; }
	const std::string &GetLabel() const { return m_label; }
	unsigned int GetFlags() const { return m_flags; }
	// Only Space::KillBody() should call this method.
	void MarkDead() { m_dead = true; }
	bool IsDead() const { return m_dead; }

	// all Bodies are in space... except where they're not (Ships hidden in hyperspace clouds)
	virtual bool IsInSpace() const { return true; }

	// Interpolated between physics ticks.
	const matrix3x3d &GetInterpOrient() const { return m_interpOrient; }
	vector3d GetInterpPosition() const { return m_interpPos; }
	vector3d GetInterpPositionRelTo(const Frame *relTo) const;
	vector3d GetInterpPositionRelTo(const Body *relTo) const;
	matrix3x3d GetInterpOrientRelTo(const Frame *relTo) const;

	// should set m_interpolatedTransform to the smoothly interpolated value
	// (interpolated by 0 <= alpha <=1) between the previous and current physics tick
	virtual void UpdateInterpTransform(double alpha) {
		m_interpOrient = GetOrient();
		m_interpPos = GetPosition();
	}

	// where to draw targeting indicators - usually equal to GetInterpolatedPositionRelTo
	virtual vector3d GetTargetIndicatorPosition(const Frame *relTo) const;

	enum { FLAG_CAN_MOVE_FRAME = (1<<0),
			FLAG_LABEL_HIDDEN = (1<<1),
			FLAG_DRAW_LAST = (1<<2) };		// causes the body drawn after other bodies in the z-sort

protected:
	virtual void Save(Serializer::Writer &wr, Space *space);
	virtual void Load(Serializer::Reader &rd, Space *space);
	unsigned int m_flags;

	// Interpolated draw orientation-position
	vector3d m_interpPos;
	matrix3x3d m_interpOrient;
private:
	vector3d m_pos;
	matrix3x3d m_orient;
	Frame *m_frame;				// frame of reference
	std::string m_label;
	bool m_dead;				// Checked in destructor to make sure body has been marked dead.
	double m_clipRadius;
	double m_physRadius;
};

#endif /* _BODY_H */
