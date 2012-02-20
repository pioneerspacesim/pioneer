#include "EquipType.h"
#include "StarSystem.h" // for ECON_* enum
#include "Lang.h"

const EquipType Equip::types[Equip::TYPE_MAX] = {
	{ Lang::NONE, 0,
	  Equip::SLOT_CARGO, -1, {},
	  0, 0, 0, 0, 0, 0
	},{
	  Lang::HYDROGEN, Lang::HYDROGEN_DESCRIPTION,
	  Equip::SLOT_CARGO, -1, {},
	  100, 1, 0, ECON_MINING, 0, 0
	},{
	  Lang::LIQUID_OXYGEN, Lang::LIQUID_OXYGEN_DESCRIPTION,
	  Equip::SLOT_CARGO, -1, { Equip::WATER, Equip::INDUSTRIAL_MACHINERY },
	  150, 1, 0, ECON_MINING, 0, 0
	},{
	  Lang::METAL_ORE, 0,
	  Equip::SLOT_CARGO, -1, { Equip::MINING_MACHINERY },
	  300, 1, 0, ECON_MINING, 0, 0
	},{
	  Lang::CARBON_ORE, Lang::CARBON_ORE_DESCRIPTION,
	  Equip::SLOT_CARGO, -1, { Equip::MINING_MACHINERY },
	  500, 1, 0, ECON_MINING, 0, 0
	},{
	  Lang::METAL_ALLOYS, 0,
	  Equip::SLOT_CARGO, -1, { Equip::METAL_ORE, Equip::INDUSTRIAL_MACHINERY },
	  800, 1, 0, ECON_INDUSTRY, 1, 0
	},{
	  Lang::PLASTICS, 0,
	  Equip::SLOT_CARGO, -1, { Equip::CARBON_ORE, Equip::INDUSTRIAL_MACHINERY },
	  1200, 1, 0, ECON_INDUSTRY, 2, 0
	},{
	  Lang::FRUIT_AND_VEG, 0,
	  Equip::SLOT_CARGO, -1, { Equip::FARM_MACHINERY, Equip::FERTILIZER },
	  1200, 1, 0, ECON_AGRICULTURE, 1, 0
	},{
	  Lang::ANIMAL_MEAT, 0,
	  Equip::SLOT_CARGO, -1, { Equip::FARM_MACHINERY, Equip::FERTILIZER },
	  1800, 1, 0, ECON_AGRICULTURE, 1, 0
	},{
	  Lang::LIVE_ANIMALS, 0,
	  Equip::SLOT_CARGO, -1, { Equip::FARM_MACHINERY, Equip::FERTILIZER },
	  3200, 1, 0, ECON_AGRICULTURE, 1, 0
	},{
	  Lang::LIQUOR, 0,
	  Equip::SLOT_CARGO, -1, { Equip::FARM_MACHINERY, Equip::FERTILIZER },
	  800, 1, 0, ECON_AGRICULTURE, 1, 0
	},{
	  Lang::GRAIN, 0,
	  Equip::SLOT_CARGO, -1, { Equip::FARM_MACHINERY, Equip::FERTILIZER },
	  1000, 1, 0, ECON_AGRICULTURE, 1, 0
	},{
	  Lang::TEXTILES, 0,
	  Equip::SLOT_CARGO, -1, { Equip::PLASTICS },
	  850, 1, 0, ECON_INDUSTRY, 2, 0
	},{
	  Lang::FERTILIZER, 0,
	  Equip::SLOT_CARGO, -1, { Equip::CARBON_ORE },
	  400, 1, 0, ECON_INDUSTRY, 2, 0
	},{
	  Lang::WATER, 0,
	  Equip::SLOT_CARGO, -1, { Equip::MINING_MACHINERY },
	  120, 1, 0, ECON_MINING, 0, 0
	},{
	  Lang::MEDICINES,0,
	  Equip::SLOT_CARGO, -1, { Equip::COMPUTERS, Equip::CARBON_ORE },
	  2200, 1, 0, ECON_INDUSTRY, 3, 0
	},{
	  Lang::CONSUMER_GOODS,0,
	  Equip::SLOT_CARGO, -1, { Equip::PLASTICS, Equip::TEXTILES },
	  14000, 1, 0, ECON_INDUSTRY, 4, 0
	},{
	  Lang::COMPUTERS,0,
	  Equip::SLOT_CARGO, -1, { Equip::PRECIOUS_METALS, Equip::INDUSTRIAL_MACHINERY },
	  8000, 1, 0, ECON_INDUSTRY, 5, 0
	},{
	  Lang::ROBOTS,0,
	  Equip::SLOT_CARGO, -1, { Equip::PLASTICS, Equip::COMPUTERS },
	  6300, 1, 0, ECON_INDUSTRY, 5, 0
	},{
	  Lang::PRECIOUS_METALS, 0,
	  Equip::SLOT_CARGO, -1, { Equip::MINING_MACHINERY },
	  18000, 1, 0, ECON_MINING, 1, 0
	},{
	  Lang::INDUSTRIAL_MACHINERY,0,
	  Equip::SLOT_CARGO, -1, { Equip::METAL_ALLOYS, Equip::ROBOTS },
	  1300, 1, 0, ECON_INDUSTRY, 3, 0
	},{
	  Lang::FARM_MACHINERY,0,
	  Equip::SLOT_CARGO, -1, { Equip::METAL_ALLOYS, Equip::ROBOTS },
	  1100, 1, 0, ECON_INDUSTRY, 3, 0
	},{
	  Lang::MINING_MACHINERY,0,
	  Equip::SLOT_CARGO, -1, { Equip::METAL_ALLOYS, Equip::ROBOTS },
	  1200, 1, 0, ECON_INDUSTRY, 3, 0
	},{
	  Lang::AIR_PROCESSORS,0,
	  Equip::SLOT_CARGO, -1, { Equip::PLASTICS, Equip::INDUSTRIAL_MACHINERY },
	  2000, 1, 0, ECON_INDUSTRY, 3, 0
	},{
	  Lang::SLAVES,0,
	  Equip::SLOT_CARGO, -1, { },
	  23200, 1, 0, ECON_AGRICULTURE, 0, 0
	},{
	  Lang::HAND_WEAPONS,0,
	  Equip::SLOT_CARGO, -1, { Equip::COMPUTERS },
	  12400, 1, 0, ECON_INDUSTRY, 4, 0
	},{
	  Lang::BATTLE_WEAPONS,0,
	  Equip::SLOT_CARGO, -1, { Equip::INDUSTRIAL_MACHINERY, Equip::METAL_ALLOYS },
	  22000, 1, 0, ECON_INDUSTRY, 4, 0
	},{
	  Lang::NERVE_GAS,0,
	  Equip::SLOT_CARGO, -1, {},
	  26500, 1, 0, ECON_INDUSTRY, 3, 0
	},{
	  Lang::NARCOTICS,0,
	  Equip::SLOT_CARGO, -1, {},
	  15700, 1, 0, ECON_INDUSTRY, 3, 0
	},{
	  Lang::MILITARY_FUEL,0,
	  Equip::SLOT_CARGO, -1, { Equip::HYDROGEN },
	  6000, 1, 0, ECON_INDUSTRY, 3, 0
	},{
	  Lang::RUBBISH,0,
	  Equip::SLOT_CARGO, -1, { },
	  -10, 1, 0, ECON_INDUSTRY, 0, 0
	},{
	  Lang::RADIOACTIVES,0,
	  Equip::SLOT_CARGO, -1, { },
	  -35, 1, 0, ECON_INDUSTRY, 0, 0
	},{
	  Lang::MISSILE_UNGUIDED,0,
	  Equip::SLOT_MISSILE, -1, {},
	  3000, 1, 0, 0, 0, 0
	},{
	  Lang::MISSILE_GUIDED,0,
	  Equip::SLOT_MISSILE, -1, {},
	  5000, 1, 0, 0, 0, 0
	},{
	  Lang::MISSILE_SMART,0,
	  Equip::SLOT_MISSILE, -1, {},
	  9500, 1, 0, 0, 3, 0
	},{
	  Lang::MISSILE_NAVAL,0,
	  Equip::SLOT_MISSILE, -1, {},
	  16000, 1, 0, 0, 4, 0
	},{
	  Lang::ATMOSPHERIC_SHIELDING,
	  Lang::ATMOSPHERIC_SHIELDING_DESCRIPTION,
	  Equip::SLOT_ATMOSHIELD, -1, {},
	  20000, 1, 1, 0, 1, 0
	},{
	  Lang::ECM_BASIC,
	  Lang::ECM_BASIC_DESCRIPTION,
	  Equip::SLOT_ECM, -1, {},
	  600000, 2, 2, 0, 2, 5.0,
	},{
	  Lang::SCANNER,
	  Lang::SCANNER_DESCRIPTION,
	  Equip::SLOT_SCANNER, -1, {},
	  68000, 1, 0, 0, 2, 0
	},{
	  Lang::ECM_ADVANCED,
	  Lang::ECM_ADVANCED_DESCRIPTION,
	  Equip::SLOT_ECM, -1, {},
	  1520000, 2, 3, 0, 5, 5.0
	},{
	  Lang::UNOCCUPIED_CABIN,
	  Lang::UNOCCUPIED_CABIN_DESCRIPTION,
	  Equip::SLOT_CABIN, -1, {},
	  135000, 5, 1, 0, 1, 5.0
	},{
	  Lang::PASSENGER_CABIN,0,
	  Equip::SLOT_CABIN, -1, {},
	  -135000, 5, 1, 0, 99, 5.0 // XXX: techLevel is 99 so that item can not be bought/sold
	},{
	  Lang::SHIELD_GENERATOR,
	  Lang::SHIELD_GENERATOR_DESCRIPTION,
	  Equip::SLOT_SHIELD, -1, {},
	  250000, 4, 1, 0, 4, 5.0
	},{
	  Lang::LASER_COOLING_BOOSTER,
	  Lang::LASER_COOLING_BOOSTER_DESCRIPTION,
	  Equip::SLOT_LASERCOOLER, -1, {},
	  38000, 1, 2, 0, 2, 0
	},{
	  Lang::CARGO_LIFE_SUPPORT,
	  Lang::CARGO_LIFE_SUPPORT_DESCRIPTION,
	  Equip::SLOT_CARGOLIFESUPPORT, -1, {},
	  70000, 1, 1, 0, 2, 0
	},{
	  Lang::AUTOPILOT,
	  Lang::AUTOPILOT_DESCRIPTION,
	  Equip::SLOT_AUTOPILOT, -1, {},
	  140000, 1, 1, 0, 2, 0
	},{
	  Lang::RADAR_MAPPER,
	  Lang::RADAR_MAPPER_DESCRIPTION,
	  Equip::SLOT_RADARMAPPER, -1, {},
	  90000, 1, 1, 0, 3, 0
	},{
	  Lang::FUEL_SCOOP,
	  Lang::FUEL_SCOOP_DESCRIPTION,
	  Equip::SLOT_FUELSCOOP, -1, {},
	  350000, 6, 1, 0, 1, 0
	},{
	  Lang::CARGO_SCOOP,
	  Lang::CARGO_SCOOP_DESCRIPTION,
	  Equip::SLOT_CARGOSCOOP, -1, {},
	  390000, 7, 1, 0, 1, 0
	},{
	  Lang::HYPERCLOUD_ANALYZER,
	  Lang::HYPERCLOUD_ANALYZER_DESCRIPTION,
	  Equip::SLOT_HYPERCLOUD, -1, {},
	  150000, 1, 1, 0, 3, 0
	},{
	  Lang::HULL_AUTOREPAIR,
	  Lang::HULL_AUTOREPAIR_DESCRIPTION,
	  Equip::SLOT_HULLAUTOREPAIR, -1, {},
	  1600000, 40, 1, 0, 4, 0
	},{
	  Lang::SHIELD_ENERGY_BOOSTER,
	  Lang::SHIELD_ENERGY_BOOSTER_DESCRIPTION,
	  Equip::SLOT_ENERGYBOOSTER, -1, {},
	  1000000, 8, 2, 0, 3, 0
	},{
	  Lang::DRIVE_CLASS1,0,
	  Equip::SLOT_ENGINE, -1, {Equip::HYDROGEN},
	  70000, 4, 1, 0, 0, 0
	},{
	  Lang::DRIVE_CLASS2,0,
	  Equip::SLOT_ENGINE, -1, {Equip::HYDROGEN},
	  130000, 10, 2, 0, 0, 0
	},{
	  Lang::DRIVE_CLASS3,0,
	  Equip::SLOT_ENGINE, -1, {Equip::HYDROGEN},
	  250000, 20, 3, 0, 0, 0
	},{
	  Lang::DRIVE_CLASS4,0,
	  Equip::SLOT_ENGINE, -1, {Equip::HYDROGEN},
	  500000, 40, 4, 0, 0, 0
	},{
	  Lang::DRIVE_CLASS5,0,
	  Equip::SLOT_ENGINE, -1, {Equip::HYDROGEN},
	  1000000, 120, 5, 0, 0, 0
	},{
	  Lang::DRIVE_CLASS6,0,
	  Equip::SLOT_ENGINE, -1, {Equip::HYDROGEN},
	  2000000, 225, 6, 0, 0, 0
	},{
	  Lang::DRIVE_CLASS7,0,
	  Equip::SLOT_ENGINE, -1, {Equip::HYDROGEN},
	  3000000, 400, 7, 0, 0, 0
	},{
	  Lang::DRIVE_CLASS8,0,
	  Equip::SLOT_ENGINE, -1, {Equip::HYDROGEN},
	  6000000, 580, 8, 0, 0, 0
	},{
	  Lang::DRIVE_CLASS9,0,
	  Equip::SLOT_ENGINE, -1, {Equip::HYDROGEN},
	  12000000, 740, 9, 0, 0, 0
	},{
	  Lang::DRIVE_MIL1,0,
	  Equip::SLOT_ENGINE, -1, {Equip::MILITARY_FUEL},
	  2300000, 3, 1, 0, 0, 0
	},{
	  Lang::DRIVE_MIL2,0,
	  Equip::SLOT_ENGINE, -1, {Equip::MILITARY_FUEL},
	  4700000, 8, 2, 0, 0, 0
	},{
	  Lang::DRIVE_MIL3,0,
	  Equip::SLOT_ENGINE, -1, {Equip::MILITARY_FUEL},
	  8500000, 16, 3, 0, 0, 0
	},{
	  Lang::DRIVE_MIL4,0,
	  Equip::SLOT_ENGINE, -1, {Equip::MILITARY_FUEL},
	  21400000, 30, 4, 0, 0, 0
	},{
	  Lang::PULSECANNON_1MW,0,
	  Equip::SLOT_LASER, 1, {},
	  60000, 1, 1, 0, 0, 0
	},{
	  Lang::PULSECANNON_DUAL_1MW,0,
	  Equip::SLOT_LASER, 2, {},
	  110000, 4, 2, 0, 0, 0
	},{
	  Lang::PULSECANNON_2MW,0,
	  Equip::SLOT_LASER, 3, {},
	  100000, 3, 2, 0, 0, 0
	},{
	  Lang::PULSECANNON_RAPID_2MW,0,
	  Equip::SLOT_LASER, 4, {},
	  180000, 7, 2, 0, 0, 0
	},{
	  Lang::PULSECANNON_4MW,0,
	  Equip::SLOT_LASER, 5, {},
	  220000, 10, 4, 0, 0, 0
	},{
	  Lang::PULSECANNON_10MW,0,
	  Equip::SLOT_LASER, 6, {},
	  490000, 30, 10, 0, 0, 0
	},{
	  Lang::PULSECANNON_20MW,0,
	  Equip::SLOT_LASER, 7, {},
	  1200000, 65, 20, 0, 0, 0
	},{
	  Lang::MININGCANNON_17MW,
	  Lang::MININGCANNON_17MW_DESCRIPTION,
	  Equip::SLOT_LASER, 8, {},
	  1060000, 10, 17, 0, 1, 0
	},{
	  Lang::SMALL_PLASMA_ACCEL,0,
	  Equip::SLOT_LASER, 9, {},
	  12000000, 22, 50, 0, 0, 0
	},{
	  Lang::LARGE_PLASMA_ACCEL,0,
	  Equip::SLOT_LASER, 10, {},
	  39000000, 50, 100, 0, 0, 0
	}
};

