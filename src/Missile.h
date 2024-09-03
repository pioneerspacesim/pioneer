// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _MISSILE_H
#define _MISSILE_H

#include "DynamicBody.h"
#include "ShipType.h"

class AICommand;

class Missile : public DynamicBody {
public:
	OBJDEF(Missile, DynamicBody, MISSILE);
	Missile() = delete;
	Missile(const ShipType::Id &type, Body *owner, int power = -1);
	Missile(const Json &jsonObj, Space *space);
	virtual ~Missile();
	void StaticUpdate(const float timeStep) override;
	void TimeStepUpdate(const float timeStep) override;
	virtual bool OnCollision(Body *o, Uint32 flags, double relVel) override;
	virtual bool OnDamage(Body *attacker, float kgDamage, const CollisionContact &contactData) override;
	virtual void NotifyRemoved(const Body *const removedBody) override;
	virtual void PostLoadFixup(Space *space) override;
	virtual void Render(Graphics::Renderer *r, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform) override;
	void ECMAttack(int power_val);

	Body *GetOwner() const { return m_owner; }
	const Body *GetTarget() const;

	bool IsArmed() const { return m_armed; }
	void Arm();
	void Disarm();
	void AIKamikaze(Body *target);

protected:
	virtual void SaveToJson(Json &jsonObj, Space *space) override;

private:
	void Explode();
	bool IsValidTarget(const Body *body);

	AICommand *m_curAICmd;
	int m_power;
	Body *m_owner;
	bool m_armed;
	const ShipType *m_type;
	Propulsion *m_propulsion;

	int m_ownerIndex; // deserialisation
};

#endif /* _MISSILE_H */
