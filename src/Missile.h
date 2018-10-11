// Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _MISSILE_H
#define _MISSILE_H

#include <list>
#include "libs.h"
#include "DynamicBody.h"
#include "ShipAICmd.h"

class Missile: public DynamicBody {
public:
	OBJDEF(Missile, DynamicBody, MISSILE);
	Missile(const ShipType::Id &type, Body *owner, int power=-1);
	Missile() {}
	virtual ~Missile();
	void StaticUpdate(const float timeStep) override;
	void TimeStepUpdate(const float timeStep) override;
	virtual bool OnCollision(Object *o, Uint32 flags, double relVel) override;
	virtual bool OnDamage(Object *attacker, float kgDamage, const CollisionContact& contactData) override;
	virtual void NotifyRemoved(const Body* const removedBody) override;
	virtual void PostLoadFixup(Space *space) override;
	virtual void Render(Graphics::Renderer *r, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform) override;
	void ECMAttack(int power_val);
	Body *GetOwner() const { return m_owner; }
	bool IsArmed() const {return m_armed;}
	void Arm();
	void Disarm();
	void AIKamikaze(Body *target);

protected:
	virtual void SaveToJson(Json &jsonObj, Space *space) override;
	virtual void LoadFromJson(const Json &jsonObj, Space *space) override;
private:
	void Explode();
	AICommand *m_curAICmd;
	int m_power;
	Body *m_owner;
	bool m_armed;
	const ShipType *m_type;

	int m_ownerIndex; // deserialisation
};

#endif /* _MISSILE_H */
