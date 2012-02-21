#ifndef _BODY_H
#define _BODY_H

#include "vector3.h"
#include "matrix4x4.h"
#include "Object.h"
#include "Serializer.h"
#include <string>

class Frame;
class ObjMesh;
class Space;
namespace Graphics { class Renderer; }

class Body: public Object {
public:
	OBJDEF(Body, Object, BODY);
	Body();
	virtual ~Body();
	void Serialize(Serializer::Writer &wr, Space *space);
	static Body *Unserialize(Serializer::Reader &rd, Space *space);
	virtual void PostLoadFixup(Space *space) {};

	virtual void SetPosition(vector3d p) = 0;
	virtual vector3d GetPosition() const = 0; // within frame
	virtual void SetVelocity(vector3d v) { assert(0); }
	virtual vector3d GetVelocity() const { return vector3d(0.0); }
	virtual double GetBoundingRadius() const = 0;
	virtual double GetClipRadius() const { return GetBoundingRadius(); }
	virtual double GetMass() const { assert(0); return 0; }
	virtual void SetRotMatrix(const matrix4x4d &r) {};
	virtual void GetRotMatrix(matrix4x4d &m) const { };

	// return true if to do collision response and apply damage
	virtual bool OnCollision(Object *o, Uint32 flags, double relVel) { return false; }
	// Attacker may be null
	virtual bool OnDamage(Object *attacker, float kgDamage) { return false; }
	virtual void OnHaveKilled(Body *guyWeKilled) {}
	// Note: Does not mean killed, just deleted.
	// Override to clear any pointers you hold to the body
	virtual void NotifyRemoved(const Body* const removedBody) {}

	// before all bodies have had TimeStepUpdate (their moving step),
	// StaticUpdate() is called. Good for special collision testing (Projectiles)
	// as you can't test for collisions if different objects are on different 'steps'
	virtual void StaticUpdate(const float timeStep) {}
	virtual void TimeStepUpdate(const float timeStep) {}
	virtual void Render(Graphics::Renderer *r, const vector3d &viewCoords, const matrix4x4d &viewTransform) = 0;

	virtual void SetFrame(Frame *f) { m_frame = f; }
	Frame *GetFrame() const { return m_frame; }
	void UpdateFrame();				// check for frame switching
	bool HasDoubleFrame() const { return m_hasDoubleFrame; }
	vector3d GetVelocityRelTo(const Body *other) const;
	vector3d GetVelocityRelTo(const Frame *f) const;
	vector3d GetPositionRelTo(const Frame *) const;
	vector3d GetPositionRelTo(const Body *) const;
	
	// Should return pointer in Pi::currentSystem
	virtual const SBody *GetSBody() const { return 0; }
	// for putting on planet surface, oriented +y up
	void OrientOnSurface(double radius, double latitude, double longitude);

	void SetLabel(const char *label) { m_label = label; }
	const std::string &GetLabel() const { return m_label; }
	unsigned int GetFlags() { return m_flags; }
	// Only Space::KillBody() should call this method.
	void MarkDead() { m_dead = true; }
	bool IsDead() const { return m_dead; }

	// Interpolated between physics ticks.
	const matrix4x4d &GetInterpolatedTransform() const { return m_interpolatedTransform; }
	vector3d GetInterpolatedPosition() const {
		return vector3d(m_interpolatedTransform[12], m_interpolatedTransform[13], m_interpolatedTransform[14]);
	}
	vector3d GetInterpolatedPositionRelTo(const Frame *relTo) const;
	vector3d GetInterpolatedPositionRelTo(const Body *relTo) const;
	matrix4x4d GetInterpolatedTransformRelTo(const Frame *relTo) const;
	// should set m_interpolatedTransform to the smoothly interpolated
	// value (interpolated by 0 <= alpha <=1) between the previous and current
	//  physics tick
	virtual void UpdateInterpolatedTransform(double alpha) {
		const vector3d pos = GetPosition();
		m_interpolatedTransform = matrix4x4d::Identity();
		m_interpolatedTransform[12] = pos.x;
		m_interpolatedTransform[13] = pos.y;
		m_interpolatedTransform[14] = pos.z;
	}

	// where to draw targeting indicators - usually equal to GetInterpolatedPositionRelTo
	virtual vector3d GetTargetIndicatorPosition(const Frame *relTo) const;

	enum { FLAG_CAN_MOVE_FRAME = (1<<0),
               FLAG_LABEL_HIDDEN = (1<<1),
	       FLAG_DRAW_LAST = (1<<2) }; // causes the body drawn after other bodies in the z-sort

protected:
	virtual void Save(Serializer::Writer &wr, Space *space);
	virtual void Load(Serializer::Reader &rd, Space *space);
	unsigned int m_flags;
	bool m_hasDoubleFrame;

	// Interpolated draw orientation-position
	matrix4x4d m_interpolatedTransform;
private:
	// frame of reference
	Frame *m_frame;
	std::string m_label;
	// Checked in destructor to make sure body has been marked dead.
	bool m_dead;
};

#endif /* _BODY_H */