const LaserType Equip::lasers[] = {
	{
		0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0, 0,
		Color(0.0f, 0.0f, 0.0f, 0.0f),
	},{		// 1mw pulse
		8.0f, 1000.0f, 1000.0f, 0.25f, 30.0f, 5.0f, 0,
		Color(1.0f, 0.2f, 0.2f, 1.0f),
	},{		// 1mw df pulse
		8.0f, 1000.0f, 1000.0f, 0.25f, 30.0f, 5.0f, Equip::LASER_DUAL,
		Color(1.0f, 0.2f, 0.2f, 1.0f),
	},{	// 2mw pulse
		8.0f, 1000.0f, 2000.0f, 0.25f, 30.0f, 5.0f, 0,
		Color(1.0f, 0.5f, 0.2f, 1.0f),
	},{	// 2mw rf pulse
		8.0f, 1000.0f, 2000.0f, 0.13f, 30.0f, 5.0f, 0,
		Color(1.0f, 0.5f, 0.2f, 1.0f),
	},{		// 4mw pulse
		8.0f, 1000.0f, 4000.0f, 0.25f, 30.0f, 5.0f, 0,
		Color(1.0f, 1.0f, 0.2f, 1.0f),
	},{		// 10mw pulse
		8.0f, 1000.0f, 10000.0f, 0.25f, 30.0f, 5.0f, 0,
		Color(0.2f, 1.0f, 0.2f, 1.0f),
	},{		// 20mw pulse
		8.0f, 1000.0f, 20000.0f, 0.25f, 30.0f, 5.0f, 0,
		Color(0.1f, 0.2f, 1.0f, 1.0f),
	},{		// 17mw mining
		8.0f, 1000.0f, 17000.0f, 2.0f, 30.0f, 5.0f, Equip::LASER_MINING,
		Color(0.2f, 0.5f, 1.0f, 1.0f),
	},{		// small plasma accel
		8.0f, 1000.0f, 50000.0f, 0.3f, 42.0f, 7.0f, 0,
		Color(0.2f, 1.0f, 1.0f, 1.0f),
	},{		// large plasma accel
		8.0f, 1000.0f, 100000.0f, 0.3f, 54.0f, 9.0f, 0,
		Color(0.5f, 1.0f, 1.0f, 1.0f),
	}
};
