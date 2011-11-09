#ifndef _MISSILE_H
#define _MISSILE_H

#include <list>
#include "libs.h"
#include "Ship.h"

class Missile: public Ship {
public:
	OBJDEF(Missile, Ship, MISSILE);
	Missile(ShipType::Type type, Body *owner, Body *target);
	Missile() {}
	virtual ~Missile() {}
	void TimeStepUpdate(const float timeStep);
	virtual bool OnCollision(Object *o, Uint32 flags, double relVel);
	virtual bool OnDamage(Object *attacker, float kgDamage);
	virtual void NotifyDeleted(const Body* const deletedBody);
	virtual void PostLoadFixup();
	void ECMAttack(int power_val);
	Body *GetOwner() const { return m_owner; }
protected:
	virtual void Save(Serializer::Writer &wr);
	virtual void Load(Serializer::Reader &rd);
private:
	void Explode();

	int m_power;
	Body *m_target;
	Body *m_owner;
	double m_distToTarget;

	int m_ownerIndex, m_targetIndex; // deserialisation
};

#endif /* _MISSILE_H */
