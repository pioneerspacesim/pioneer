#include "ShipType.h"

const ShipType ShipType::types[] = {
	{
		// besides running a wicked corporatist regime in the
		// sirius system, Sirius corporation make a range of
		// lovely starships
		"Sirius Interdictor", 10,
		{ 250,-250,50,-50,-50,50 },
		2000.0,
		{
			{ vector3f(0,-0.5,0), vector3f(0,0,-1) },
			{ vector3f(0,0,0), vector3f(0,0,1) }
		},
		{ 1, 2, 0 },
		100, 20,
	}, {
		// john - you should pick names yourself or this happens
		"Ladybird Starfighter",
		13,
		{ 250,-250,50,-50,-50,50 },
		500.0,
		{
			{ vector3f(0,-0.5,0), vector3f(0,0,-1) },
			{ vector3f(0,0,0), vector3f(0,0,1) }
		},
		{ 1, 1, 0 },
		60, 15,
	}, {
		"Flowerfairy Heavy Trader",
		14,
		{ 250,-250,50,-50,-50,50 },
		500.0,
		{
			{ vector3f(0,-0.5,0), vector3f(0,0,-1) },
			{ vector3f(0,0,0), vector3f(0,0,1) }
		},
		{ 1, 2, 0 },
		500, 125,
	}
};

const EquipType EquipType::types[] = {
	{ "None",
	  Equip::SLOT_ENGINE,
	  0,
	},{
	  "Interplanetary Drive",
	  Equip::SLOT_ENGINE,
	  1
	},{
	  "Class 1 Hyperdrive",
	  Equip::SLOT_ENGINE,
	  4
	},{
	  "1MW beam laser",
	  Equip::SLOT_LASER,
	  1
	}
};
