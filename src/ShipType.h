#ifndef _SHIPTYPE_H
#define _SHIPTYPE_H

#include "libs.h"
#include "vector3.h"
#include <vector>

namespace Equip {
	enum Slot { SLOT_CARGO, SLOT_ENGINE, SLOT_LASER, SLOT_MISSILE, SLOT_MAX };
	enum Type { NONE, HYDROGEN, LIQUID_OXYGEN, METAL_ORE, CARBON_ORE, METAL_ALLOYS, PLASTICS, FRUIT_AND_VEG, ANIMAL_MEAT, LIQUOR, GRAIN, TEXTILES, FERTILIZER, WATER, MEDICINES, CONSUMER_GOODS, COMPUTERS, ROBOTS, PRECIOUS_METALS, INDUSTRIAL_MACHINERY, FARM_MACHINERY, AIR_PROCESSORS, HAND_WEAPONS, BATTLE_WEAPONS, NARCOTICS, DRIVE_INTERPLANETARY, DRIVE_CLASS1, DRIVE_CLASS2,
	DRIVE_CLASS3, DRIVE_CLASS4, DRIVE_CLASS5, DRIVE_CLASS6,
	LASER_1MW_BEAM, LASER_2MW_BEAM, LASER_4MW_BEAM, TYPE_MAX };
};
	
struct ShipType {
	enum Thruster { THRUSTER_FRONT, THRUSTER_REAR, THRUSTER_TOP, THRUSTER_BOTTOM, THRUSTER_LEFT, THRUSTER_RIGHT, THRUSTER_MAX };
	enum Type { SWANKY, LADYBIRD, FLOWERFAIRY };
	enum { GUN_FRONT, GUN_REAR, GUNMOUNT_MAX = 2 };

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
	bool Add(Equip::Slot s, Equip::Type e) {
		for (unsigned int i=0; i<equip[s].size(); i++) {
			if (equip[s][i] == Equip::NONE) {
				equip[s][i] = e;
				return true;
			}
		}
		return false;
	}
	void Remove(Equip::Slot s, Equip::Type e, int num) {
		for (unsigned int i=0; i<equip[s].size(); i++) {
			if (equip[s][i] == e) {
				equip[s][i] = Equip::NONE;
				num--;
			}
			if (num == 0) break;
		}
	}
	int Count(Equip::Slot s, Equip::Type e) const {
		int num = 0;
		for (unsigned int i=0; i<equip[s].size(); i++) {
			if (equip[s][i] == e) num++;
		}
		return num;
	}
	int FreeSpace(Equip::Slot s) const {
		int free = 0;
		for (unsigned int i=0; i<equip[s].size(); i++) {
			if (equip[s][i] == Equip::NONE) free++;
		}
		return free;
	}
	void Save();
	void Load();
private:
	std::vector<Equip::Type> equip[Equip::SLOT_MAX];
};

struct EquipType {
	const char *name;
	const char *description;
	Equip::Slot slot;
	int basePrice;
	int mass;
	int pval; // hello angband. used for general 'power' attribute...
	int econType;
	int techLevel;
	
	static const EquipType types[];
};


#endif /* _SHIPTYPE_H */
