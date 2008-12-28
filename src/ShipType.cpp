#include "ShipType.h"
#include "Serializer.h"
#include "StarSystem.h"

const ShipType ShipType::types[] = {
	{
		// besides running a wicked corporatist regime in the
		// sirius system, Sirius corporation make a range of
		// lovely starships
		"Sirius Interdictor", 61,
		{ 4e6,-4e6,1e6,-1e6,-1e6,1e6 },
		1e7,
		{
			{ vector3f(0,-0.5,0), vector3f(0,0,-1) },
			{ vector3f(0,-0.5,0), vector3f(0,0,1) }
		},
		{ 50, 1, 2, 0 },
		100, 20,
	}, {
		// john - you should pick names yourself or this happens
		"Ladybird Starfighter",
		62,
		{ 2e6,-2e6,1e6,-1e6,-1e6,1e6 },
		1e7,
		{
			{ vector3f(0,-0.5,0), vector3f(0,0,-1) },
			{ vector3f(0,0,0), vector3f(0,0,1) }
		},
		{ 20, 1, 1, 0 },
		60, 15,
	}, {
		"Flowerfairy Heavy Trader",
		63,
		{ 1e6,-1e6,1e6,-1e6,-1e6,1e6 },
		1e7,
		{
			{ vector3f(0,-0.5,0), vector3f(0,0,-1) },
			{ vector3f(0,0,0), vector3f(0,0,1) }
		},
		{ 200, 1, 2, 0 },
		500, 125,
	}
};

const EquipType EquipType::types[] = {
	{ "None", 0,
	  Equip::SLOT_CARGO,
	  0, 0
	},{
	  "Hydrogen", "Hydrogen is primarily used as a fusion fuel",
	  Equip::SLOT_CARGO,
	  5, 1, 0,
	},{
	  "Liquid Oxygen", "Oxygen is required for life support systems and "
	  "some industrial processes",
	  Equip::SLOT_CARGO,
	  20, 1, 0,
	},{
	  "Metal ore", 0,
	  Equip::SLOT_CARGO,
	  20, 1, 0, ECON_MINING
	},{
	  "Carbon ore", "Carbon ores (coal and oil) are required for the synthesis "
	  "of many useful chemicals, including plastics, synthetic foodstuffs, "
	  "medicines and textiles",
	  Equip::SLOT_CARGO,
	  20, 1, 0, ECON_AGRICULTURE | ECON_MINING
	},{
	  "Metal alloys", 0,
	  Equip::SLOT_CARGO,
	  20, 1, 0, ECON_MINING | ECON_INDUSTRY
	},{
	  "Plastics", 0,
	  Equip::SLOT_CARGO,
	  20, 1, 0, ECON_INDUSTRY
	},{
	  "Fruit and Veg", 0,
	  Equip::SLOT_CARGO,
	  20, 1, 0, ECON_AGRICULTURE
	},{
	  "Animal Meat", 0,
	  Equip::SLOT_CARGO,
	  20, 1, 0, ECON_AGRICULTURE
	},{
	  "Liquor", 0,
	  Equip::SLOT_CARGO,
	  20, 1, 0, ECON_AGRICULTURE
	},{
	  "Grain", 0,
	  Equip::SLOT_CARGO,
	  20, 1, 0, ECON_AGRICULTURE
	},{
	  "Textiles", 0,
	  Equip::SLOT_CARGO,
	  20, 1, 0, 
	},{
	  "Fertilizer", 0,
	  Equip::SLOT_CARGO,
	  20, 1, 0, ECON_INDUSTRY
	},{
	  "Water", 0,
	  Equip::SLOT_CARGO,
	  8, 1, 0
	},{
	  "Medicines",0,
	  Equip::SLOT_CARGO,
	  20, 1, 0, ECON_INDUSTRY
	},{
	  "Consumer goods",0,
	  Equip::SLOT_CARGO,
	  20, 1, 0, ECON_INDUSTRY
	},{
	  "Computers",0,
	  Equip::SLOT_CARGO,
	  20, 1, 0, ECON_INDUSTRY
	},{
	  "Robots",0,
	  Equip::SLOT_CARGO,
	  20, 1, 0, ECON_INDUSTRY
	},{
	  "Precious metals", 0,
	  Equip::SLOT_CARGO,
	  500, 1, 0, ECON_MINING
	},{
	  "Industrial machinery",0,
	  Equip::SLOT_CARGO,
	  20, 1, 0, ECON_INDUSTRY
	},{
	  "Farm machinery",0,
	  Equip::SLOT_CARGO,
	  20, 1, 0, ECON_INDUSTRY
	},{
	  "Air processors",0,
	  Equip::SLOT_CARGO,
	  20, 1, 0, ECON_INDUSTRY
	},{
	  "Hand weapons",0,
	  Equip::SLOT_CARGO,
	  20, 1, 0, ECON_INDUSTRY
	},{
	  "Battle weapons",0,
	  Equip::SLOT_CARGO,
	  20, 1, 0, ECON_INDUSTRY
	},{
	  "Narcotics",0,
	  Equip::SLOT_CARGO,
	  20, 1, 0, ECON_INDUSTRY
	},{
	  "Interplanetary Drive",0,
	  Equip::SLOT_ENGINE,
	  4000, 1, 0
	},{
	  "Class 1 Hyperdrive",0,
	  Equip::SLOT_ENGINE,
	  7000, 4, 1
	},{
	  "Class 2 Hyperdrive",0,
	  Equip::SLOT_ENGINE,
	  13000, 10, 2
	},{
	  "Class 3 Hyperdrive",0,
	  Equip::SLOT_ENGINE,
	  25000, 20, 3
	},{
	  "Class 4 Hyperdrive",0,
	  Equip::SLOT_ENGINE,
	  50000, 40, 4
	},{
	  "Class 5 Hyperdrive",0,
	  Equip::SLOT_ENGINE,
	  100000, 120, 4
	},{
	  "Class 6 Hyperdrive",0,
	  Equip::SLOT_ENGINE,
	  200000, 225, 4
	},{
	  "1MW beam laser",0,
	  Equip::SLOT_LASER,
	  6000, 1, 1
	},{
	  "2MW beam laser",0,
	  Equip::SLOT_LASER,
	  10000, 1, 2
	},{
	  "4MW beam laser",0,
	  Equip::SLOT_LASER,
	  22000, 1, 4
	}
};

void EquipSet::Save()
{
	using namespace Serializer::Write;
	for (int i=0; i<Equip::SLOT_MAX; i++) {
		for (unsigned int j=0; j<equip[i].size(); j++) {
			wr_int(static_cast<int>(equip[i][j]));
		}
	}
}

/*
 * Should have initialised with EquipSet(ShipType::Type) first
 */
void EquipSet::Load()
{
	using namespace Serializer::Read;
	for (int i=0; i<Equip::SLOT_MAX; i++) {
		for (unsigned int j=0; j<equip[i].size(); j++) {
			equip[i][j] = static_cast<Equip::Type>(rd_int());
		}
	}
}

