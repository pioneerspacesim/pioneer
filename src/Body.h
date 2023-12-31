// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _BODY_H
#define _BODY_H

#include "BodyComponent.h"
#include "DeleteEmitter.h"
#include "FrameId.h"
#include "lua/PropertiedObject.h"
#include "matrix3x3.h"
#include "matrix4x4.h"
#include "vector3.h"
#include <string>

class Space;
class Camera;
class Frame;
class SystemBody;

namespace Graphics {
	class Renderer;
}
struct CollisionContact;

// ObjectType is used as a form of RTTI for Body and its children.
// Think carefully before adding more entries; we'd like to switch
// to a composition-based system instead.
enum class ObjectType { // <enum name=PhysicsObjectType scope='ObjectType' public>
	// only creating enum strings for types that are exposed to Lua
	BODY,
	MODELBODY,
	DYNAMICBODY, // <enum skip>
	SHIP,
	PLAYER,
	SPACESTATION,
	TERRAINBODY, // <enum skip>
	PLANET,
	STAR,
	CARGOBODY,
	PROJECTILE, // <enum skip>
	MISSILE,
	HYPERSPACECLOUD // <enum skip>
};

enum class AltitudeType { // <enum name=AltitudeType scope='AltitudeType' public>
	//SEA_LEVEL if distant, ABOVE_TERRAIN otherwise
	DEFAULT,
	SEA_LEVEL,
	ABOVE_TERRAIN
};

#define OBJDEF(__thisClass, __parentClass, __TYPE)                                  \
	static constexpr ObjectType StaticType() { return ObjectType::__TYPE; }         \
	static constexpr ObjectType SuperType() { return __parentClass::StaticType(); } \
	virtual ObjectType GetType() const override { return ObjectType::__TYPE; }      \
	virtual bool IsType(ObjectType c) const override                                \
	{                                                                               \
		if (__thisClass::GetType() == (c))                                          \
			return true;                                                            \
		else                                                                        \
			return __parentClass::IsType(c);                                        \
	}

class Body : public DeleteEmitter, public PropertiedObject {
public:
	static constexpr ObjectType StaticType() { return ObjectType::BODY; }
	virtual ObjectType GetType() const { return ObjectType::BODY; }
	virtual bool IsType(ObjectType c) const { return GetType() == c; }

	Body();
	Body(const Json &jsonObj, Space *space);
	virtual ~Body();
	void ToJson(Json &jsonObj, Space *space);
	static Body *FromJson(const Json &jsonObj, Space *space);
	virtual void PostLoadFixup(Space *space){};

	virtual void SetPosition(const vector3d &p) { m_pos = p; }
	vector3d GetPosition() const { return m_pos; }
	virtual void SetOrient(const matrix3x3d &r) { m_orient = r; }
	const matrix3x3d &GetOrient() const { return m_orient; }
	virtual void SetVelocity(const vector3d &v) { assert(0); }
	virtual vector3d GetVelocity() const { return vector3d(0.0); }
	virtual void SetAngVelocity(const vector3d &v) { assert(0); }
	virtual vector3d GetAngVelocity() const { return vector3d(0.0); }

	void SetPhysRadius(double r) { m_physRadius = r; }
	double GetPhysRadius() const { return m_physRadius; }
	void SetClipRadius(double r) { m_clipRadius = r; }
	double GetClipRadius() const { return m_clipRadius; }
	virtual double GetMass() const
	{
		assert(0);
		return 0;
	}

	// return true if to do collision response and apply damage
	virtual bool OnCollision(Body *o, Uint32 flags, double relVel) { return false; }
	// Attacker may be null
	virtual bool OnDamage(Body *attacker, float kgDamage, const CollisionContact &contactData) { return false; }
	// Override to clear any pointers you hold to the body
	virtual void NotifyRemoved(const Body *const removedBody) {}

	// before all bodies have had TimeStepUpdate (their moving step),
	// StaticUpdate() is called. Good for special collision testing (Projectiles)
	// as you can't test for collisions if different objects are on different 'steps'
	virtual void StaticUpdate(const float timeStep) {}
	virtual void TimeStepUpdate(const float timeStep) {}
	virtual void Render(Graphics::Renderer *r, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform) = 0;

	virtual void SetFrame(FrameId f) { m_frame = f; }
	FrameId GetFrame() const { return m_frame; }
	void SwitchToFrame(FrameId newFrame);
	void UpdateFrame(); // check for frame switching

