#include "ShipType.h"
#include "Serializer.h"

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
	{ "None",
	  Equip::SLOT_CARGO,
	  0, 0
	},{
	  "Hydrogen",
	  Equip::SLOT_CARGO,
	  5, 1, 0
	},{
	  "Liquid Oxygen",
	  Equip::SLOT_CARGO,
	  20, 1, 0
	},{
	  "Metal ore",
	  Equip::SLOT_CARGO,
	  20, 1, 0
	},{
	  "Oil",
	  Equip::SLOT_CARGO,
	  20, 1, 0
	},{
	  "Metal alloys",
	  Equip::SLOT_CARGO,
	  20, 1, 0
	},{
	  "Plastics",
	  Equip::SLOT_CARGO,
	  20, 1, 0
	},{
	  "Fruit and Veg",
	  Equip::SLOT_CARGO,
	  20, 1, 0
	},{
	  "Animal Meat",
	  Equip::SLOT_CARGO,
	  20, 1, 0
	},{
	  "Liquor",
	  Equip::SLOT_CARGO,
	  20, 1, 0
	},{
	  "Grain",
	  Equip::SLOT_CARGO,
	  20, 1, 0
	},{
	  "Textiles",
	  Equip::SLOT_CARGO,
	  20, 1, 0
	},{
	  "Fertilizer",
	  Equip::SLOT_CARGO,
	  20, 1, 0
	},{
	  "Water",
	  Equip::SLOT_CARGO,
	  8, 1, 0
	},{
	  "Medicines",
	  Equip::SLOT_CARGO,
	  20, 1, 0
	},{
	  "Consumer goods",
	  Equip::SLOT_CARGO,
	  20, 1, 0
	},{
	  "Computers",
	  Equip::SLOT_CARGO,
	  20, 1, 0
	},{
	  "Robots",
	  Equip::SLOT_CARGO,
	  20, 1, 0
	},{
	  "Precious metals",
	  Equip::SLOT_CARGO,
	  500, 1, 0
	},{
	  "Industrial machinery",
	  Equip::SLOT_CARGO,
	  20, 1, 0
	},{
	  "Farm machinery",
	  Equip::SLOT_CARGO,
	  20, 1, 0
	},{
	  "Air processors",
	  Equip::SLOT_CARGO,
	  20, 1, 0
	},{
	  "Hand weapons",
	  Equip::SLOT_CARGO,
	  20, 1, 0
	},{
	  "Battle weapons",
	  Equip::SLOT_CARGO,
	  20, 1, 0
	},{
	  "Narcotics",
	  Equip::SLOT_CARGO,
	  20, 1, 0
	},{
	  "Interplanetary Drive",
	  Equip::SLOT_ENGINE,
	  4000, 1, 0
	},{
	  "Class 1 Hyperdrive",
	  Equip::SLOT_ENGINE,
	  7000, 4, 1
	},{
	  "Class 2 Hyperdrive",
	  Equip::SLOT_ENGINE,
	  13000, 10, 2
	},{
	  "Class 3 Hyperdrive",
	  Equip::SLOT_ENGINE,
	  25000, 20, 3
	},{
	  "Class 4 Hyperdrive",
	  Equip::SLOT_ENGINE,
	  50000, 40, 4
	},{
	  "Class 5 Hyperdrive",
	  Equip::SLOT_ENGINE,
	  100000, 120, 4
	},{
	  "Class 6 Hyperdrive",
	  Equip::SLOT_ENGINE,
	  200000, 225, 4
	},{
	  "1MW beam laser",
	  Equip::SLOT_LASER,
	  6000, 1, 1
	},{
	  "2MW beam laser",
	  Equip::SLOT_LASER,
	  10000, 1, 2
	},{
	  "4MW beam laser",
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

