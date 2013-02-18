// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _WEAPON_H
#define _WEAPON_H
/*
 * Gun used by ships (or why not other entities too)
 */
#include "libs.h"
#include "Serializer.h"

class Ship;

class Weapon {
public:
	Weapon();
	virtual ~Weapon();

	virtual void Save(Serializer::Writer &wr);
	virtual void Load(Serializer::Reader &rd);

	void Update(float timeStep);

	void Fire();

	//XXX silly. Remove?
	void SetState(int state) { m_state = state; }
	Uint32 GetState() const  { return m_state;  }

	float GetTemperature() const { return m_temperature; }

	void SetCoolingMultiplier(float) { m_coolingMultiplier = 1.f; }

	void SetShip(Ship *s) { m_ship = s; }

private:
	Uint32 m_state;
	float m_recharge;
	float m_temperature;

	float m_coolingRate;
	float m_coolingMultiplier;

	Ship* m_ship;
};

typedef std::vector<Weapon*>::iterator WeaponIterator;

#endif
