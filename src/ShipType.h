#ifndef _SHIPTYPE_H
#define _SHIPTYPE_H

#include "libs.h"
#include "vector3.h"
#include <vector>

namespace Equip {
	enum Slot { SLOT_ENGINE, SLOT_LASER, SLOT_MISSILE, SLOT_MAX };
	enum Type { NONE, DRIVE_INTERPLANETARY, DRIVE_CLASS1, LASER_1MW_BEAM };
};
	
struct ShipType {
	enum Thruster { THRUSTER_FRONT, THRUSTER_REAR, THRUSTER_TOP, THRUSTER_BOTTOM, THRUSTER_LEFT, THRUSTER_RIGHT, THRUSTER_MAX };
	enum Type { SWANKY, LADYBIRD, FLOWERFAIRY };
	enum { GUNMOUNT_MAX = 2 };

	////////
	const char *name;
	int sbreModel;
	float linThrust[THRUSTER_MAX];
	float angThrust;
	struct GunMount {
		vector3f pos;
		vector3f dir;
	} gunMount[GUNMOUNT_MAX];
	int equipSlotCapacity[Equip::SLOT_MAX];
	int capacity; // tonnes
	int hullMass;
	///////

	static const ShipType types[];
};

class EquipSet {
public:
	EquipSet() {}
	EquipSet(ShipType::Type t) {
		for (int i=0; i<Equip::SLOT_MAX; i++) {
			equip[i] = std::vector<Equip::Type>(ShipType::types[t].equipSlotCapacity[i]);
		}
	}
	Equip::Type Get(Equip::Slot s) {
		return equip[s][0];
	}
	Equip::Type Get(Equip::Slot s, int idx) {
		return equip[s][idx];
	}
	void Set(Equip::Slot s, int idx, Equip::Type e) {
		equip[s][idx] = e;
	}
private:
	std::vector<Equip::Type> equip[Equip::SLOT_MAX];
};
	
struct EquipType {
	const char *name;
	Equip::Slot slot;
	int mass;
	
	static const EquipType types[];
};


#endif /* _SHIPTYPE_H */
