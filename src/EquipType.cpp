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
	  "Radar Mapper",
	  "Used to remotely inspect the equipment, cargo and state of other ships.",
	  Equip::SLOT_RADARMAPPER, {},
	  90000, 1, 1,
	  0, 3
	},{
	  "Hypercloud Analyzer",
	  "Analyze hyperspace clouds to determine destination and time of arrival or departure.",
	  Equip::SLOT_HYPERCLOUD, {},
	  150000, 1, 1,
	  0, 3
	},{
	  "Interplanetary Drive",0,
	  Equip::SLOT_ENGINE, {},
	  40000, 1, 0,
	  0, 0,
	},{
	  "Class 1 Hyperdrive",0,
	  Equip::SLOT_ENGINE, {},
	  70000, 4, 1
	},{
	  "Class 2 Hyperdrive",0,
	  Equip::SLOT_ENGINE, {},
	  130000, 10, 2
	},{
	  "Class 3 Hyperdrive",0,
	  Equip::SLOT_ENGINE, {},
	  250000, 20, 3
	},{
	  "Class 4 Hyperdrive",0,
	  Equip::SLOT_ENGINE, {},
	  500000, 40, 4
	},{
	  "Class 5 Hyperdrive",0,
	  Equip::SLOT_ENGINE, {},
	  1000000, 120, 5
	},{
	  "Class 6 Hyperdrive",0,
	  Equip::SLOT_ENGINE, {},
	  2000000, 225, 6
	},{
	  "Class 7 Hyperdrive",0,
	  Equip::SLOT_ENGINE, {},
	  3000000, 400, 7
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
	}
};

