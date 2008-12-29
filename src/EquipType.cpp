#include "EquipType.h"
#include "StarSystem.h"

const EquipType EquipType::types[Equip::TYPE_MAX] = {
	{ "None", 0,
	  Equip::SLOT_CARGO, Equip::NONE,
	  0, 0
	},{
	  "Hydrogen", "Hydrogen is primarily used as a fusion fuel",
	  Equip::SLOT_CARGO, Equip::NONE,
	  100, 1, 0,
	},{
	  "Liquid Oxygen", "Oxygen is required for life support systems and "
	  "some industrial processes",
	  Equip::SLOT_CARGO, Equip::NONE,
	  150, 1, 0,
	},{
	  "Metal ore", 0,
	  Equip::SLOT_CARGO, Equip::INDUSTRIAL_MACHINERY,
	  300, 1, 0, ECON_MINING
	},{
	  "Carbon ore", "Carbon ores (coal and oil) are required for the synthesis "
	  "of many useful chemicals, including plastics, synthetic foodstuffs, "
	  "medicines and textiles",
	  Equip::SLOT_CARGO, Equip::INDUSTRIAL_MACHINERY,
	  500, 1, 0, ECON_AGRICULTURE | ECON_MINING
	},{
	  "Metal alloys", 0,
	  Equip::SLOT_CARGO, Equip::METAL_ORE,
	  800, 1, 0, ECON_MINING | ECON_INDUSTRY
	},{
	  "Plastics", 0,
	  Equip::SLOT_CARGO, Equip::CARBON_ORE,
	  1200, 1, 0, ECON_INDUSTRY
	},{
	  "Fruit and Veg", 0,
	  Equip::SLOT_CARGO, Equip::FARM_MACHINERY,
	  1200, 1, 0, ECON_AGRICULTURE
	},{
	  "Animal Meat", 0,
	  Equip::SLOT_CARGO, Equip::FARM_MACHINERY,
	  1800, 1, 0, ECON_AGRICULTURE
	},{
	  "Liquor", 0,
	  Equip::SLOT_CARGO, Equip::FARM_MACHINERY,
	  800, 1, 0, ECON_AGRICULTURE
	},{
	  "Grain", 0,
	  Equip::SLOT_CARGO, Equip::FARM_MACHINERY,
	  1000, 1, 0, ECON_AGRICULTURE
	},{
	  "Textiles", 0,
	  Equip::SLOT_CARGO, Equip::NONE,
	  850, 1, 0, 
	},{
	  "Fertilizer", 0,
	  Equip::SLOT_CARGO, Equip::NONE,
	  400, 1, 0, ECON_INDUSTRY
	},{
	  "Water", 0,
	  Equip::SLOT_CARGO, Equip::NONE,
	  120, 1, 0
	},{
	  "Medicines",0,
	  Equip::SLOT_CARGO, Equip::COMPUTERS,
	  2200, 1, 0, ECON_INDUSTRY,
	},{
	  "Consumer goods",0,
	  Equip::SLOT_CARGO,
	  Equip::PLASTICS,
	  14000, 1, 0, ECON_INDUSTRY,
	},{
	  "Computers",0,
	  Equip::SLOT_CARGO,
	  Equip::ROBOTS,
	  8000, 1, 0, ECON_INDUSTRY,
	},{
	  "Robots",0,
	  Equip::SLOT_CARGO,
	  Equip::INDUSTRIAL_MACHINERY,
	  6300, 1, 0, ECON_INDUSTRY,
	},{
	  "Precious metals", 0,
	  Equip::SLOT_CARGO,
	  Equip::INDUSTRIAL_MACHINERY,
	  18000, 1, 0, ECON_MINING,
	},{
	  "Industrial machinery",0,
	  Equip::SLOT_CARGO,
	  Equip::ROBOTS,
	  1300, 1, 0, ECON_INDUSTRY,
	},{
	  "Farm machinery",0,
	  Equip::SLOT_CARGO,
	  Equip::ROBOTS,
	  1100, 1, 0, ECON_INDUSTRY,
	},{
	  "Air processors",0,
	  Equip::SLOT_CARGO,
	  Equip::ROBOTS,
	  2000, 1, 0, ECON_INDUSTRY,
	},{
	  "Hand weapons",0,
	  Equip::SLOT_CARGO, Equip::NONE,
	  12400, 1, 0, ECON_INDUSTRY,
	},{
	  "Battle weapons",0,
	  Equip::SLOT_CARGO, Equip::INDUSTRIAL_MACHINERY,
	  22000, 1, 0, ECON_INDUSTRY,
	},{
	  "Narcotics",0,
	  Equip::SLOT_CARGO, Equip::NONE,
	  15700, 1, 0, ECON_INDUSTRY,
	},{
	  "Interplanetary Drive",0,
	  Equip::SLOT_ENGINE, Equip::NONE,
	  40000, 1, 0
	},{
	  "Class 1 Hyperdrive",0,
	  Equip::SLOT_ENGINE, Equip::NONE,
	  70000, 4, 1
	},{
	  "Class 2 Hyperdrive",0,
	  Equip::SLOT_ENGINE, Equip::NONE,
	  130000, 10, 2
	},{
	  "Class 3 Hyperdrive",0,
	  Equip::SLOT_ENGINE, Equip::NONE,
	  250000, 20, 3
	},{
	  "Class 4 Hyperdrive",0,
	  Equip::SLOT_ENGINE, Equip::NONE,
	  500000, 40, 4
	},{
	  "Class 5 Hyperdrive",0,
	  Equip::SLOT_ENGINE, Equip::NONE,
	  1000000, 120, 4
	},{
	  "Class 6 Hyperdrive",0,
	  Equip::SLOT_ENGINE, Equip::NONE,
	  2000000, 225, 4
	},{
	  "1MW beam laser",0,
	  Equip::SLOT_LASER, Equip::NONE,
	  60000, 1, 1
	},{
	  "2MW beam laser",0,
	  Equip::SLOT_LASER, Equip::NONE,
	  100000, 1, 2
	},{
	  "4MW beam laser",0,
	  Equip::SLOT_LASER, Equip::NONE,
	  220000, 1, 4
	}
};

