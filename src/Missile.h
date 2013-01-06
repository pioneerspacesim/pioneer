// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _MISSILE_H
#define _MISSILE_H

#include <list>
#include "libs.h"
#include "Ship.h"

class Missile: public Ship {
public:
	OBJDEF(Missile, Ship, MISSILE);
	Missile(ShipType::Id shipId, Body *owner, Body *target);
	Missile() {}
	virtual ~Missile() {}
	void TimeStepUpdate(const float timeStep);
	virtual bool OnCollision(Object *o, Uint32 flags, double relVel);
	virtual bool OnDamage(Object *attacker, float kgDamage);
	virtual void NotifyRemoved(const Body* const removedBody);
	virtual void PostLoadFixup(Space *space);
	void ECMAttack(int power_val);
	Body *GetOwner() const { return m_owner; }
protected:
	virtual void Save(Serializer::Writer &wr, Space *space);
	virtual void Load(Serializer::Reader &rd, Space *space);
private:
	void Explode();

	int m_power;
	Body *m_target;
	Body *m_owner;
	double m_distToTarget;

	int m_ownerIndex, m_targetIndex; // deserialisation
};

#endif /* _MISSILE_H */
