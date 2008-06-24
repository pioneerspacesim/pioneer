#include "ShipType.h"

const ShipType ShipType::types[1] = {
	{
		{ 250,-250,50,-50,-50,50 },
		500.0,
		{
			{ vector3f(0,-0.5,0), vector3f(0,0,-1) },
			{ vector3f(0,0,0), vector3f(0,0,1) }
		}
	}
};

