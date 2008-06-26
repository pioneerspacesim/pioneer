#include "ShipType.h"

const ShipType ShipType::types[] = {
	{
		// besides running a wicked corporatist regime in the
		// sirius system, Sirius corporation make a range of
		// lovely starships
		"Sirius Interdictor", 10,
		{ 250,-250,50,-50,-50,50 },
		700.0,
		{
			{ vector3f(0,-0.5,0), vector3f(0,0,-1) },
			{ vector3f(0,0,0), vector3f(0,0,1) }
		}
	}, {
		// john - you should pick names yourself or this happens
		"Ladybird Starfighter",
		13,
		{ 250,-250,50,-50,-50,50 },
		500.0,
		{
			{ vector3f(0,-0.5,0), vector3f(0,0,-1) },
			{ vector3f(0,0,0), vector3f(0,0,1) }
		}
	}, {
		"Flowerfairy Heavy Trader",
		14,
		{ 250,-250,50,-50,-50,50 },
		500.0,
		{
			{ vector3f(0,-0.5,0), vector3f(0,0,-1) },
			{ vector3f(0,0,0), vector3f(0,0,1) }
		}
	}
};

