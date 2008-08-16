#include "custom_starsystems.h"


const CustomSBody sol_system[] = {
	{ "Sol", StarSystem::TYPE_STAR_G,
	  -1, fixed(1,1), fixed(1,1), 5700 },
	{ "Mercury", StarSystem::TYPE_PLANET_SMALL,
	  0, fixed(38,100), fixed(55,1000), 340,
	  fixed(387,1000), fixed(205,1000), DEG2RAD(7.0), fixed(59,1) },
	{ "Venus", StarSystem::TYPE_PLANET_CO2_THICK_ATMOS,
	  0, fixed(95,100), fixed(815,1000), 735,
	  fixed(723,1000), fixed(7,1000), DEG2RAD(3.39), fixed(243,1) },
	{ "Earth", StarSystem::TYPE_PLANET_INDIGENOUS_LIFE,
	  0, fixed(1,1), fixed(1,1), 288,
	  fixed(1,1), fixed(167,10000), 0, fixed(1,1) },
	{ "Moon", StarSystem::TYPE_PLANET_DWARF,
	  3, fixed(273,1000), fixed(12,1000), 220,
	  fixed(257,100000), fixed(549,10000), DEG2RAD(5.145), fixed(273,10) },
	{ "Mars", StarSystem::TYPE_PLANET_SMALL,
	  0, fixed(533,1000), fixed(107,1000), 227,
	  fixed(152,100), fixed(933,10000), DEG2RAD(1.85), fixed(1027,1000) },
	{ "Jupiter", StarSystem::TYPE_PLANET_LARGE_GAS_GIANT,
	  0, fixed(11,1), fixed(3178,10), 165,
	  fixed(5204,1000), fixed(488,10000), DEG2RAD(1.305), fixed(4,10) },
	{ "Saturn", StarSystem::TYPE_PLANET_MEDIUM_GAS_GIANT,
	  0, fixed(9,1), fixed(95152,100), 134,
	  fixed(9582,1000), fixed(557,10000), DEG2RAD(2.485), fixed(4,10) },
	{ "Uranus", StarSystem::TYPE_PLANET_SMALL_GAS_GIANT,
	  0, fixed(4,1), fixed(145,10), 76,
	  fixed(19229,1000), fixed(444,10000), DEG2RAD(0.772), fixed(7,10) },
	{ "Neptune", StarSystem::TYPE_PLANET_SMALL_GAS_GIANT,
	  0, fixed(38,10), fixed(17147,100), 72,
	  fixed(30104,1000), fixed(112,10000), DEG2RAD(1.768), fixed(75,100) },
	// moons of jupiter
	{ "Io", StarSystem::TYPE_PLANET_HIGHLY_VOLCANIC,
	  6, fixed(286,1000), fixed(15,1000), 130,
	  fixed(282,100000), fixed(41,10000), DEG2RAD(2.21), fixed(177,10) },
	{ "Europa", StarSystem::TYPE_PLANET_WATER,
	  6, fixed(245,1000), fixed(8,1000), 102,
	  fixed(441,100000), fixed(9,1000), 0.0, fixed(355,10) },
	  
	{ 0 }
};

const CustomSystem custom_systems[] = {
	{ "Sol", sol_system, StarSystem::TYPE_STAR_G, 0, 0, vector3f(.5, .5, .5) },
	{ "Barnard's Star", 0, StarSystem::TYPE_STAR_M, 0, 0, vector3f(.2, .3, .2) },
	{ "Ross 154", 0, StarSystem::TYPE_STAR_M, 0, 0, vector3f(.1, .6, -.2) },
	{ 0 }
};
