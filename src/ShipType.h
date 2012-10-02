// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SHIPTYPE_H
#define _SHIPTYPE_H

#include "libs.h"
#include "vector3.h"
#include "EquipType.h"
#include <vector>
#include <map>

struct ShipType {
	enum Thruster { // <enum scope='ShipType' name=ShipTypeThruster prefix=THRUSTER_>
		THRUSTER_REVERSE,
		THRUSTER_FORWARD,
		THRUSTER_UP,
		THRUSTER_DOWN,
		THRUSTER_LEFT,
		THRUSTER_RIGHT,
		THRUSTER_MAX // <enum skip>
	};
	enum {
		GUN_FRONT,
		GUN_REAR,
		GUNMOUNT_MAX = 2
	};
	enum Tag { // <enum scope='ShipType' name=ShipTypeTag prefix=TAG_>
		TAG_NONE,
		TAG_SHIP,
		TAG_STATIC_SHIP,
		TAG_MISSILE,
		TAG_MAX // <enum skip>
	};
	typedef std::string Type;

	////////
	Tag tag;
	std::string name;
	std::string lmrModelName;
	float linThrust[THRUSTER_MAX];
	float angThrust;
	struct GunMount {
		vector3f pos;
		vector3f dir;
		double sep;
		int rot;
	} gunMount[GUNMOUNT_MAX];
	int equipSlotCapacity[Equip::SLOT_MAX];
	int capacity; // tonnes
	int hullMass;
	float thrusterFuelUse; //%p per second at full thrust
	int fuelTankMass; //full fuel tank mass, on top of hullMass
	int baseprice;
	Equip::Type hyperdrive;
	vector3d frontViewOffset;
	vector3d rearViewOffset;
	vector3d frontCameraOffset;
	vector3d rearCameraOffset;
	vector3d leftCameraOffset;
	vector3d rightCameraOffset;
	vector3d topCameraOffset;
	vector3d bottomCameraOffset;
	///////

	static std::string LADYBIRD;
	static std::string SIRIUS_INTERDICTOR;
	static std::string EAGLE_LRF;
	static std::string EAGLE_MK3;
	static std::string MISSILE_GUIDED;
	static std::string MISSILE_NAVAL;
	static std::string MISSILE_SMART;
	static std::string MISSILE_UNGUIDED;

	static std::map<Type, ShipType> types;
	static std::vector<Type> player_ships;
	static std::vector<Type> static_ships;
	static std::vector<Type> missile_ships;

	static std::vector<Type> playable_atmospheric_ships;

	static const char *gunmountNames[GUNMOUNT_MAX];
	static void Init();
	static const ShipType *Get(const char *name) {
		std::map<Type, ShipType>::iterator t = types.find(name);
		if (t == types.end()) return 0;
		else return &(*t).second;
	}
};

#endif /* _SHIPTYPE_H */