	vector3d GetVelocityRelTo(const Body *) const;
	vector3d GetVelocityRelTo(FrameId) const;
	vector3d GetPositionRelTo(FrameId) const;
	vector3d GetPositionRelTo(const Body *) const;
	matrix3x3d GetOrientRelTo(FrameId) const;

	// Should return pointer in Pi::currentSystem
	virtual const SystemBody *GetSystemBody() const { return nullptr; }
	// for putting on planet surface, oriented +y up
	void OrientOnSurface(double radius, double latitude, double longitude);

	virtual void SetLabel(const std::string &label);
	const std::string &GetLabel() const { return m_label; }

	unsigned int GetFlags() const { return m_flags; }
	// TODO(sturnclaw) use this sparingly, the flags interface is rather fragile and needs work
	void SetFlag(unsigned int flag, bool enable)
	{
		if (enable)
			m_flags |= flag;
		else
			m_flags &= ~flag;
	}

	// Check if a specific component is present. This involves a lookup through std::map
	// so it's not quite as efficient as it should be.
	template <typename T>
	bool HasComponent() const
	{
		return m_components & (uint64_t(1) << uint8_t(BodyComponentDB::GetComponentType<T>()->componentIndex));
	}

	// Return a pointer to the component of type T attached to this instance or nullptr.
	// This returns a non-const pointer for simplicity as the component is technically
	// not part of the object.
	template <typename T>
	T *GetComponent() const
	{
		auto *type = BodyComponentDB::GetComponentType<T>();
		return m_components & (uint64_t(1) << uint8_t(type->componentIndex)) ? type->get(this) : nullptr;
	}

	template <typename T>
	T *AddComponent()
	{
		auto *type = BodyComponentDB::GetComponentType<T>();
		if (m_components & (uint64_t(1) << uint8_t(type->componentIndex)))
			return type->get(this);

		m_components |= (uint64_t(1) << uint8_t(type->componentIndex));
		return type->newComponent(this);
	}

	// Returns the bitset of components attached to this body. Prefer using HasComponent<> or GetComponent<> instead.
	uint64_t GetComponentList() const { return m_components; }

	// Only Space::KillBody() should call this method.
	void MarkDead() { m_dead = true; }
	bool IsDead() const { return m_dead; }

	// all Bodies are in space... except where they're not (Ships hidden in hyperspace clouds)
	virtual bool IsInSpace() const { return true; }

	// Interpolated between physics ticks.
	const matrix3x3d &GetInterpOrient() const { return m_interpOrient; }
	vector3d GetInterpPosition() const { return m_interpPos; }
	vector3d GetInterpPositionRelTo(FrameId relToId) const;
	vector3d GetInterpPositionRelTo(const Body *relTo) const;
	matrix3x3d GetInterpOrientRelTo(FrameId relToId) const;
	double GetAltitudeRelTo(const Body* relTo, AltitudeType altType = AltitudeType::DEFAULT);

	// should set m_interpolatedTransform to the smoothly interpolated value
	// (interpolated by 0 <= alpha <=1) between the previous and current physics tick
	virtual void UpdateInterpTransform(double alpha)
	{
		m_interpOrient = GetOrient();
		m_interpPos = GetPosition();
	}

	// TODO: abstract this functionality into a component of some fashion
	// Return the position in body-local coordinates where the target indicator should be displayed.
	// Usually equal to the center of the body == vector3d(0, 0, 0)
	virtual vector3d GetTargetIndicatorPosition() const;

	enum {
		FLAG_CAN_MOVE_FRAME = (1 << 0),
		FLAG_LABEL_HIDDEN = (1 << 1),
		FLAG_DRAW_LAST = (1 << 2),	 // causes the body drawn after other bodies in the z-sort
		FLAG_DRAW_EXCLUDE = (1 << 3) // do not draw this body, intended for e.g. when camera is inside
	};

private:
	uint64_t m_components = 0;

protected:
	virtual void SaveToJson(Json &jsonObj, Space *space);
	unsigned int m_flags = 0;

	// Interpolated draw orientation-position
	vector3d m_interpPos;
	matrix3x3d m_interpOrient;

private:
	vector3d m_pos;
	matrix3x3d m_orient;
	FrameId m_frame; // frame of reference
	std::string m_label;
	bool m_dead; // Checked in destructor to make sure body has been marked dead.
	double m_clipRadius;
	double m_physRadius;
};

#endif /* _BODY_H */
