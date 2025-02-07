// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _MISSILE_H
#define _MISSILE_H

#include "DynamicBody.h"
#include "ShipType.h"

class AICommand;

struct MissileDef {
	StringName shipType;

	// Distance in meters within which the missile will attempt to explode on a valid target
	float fuzeRadius = 50.0;
	// How much energy does the warhead have (in kg of TNT)
	float warheadSize = 100.0;
	// What is the effective radius within which the warhead can meaningfully damage something
	float effectiveRadius = 2000.0;
	// How effective is the blast at hitting a target compared to a omnidirectional warhead?
	// defaults to 4x effectiveness, this is sufficient for most anti-ship missile explosions
	// 1.0 = omnidirectional blast
	// >1.0 = shaped charge in the direction of the target
	float chargeEffectiveness = 4.0;
	// How effective is the missile at resisting an ECM system?
	float ecmResist = 1.0;
};

class Missile : public DynamicBody {
public:
	OBJDEF(Missile, DynamicBody, MISSILE);
	Missile() = delete;
	Missile(Body *owner, const MissileDef &def);
	Missile(const Json &jsonObj, Space *space);
	virtual ~Missile();

	void Init();
	void StaticUpdate(const float timeStep) override;
	void TimeStepUpdate(const float timeStep) override;
	bool OnCollision(Body *o, Uint32 flags, double relVel) override;
	bool OnDamage(Body *attacker, float kgDamage, const CollisionContact &contactData) override;
	void NotifyRemoved(const Body *const removedBody) override;
	void PostLoadFixup(Space *space) override;
	void Render(Graphics::Renderer *r, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform) override;
	void ECMAttack(int power_val);

	Body *GetOwner() const { return m_owner; }
	const Body *GetTarget() const;

	bool IsArmed() const { return m_armed; }
	void Arm();
	void Disarm();
	void AIKamikaze(Body *target);

protected:
	void SaveToJson(Json &jsonObj, Space *space) override;

private:
	void Explode();
	bool IsValidTarget(const Body *body);

	AICommand *m_curAICmd;
	Body *m_owner;
	bool m_armed;
	const ShipType *m_type;
	Propulsion *m_propulsion;

	MissileDef m_missileStats;

	int m_ownerIndex; // deserialisation
};

#endif /* _MISSILE_H */
