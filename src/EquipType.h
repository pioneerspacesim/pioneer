#ifndef _EQUIPTYPE_H
#define _EQUIPTYPE_H
#include "Color.h"

#define EQUIP_INPUTS	2

struct EquipType;
struct LaserType;

namespace Equip {
	enum Slot { // <enum scope='Equip' name=EquipSlot prefix=SLOT_>
		SLOT_CARGO,
		SLOT_ENGINE,
		SLOT_LASER,
		SLOT_MISSILE,
		SLOT_ECM,
		SLOT_SCANNER,
		SLOT_RADARMAPPER,
		SLOT_HYPERCLOUD,
		SLOT_HULLAUTOREPAIR,
		SLOT_ENERGYBOOSTER,
		SLOT_CABIN,
		SLOT_SHIELD,
		SLOT_ATMOSHIELD,
		SLOT_FUELSCOOP,
		SLOT_CARGOSCOOP,
		SLOT_LASERCOOLER,
		SLOT_CARGOLIFESUPPORT,
		SLOT_AUTOPILOT,
		SLOT_MAX // <enum skip>
	};
	enum Type { // <enum scope='Equip' name=EquipType>
		NONE,
		HYDROGEN,
		LIQUID_OXYGEN,
		METAL_ORE,
		CARBON_ORE,
		METAL_ALLOYS,
		PLASTICS,
		FRUIT_AND_VEG,
		ANIMAL_MEAT,
		LIVE_ANIMALS,
		LIQUOR,
		GRAIN,
		TEXTILES,
		FERTILIZER,
		WATER,
		MEDICINES,
		CONSUMER_GOODS,
		COMPUTERS,
		ROBOTS,
		PRECIOUS_METALS,
		INDUSTRIAL_MACHINERY,
		FARM_MACHINERY,
		MINING_MACHINERY,
		AIR_PROCESSORS,
		SLAVES,
		HAND_WEAPONS,
		BATTLE_WEAPONS,
		NERVE_GAS,
		NARCOTICS,
		MILITARY_FUEL,
		RUBBISH,
		RADIOACTIVES,

		MISSILE_UNGUIDED,
		MISSILE_GUIDED,
		MISSILE_SMART,
		MISSILE_NAVAL,
		ATMOSPHERIC_SHIELDING,
		ECM_BASIC,
		SCANNER,
		ECM_ADVANCED,
		UNOCCUPIED_CABIN,
		PASSENGER_CABIN,
		SHIELD_GENERATOR,
		LASER_COOLING_BOOSTER,
		CARGO_LIFE_SUPPORT,
		AUTOPILOT,
		RADAR_MAPPER,
		FUEL_SCOOP,
		CARGO_SCOOP,
		HYPERCLOUD_ANALYZER,
		HULL_AUTOREPAIR,
		SHIELD_ENERGY_BOOSTER,
		DRIVE_CLASS1,
		DRIVE_CLASS2,
		DRIVE_CLASS3,
		DRIVE_CLASS4,
		DRIVE_CLASS5,
		DRIVE_CLASS6,
		DRIVE_CLASS7,
		DRIVE_CLASS8,
		DRIVE_CLASS9,
		DRIVE_MIL1,
		DRIVE_MIL2,
		DRIVE_MIL3,
		DRIVE_MIL4,
		PULSECANNON_1MW,
		PULSECANNON_DUAL_1MW,
		PULSECANNON_2MW,
		PULSECANNON_RAPID_2MW,
		PULSECANNON_4MW,
		PULSECANNON_10MW,
		PULSECANNON_20MW,
		MININGCANNON_17MW,
		SMALL_PLASMA_ACCEL,
		LARGE_PLASMA_ACCEL,

		TYPE_MAX, // <enum skip>

		// markers for the start & end of the lists
		FIRST_COMMODITY = HYDROGEN, // <enum skip>
		LAST_COMMODITY = RADIOACTIVES, // <enum skip>
		FIRST_SHIPEQUIP = MISSILE_UNGUIDED, // <enum skip>
		LAST_SHIPEQUIP = LARGE_PLASMA_ACCEL, // <enum skip>
	};

	const int LASER_MINING = 0x1;
	const int LASER_DUAL = 0x2;

	extern const LaserType lasers[];
	extern const EquipType types[];
}

struct EquipType {
	const char *name;
	const char *description;
	Equip::Slot slot;
	int tableIndex;			// Index into subtype-specific table
	// for commodities: production requirement. eg metal alloys input would be metal ore
	// for hyperdrives: inputs[0] = fuel commodity
	Equip::Type inputs[EQUIP_INPUTS];
	int basePrice;
	int mass;
	int pval; // hello angband. used for general 'power' attribute...
	int econType;
	bool purchasable;
	float rechargeTime;			// to be eliminated maybe
};

struct LaserType {
	float lifespan;		// seconds
	float speed;		// m/s
	float damage;
	float rechargeTime;	// seconds
	float length;		// meters
	float width;		// meters
	int flags;
	Color color;
};

#endif /* _EQUIPTYPE_H */
