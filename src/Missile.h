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
	bool OnDamage(Object *attacker, float kgDamage);
	virtual void NotifyDeleted(const Body* const deletedBody);
	virtual void PostLoadFixup();
	void ECMAttack(int power_val);
protected:
	virtual void Save();
	virtual void Load();
private:
	void Explode();

	int m_power;
	Body *m_target;
	Body *m_owner;
	double m_distToTarget;
};

#endif /* _MISSILE_H */
