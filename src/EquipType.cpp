#include "EquipType.h"
#include "StarSystem.h"

const EquipType EquipType::types[Equip::TYPE_MAX] = {
	{ "None", 0,
	  Equip::SLOT_CARGO, {},
	  0, 0
	},{
	  "Hydrogen", "Hydrogen is primarily used as a fusion fuel",
	  Equip::SLOT_CARGO, {},
	  100, 1, 0, ECON_MINING, 0,
	},{
	  "Liquid Oxygen", "Oxygen is required for life support systems and "
	  "some industrial processes",
	  Equip::SLOT_CARGO, { Equip::WATER, Equip::INDUSTRIAL_MACHINERY },
	  150, 1, 0, ECON_MINING, 0,
	},{
	  "Metal ore", 0,
	  Equip::SLOT_CARGO, { Equip::MINING_MACHINERY },
	  300, 1, 0, ECON_MINING, 0,
	},{
	  "Carbon ore", "Carbon ores (coal and oil) are required for the synthesis "
	  "of many useful chemicals, including plastics, synthetic foodstuffs, "
	  "medicines and textiles",
	  Equip::SLOT_CARGO, { Equip::MINING_MACHINERY },
	  500, 1, 0, ECON_MINING, 0,
	},{
	  "Metal alloys", 0,
	  Equip::SLOT_CARGO, { Equip::METAL_ORE, Equip::INDUSTRIAL_MACHINERY },
	  800, 1, 0, ECON_INDUSTRY, 1,
	},{
	  "Plastics", 0,
	  Equip::SLOT_CARGO, { Equip::CARBON_ORE, Equip::INDUSTRIAL_MACHINERY },
	  1200, 1, 0, ECON_INDUSTRY, 2,
	},{
	  "Fruit and Veg", 0,
	  Equip::SLOT_CARGO, { Equip::FARM_MACHINERY, Equip::FERTILIZER },
	  1200, 1, 0, ECON_AGRICULTURE, 1,
	},{
	  "Animal Meat", 0,
	  Equip::SLOT_CARGO, { Equip::FARM_MACHINERY, Equip::FERTILIZER },
	  1800, 1, 0, ECON_AGRICULTURE, 1,
	},{
	  "Live Animals", 0,
	  Equip::SLOT_CARGO, { Equip::FARM_MACHINERY, Equip::FERTILIZER },
	  3200, 1, 0, ECON_AGRICULTURE, 1,
	},{
	  "Liquor", 0,
	  Equip::SLOT_CARGO, { Equip::FARM_MACHINERY, Equip::FERTILIZER },
	  800, 1, 0, ECON_AGRICULTURE, 1,
	},{
	  "Grain", 0,
	  Equip::SLOT_CARGO, { Equip::FARM_MACHINERY, Equip::FERTILIZER },
	  1000, 1, 0, ECON_AGRICULTURE, 1,
	},{
	  "Textiles", 0,
	  Equip::SLOT_CARGO, { Equip::PLASTICS },
	  850, 1, 0, ECON_INDUSTRY, 2,
	},{
	  "Fertilizer", 0,
	  Equip::SLOT_CARGO, { Equip::CARBON_ORE },
	  400, 1, 0, ECON_INDUSTRY, 2,
	},{
	  "Water", 0,
	  Equip::SLOT_CARGO, { Equip::MINING_MACHINERY },
	  120, 1, 0, ECON_MINING, 0,
	},{
	  "Medicines",0,
	  Equip::SLOT_CARGO, { Equip::COMPUTERS, Equip::CARBON_ORE },
	  2200, 1, 0, ECON_INDUSTRY, 3,
	},{
	  "Consumer goods",0,
	  Equip::SLOT_CARGO, { Equip::PLASTICS, Equip::TEXTILES },
	  14000, 1, 0, ECON_INDUSTRY, 4,
	},{
	  "Computers",0,
	  Equip::SLOT_CARGO, { Equip::PRECIOUS_METALS, Equip::INDUSTRIAL_MACHINERY },
	  8000, 1, 0, ECON_INDUSTRY, 5,
	},{
	  "Robots",0,
	  Equip::SLOT_CARGO, { Equip::PLASTICS, Equip::COMPUTERS },
	  6300, 1, 0, ECON_INDUSTRY, 5,
	},{
	  "Precious metals", 0,
	  Equip::SLOT_CARGO, { Equip::MINING_MACHINERY },
	  18000, 1, 0, ECON_MINING, 1,
	},{
	  "Industrial machinery",0,
	  Equip::SLOT_CARGO, { Equip::METAL_ALLOYS, Equip::ROBOTS },
	  1300, 1, 0, ECON_INDUSTRY, 3,
	},{
	  "Farm machinery",0,
	  Equip::SLOT_CARGO, { Equip::METAL_ALLOYS, Equip::ROBOTS },
	  1100, 1, 0, ECON_INDUSTRY, 3,
	},{
	  "Mining machinery",0,
	  Equip::SLOT_CARGO, { Equip::METAL_ALLOYS, Equip::ROBOTS },
	  1200, 1, 0, ECON_INDUSTRY, 3,
	},{
	  "Air processors",0,
	  Equip::SLOT_CARGO, { Equip::PLASTICS, Equip::INDUSTRIAL_MACHINERY },
	  2000, 1, 0, ECON_INDUSTRY, 3,
	},{
	  "Slaves",0,
	  Equip::SLOT_CARGO, { },
	  23200, 1, 0, ECON_AGRICULTURE, 0,
	},{
	  "Hand weapons",0,
	  Equip::SLOT_CARGO, { Equip::COMPUTERS },
	  12400, 1, 0, ECON_INDUSTRY, 4,
	},{
	  "Battle weapons",0,
	  Equip::SLOT_CARGO, { Equip::INDUSTRIAL_MACHINERY, Equip::METAL_ALLOYS },
	  22000, 1, 0, ECON_INDUSTRY, 4,
	},{
	  "Nerve Gas",0,
	  Equip::SLOT_CARGO, {},
	  26500, 1, 0, ECON_INDUSTRY, 3,
	},{
	  "Narcotics",0,
	  Equip::SLOT_CARGO, {},
	  15700, 1, 0, ECON_INDUSTRY, 3,
	},{
	  "Military fuel",0,
	  Equip::SLOT_CARGO, { Equip::HYDROGEN },
	  6000, 1, 0, ECON_INDUSTRY, 3,
	},{
	  "Rubbish",0,
	  Equip::SLOT_CARGO, { },
	  -10, 1, 0, ECON_INDUSTRY, 0,
	},{
	  "Radioactive waste",0,
	  Equip::SLOT_CARGO, { },
	  -35, 1, 0, ECON_INDUSTRY, 0,
	},{
	  "R40 Unguided Rocket",0,
	  Equip::SLOT_MISSILE, {},
	  3000, 1, 0,
	  0, 0,
	},{
	  "Guided Missile",0,
	  Equip::SLOT_MISSILE, {},
	  5000, 1, 0,
	  0, 0,
	},{
	  "Smart Missile",0,
	  Equip::SLOT_MISSILE, {},
	  9500, 1, 0,
	  0, 3,
	},{
	  "Naval Missile",0,
	  Equip::SLOT_MISSILE, {},
	  16000, 1, 0,
	  0, 4
	},{
	  "Atmospheric Shielding",
	  "Shields your spaceship from the heat of atmospheric re-entry.",
	  Equip::SLOT_ATMOSHIELD, {},
	  20000, 1, 1,
	  0, 1,
	},{
	  "ECM system",
	  "An electronic countermeasure missile defence system, capable of destroying some homing missiles.",
	  Equip::SLOT_ECM, {},
	  600000, 2, 2,
	  0, 2, 5.0,
	},{
	  "Scanner",
	  "Provides a 3D map of nearby ships.",
	  Equip::SLOT_SCANNER, {},
	  68000, 1, 0,
	  0, 2
	},{
	  "Advanced ECM system",
	  "An electronic countermeasure missile defence system, capable of destroying more advanced types of homing missiles.",
	  Equip::SLOT_ECM, {},
	  1520000, 2, 3,
	  0, 5, 5.0
	},{
	  "Shield Generator",
	  "Provides additional hull defences with each unit fitted.",
	  Equip::SLOT_CARGO, {},
	  250000, 4, 1,
	  0, 4, 5.0
	},{
	  "Laser Cooling Booster",
	  "An improved cooling system for your weapons.",
	  Equip::SLOT_LASERCOOLER, {},
	  38000, 1, 2,
	  0, 2
	},{
	  "Cargo Bay Life Support",
	  "Allows the transport of live cargo.",
	  Equip::SLOT_CARGOLIFESUPPORT, {},
	  70000, 1, 1,
	  0, 2
	},{
	  "Autopilot",
	  "An onboard flight computer.",
	  Equip::SLOT_AUTOPILOT, {},
	  140000, 1, 1,
	  0, 2
	},{
	  "Radar Mapper",
	  "Used to remotely inspect the equipment, cargo and state of other ships.",
	  Equip::SLOT_RADARMAPPER, {},
	  90000, 1, 1,
	  0, 3
	},{
	  "Fuel Scoop",
	  "Permits scooping hydrogen fuel from gas giant planets.",
	  Equip::SLOT_FUELSCOOP, {},
	  350000, 6, 1,
	  0, 1
	},{
	  "Hypercloud Analyzer",
	  "Analyze hyperspace clouds to determine destination and time of arrival or departure.",
	  Equip::SLOT_HYPERCLOUD, {},
	  150000, 1, 1,
	  0, 3
	},{
	  "Hull Auto-Repair System",
	  "Automatically repairs the ship's hull in the event of damage.",
	  Equip::SLOT_HULLAUTOREPAIR, {},
	  1600000, 40, 1,
	  0, 4
	},{
	  "Shield Energy Booster",
	  "Increases the rate at which shields recharge.",
	  Equip::SLOT_ENERGYBOOSTER, {},
	  1000000, 8, 2,
	  0, 3
	},{
	  "Class 1 Hyperdrive",0,
	  Equip::SLOT_ENGINE, {Equip::HYDROGEN},
	  70000, 4, 1
	},{
	  "Class 2 Hyperdrive",0,
	  Equip::SLOT_ENGINE, {Equip::HYDROGEN},
	  130000, 10, 2
	},{
	  "Class 3 Hyperdrive",0,
	  Equip::SLOT_ENGINE, {Equip::HYDROGEN},
	  250000, 20, 3
	},{
	  "Class 4 Hyperdrive",0,
	  Equip::SLOT_ENGINE, {Equip::HYDROGEN},
	  500000, 40, 4
	},{
	  "Class 5 Hyperdrive",0,
	  Equip::SLOT_ENGINE, {Equip::HYDROGEN},
	  1000000, 120, 5
	},{
	  "Class 6 Hyperdrive",0,
	  Equip::SLOT_ENGINE, {Equip::HYDROGEN},
	  2000000, 225, 6
	},{
	  "Class 7 Hyperdrive",0,
	  Equip::SLOT_ENGINE, {Equip::HYDROGEN},
	  3000000, 400, 7
	},{
	  "Class 8 Hyperdrive",0,
	  Equip::SLOT_ENGINE, {Equip::HYDROGEN},
	  6000000, 580, 8
	},{
	  "Class 9 Hyperdrive",0,
	  Equip::SLOT_ENGINE, {Equip::HYDROGEN},
	  12000000, 740, 9
	},{
	  "Class 1 Military drive",0,
	  Equip::SLOT_ENGINE, {Equip::MILITARY_FUEL},
	  2300000, 3, 1
	},{
	  "Class 2 Military drive",0,
	  Equip::SLOT_ENGINE, {Equip::MILITARY_FUEL},
	  4700000, 8, 2
	},{
	  "Class 3 Military drive",0,
	  Equip::SLOT_ENGINE, {Equip::MILITARY_FUEL},
	  8500000, 16, 3
	},{
	  "Class 4 Military drive",0,
	  Equip::SLOT_ENGINE, {Equip::MILITARY_FUEL},
	  21400000, 30, 4
	},{
	  "1MW pulse cannon",0,
	  Equip::SLOT_LASER, {},
	  60000, 1, 1, 0, 0, 0.25
	},{
	  "1MW dual-fire pulse cannon",0,
	  Equip::SLOT_LASER, {},
	  110000, 4, 2, 0, 0, 0.25
	},{
	  "2MW pulse cannon",0,
	  Equip::SLOT_LASER, {},
	  100000, 3, 2, 0, 0, 0.25
	},{
	  "2MW rapid-fire pulse cannon",0,
	  Equip::SLOT_LASER, {},
	  180000, 7, 2, 0, 0, 0.13
	},{
	  "4MW pulse cannon",0,
	  Equip::SLOT_LASER, {},
	  220000, 10, 4, 0, 0, 0.25
	},{
	  "10MW pulse cannon",0,
	  Equip::SLOT_LASER, {},
	  490000, 30, 10, 0, 0, 0.25
	},{
	  "20MW pulse cannon",0,
	  Equip::SLOT_LASER, {},
	  1200000, 65, 20, 0, 0, 0.25
	},{
	  "17MW blast-mining cannon",
	  "Used to blast-mine mineral rich asteroids.",
	  Equip::SLOT_LASER, {},
	  1060000, 10, 17, 0, 1, 2.0
	},{
	  "Small plasma accelerator",0,
	  Equip::SLOT_LASER, {},
	  12000000, 22, 50, 0, 0, 0.3
	},{
	  "Large plasma accelerator",0,
	  Equip::SLOT_LASER, {},
	  39000000, 50, 100, 0, 0, 0.3
	}
};

