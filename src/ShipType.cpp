#include "ShipType.h"

const ShipType ShipType::types[] = {
	{
		// besides running a wicked corporatist regime in the
		// sirius system, Sirius corporation make a range of
		// lovely starships
		"Sirius Interdictor", 61,
		{ 4e8,-4e8,1e8,-1e8,-1e8,1e8 },
		1e7,
		{
			{ vector3f(0,-0.5,0), vector3f(0,0,-1) },
			{ vector3f(0,0,0), vector3f(0,0,1) }
		},
		{ 1, 2, 0 },
		100, 20,
	}, {
		// john - you should pick names yourself or this happens
		"Ladybird Starfighter",
		62,
		{ 1e7,-1e7,1e6,-1e6,-1e6,1e6 },
		1e7,
		{
			{ vector3f(0,-0.5,0), vector3f(0,0,-1) },
			{ vector3f(0,0,0), vector3f(0,0,1) }
		},
		{ 1, 1, 0 },
		60, 15,
	}, {
		"Flowerfairy Heavy Trader",
		63,
		{ 1e7,-1e7,1e6,-1e6,-1e6,1e6 },
		1e7,
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
	  0, 0
	},{
	  "Interplanetary Drive",
	  Equip::SLOT_ENGINE,
	  1, 0
	},{
	  "Class 1 Hyperdrive",
	  Equip::SLOT_ENGINE,
	  4, 1
	},{
	  "1MW beam laser",
	  Equip::SLOT_LASER,
	  1, 1
	}
};
