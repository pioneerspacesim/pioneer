// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SHIPTYPE_H
#define _SHIPTYPE_H

#include "libs.h"
#include "vector3.h"
#include <vector>
#include <map>
#include <string>
#include "Propulsion.h"
#include "FixedGuns.h"

struct ShipType {
	enum DualLaserOrientation { // <enum scope='ShipType' name='DualLaserOrientation' prefix='DUAL_LASERS_' public>
		DUAL_LASERS_HORIZONTAL,
		DUAL_LASERS_VERTICAL
	};
	enum Tag { // <enum scope='ShipType' name=ShipTypeTag prefix=TAG_ public>
		TAG_NONE,
		TAG_SHIP,
		TAG_STATIC_SHIP,
		TAG_MISSILE,
		TAG_MAX // <enum skip>
	};
	typedef std::string Id;

	ShipType() {};
	ShipType(const Id &id, const std::string &path);

	////////
	Tag tag;
	Id id;
	std::string name;
	std::string shipClass;
	std::string manufacturer;
	std::string modelName;
	std::string cockpitName;
	float linThrust[THRUSTER_MAX];
	float angThrust;
	std::map<std::string, int> slots;
	double thrusterUpgrades[4];
	int capacity; // tonnes
	int hullMass;
	float effectiveExhaustVelocity; // velocity at which the propellant escapes the engines
	int fuelTankMass; //full fuel tank mass, on top of hullMass

	// storing money as a double is weird, but the value is a double on the Lua side anyway,
	// so we don't lose anything by storing it as a double here too
	double baseprice;

	int hyperdriveClass;
	int minCrew, maxCrew; // XXX really only for Lua, but needs to be declared in the ship def
	///////

	// percentage (ie, 0--100) of tank used per second at full thrust
	float GetFuelUseRate() const;

	static const std::string POLICE;
	static const std::string MISSILE_GUIDED;
	static const std::string MISSILE_NAVAL;
	static const std::string MISSILE_SMART;
	static const std::string MISSILE_UNGUIDED;

	static std::map<Id, const ShipType> types;
	static std::vector<Id> player_ships;
	static std::vector<Id> static_ships;
	static std::vector<Id> missile_ships;

	static void Init();
	static const ShipType *Get(const char *name) {
		std::map<Id, const ShipType>::iterator t = types.find(name);
		if (t == types.end()) return 0;
		else return &(*t).second;
	}
};

#endif /* _SHIPTYPE_H */
