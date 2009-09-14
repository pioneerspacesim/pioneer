#include "EquipType.h"
#include "StarSystem.h"

const EquipType EquipType::types[Equip::TYPE_MAX] = {
	{ "None", 0,
	  Equip::SLOT_CARGO, {},
	  0, 0
	},{
	  "Hydrogen", "Hydrogen is primarily used as a fusion fuel",
	  Equip::SLOT_CARGO, {},
	  100, 1, 0, ECON_MINING
	},{
	  "Liquid Oxygen", "Oxygen is required for life support systems and "
	  "some industrial processes",
	  Equip::SLOT_CARGO, { Equip::WATER, Equip::INDUSTRIAL_MACHINERY },
	  150, 1, 0, ECON_MINING
	},{
	  "Metal ore", 0,
	  Equip::SLOT_CARGO, { Equip::MINING_MACHINERY },
	  300, 1, 0, ECON_MINING
	},{
	  "Carbon ore", "Carbon ores (coal and oil) are required for the synthesis "
	  "of many useful chemicals, including plastics, synthetic foodstuffs, "
	  "medicines and textiles",
	  Equip::SLOT_CARGO, { Equip::MINING_MACHINERY },
	  500, 1, 0, ECON_MINING
	},{
	  "Metal alloys", 0,
	  Equip::SLOT_CARGO, { Equip::METAL_ORE, Equip::INDUSTRIAL_MACHINERY },
	  800, 1, 0, ECON_INDUSTRY
	},{
	  "Plastics", 0,
	  Equip::SLOT_CARGO, { Equip::CARBON_ORE, Equip::INDUSTRIAL_MACHINERY },
	  1200, 1, 0, ECON_INDUSTRY
	},{
	  "Fruit and Veg", 0,
	  Equip::SLOT_CARGO, { Equip::FARM_MACHINERY, Equip::FERTILIZER },
	  1200, 1, 0, ECON_AGRICULTURE
	},{
	  "Animal Meat", 0,
	  Equip::SLOT_CARGO, { Equip::FARM_MACHINERY, Equip::FERTILIZER },
	  1800, 1, 0, ECON_AGRICULTURE
	},{
	  "Liquor", 0,
	  Equip::SLOT_CARGO, { Equip::FARM_MACHINERY, Equip::FERTILIZER },
	  800, 1, 0, ECON_AGRICULTURE
	},{
	  "Grain", 0,
	  Equip::SLOT_CARGO, { Equip::FARM_MACHINERY, Equip::FERTILIZER },
	  1000, 1, 0, ECON_AGRICULTURE
	},{
	  "Textiles", 0,
	  Equip::SLOT_CARGO, { Equip::PLASTICS },
	  850, 1, 0, ECON_INDUSTRY
	},{
	  "Fertilizer", 0,
	  Equip::SLOT_CARGO, { Equip::CARBON_ORE },
	  400, 1, 0, ECON_INDUSTRY
	},{
	  "Water", 0,
	  Equip::SLOT_CARGO, { Equip::MINING_MACHINERY },
	  120, 1, 0, ECON_MINING
	},{
	  "Medicines",0,
	  Equip::SLOT_CARGO, { Equip::COMPUTERS, Equip::CARBON_ORE },
	  2200, 1, 0, ECON_INDUSTRY,
	},{
	  "Consumer goods",0,
	  Equip::SLOT_CARGO, { Equip::PLASTICS, Equip::TEXTILES },
	  14000, 1, 0, ECON_INDUSTRY,
	},{
	  "Computers",0,
	  Equip::SLOT_CARGO, { Equip::PRECIOUS_METALS, Equip::INDUSTRIAL_MACHINERY },
	  8000, 1, 0, ECON_INDUSTRY,
	},{
	  "Robots",0,
	  Equip::SLOT_CARGO, { Equip::PLASTICS, Equip::COMPUTERS },
	  6300, 1, 0, ECON_INDUSTRY,
	},{
	  "Precious metals", 0,
	  Equip::SLOT_CARGO, { Equip::MINING_MACHINERY },
	  18000, 1, 0, ECON_MINING,
	},{
	  "Industrial machinery",0,
	  Equip::SLOT_CARGO, { Equip::METAL_ALLOYS, Equip::ROBOTS },
	  1300, 1, 0, ECON_INDUSTRY,
	},{
	  "Farm machinery",0,
	  Equip::SLOT_CARGO, { Equip::METAL_ALLOYS, Equip::ROBOTS },
	  1100, 1, 0, ECON_INDUSTRY,
	},{
	  "Mining machinery",0,
	  Equip::SLOT_CARGO, { Equip::METAL_ALLOYS, Equip::ROBOTS },
	  1200, 1, 0, ECON_INDUSTRY,
	},{
	  "Air processors",0,
	  Equip::SLOT_CARGO, { Equip::PLASTICS, Equip::INDUSTRIAL_MACHINERY },
	  2000, 1, 0, ECON_INDUSTRY,
	},{
	  "Hand weapons",0,
	  Equip::SLOT_CARGO, { Equip::COMPUTERS },
	  12400, 1, 0, ECON_INDUSTRY,
	},{
	  "Battle weapons",0,
	  Equip::SLOT_CARGO, { Equip::INDUSTRIAL_MACHINERY, Equip::METAL_ALLOYS },
	  22000, 1, 0, ECON_INDUSTRY,
	},{
	  "Narcotics",0,
	  Equip::SLOT_CARGO, {},
	  15700, 1, 0, ECON_INDUSTRY,
	},{
	  "Interplanetary Drive",0,
	  Equip::SLOT_ENGINE, {},
	  40000, 1, 0
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
	  1000000, 120, 4
	},{
	  "Class 6 Hyperdrive",0,
	  Equip::SLOT_ENGINE, {},
	  2000000, 225, 4
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

