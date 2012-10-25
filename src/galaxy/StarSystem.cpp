// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "StarSystem.h"
#include "Sector.h"
#include "Factions.h"

#include "Serializer.h"
#include "Pi.h"
#include "LuaNameGen.h"
#include <map>
#include "utils.h"
#include "Lang.h"
#include "StringF.h"

#define CELSIUS	273.15
//#define DEBUG_DUMP

// minimum moon mass a little under Europa's
static const fixed MIN_MOON_MASS = fixed(1,30000); // earth masses
static const fixed MIN_MOON_DIST = fixed(15,10000); // AUs
static const fixed MAX_MOON_DIST = fixed(2, 100); // AUs
// if binary stars have separation s, planets can have stable
// orbits at (0.5 * s * SAFE_DIST_FROM_BINARY)
static const fixed SAFE_DIST_FROM_BINARY = fixed(5,1);
static const fixed PLANET_MIN_SEPARATION = fixed(135,100);

// very crudely
static const fixed AU_SOL_RADIUS = fixed(305,65536);
static const fixed AU_EARTH_RADIUS = fixed(3, 65536);

// indexed by enum type turd
float StarSystem::starColors[][3] = {
	{ 0, 0, 0 }, // gravpoint
	{ 0.5f, 0.0f, 0.0f }, // brown dwarf
	{ 0.4f, 0.4f, 0.8f }, // white dwarf
	{ 1.0f, 0.2f, 0.0f }, // M
	{ 1.0f, 0.6f, 0.1f }, // K
	{ 1.0f, 1.0f, 0.4f }, // G
	{ 1.0f, 1.0f, 0.8f }, // F
	{ 1.0f, 1.0f, 1.0f }, // A
	{ 0.7f, 0.7f, 1.0f }, // B
	{ 1.0f, 0.7f, 1.0f }, // O
	{ 1.0f, 0.2f, 0.0f }, // M Giant
	{ 1.0f, 0.6f, 0.1f }, // K Giant
	{ 1.0f, 1.0f, 0.4f }, // G Giant
	{ 1.0f, 1.0f, 0.8f }, // F Giant
	{ 1.0f, 1.0f, 1.0f }, // A Giant
	{ 0.7f, 0.7f, 1.0f }, // B Giant
	{ 1.0f, 0.7f, 1.0f }, // O Giant
	{ 1.0f, 0.2f, 0.0f }, // M Super Giant
	{ 1.0f, 0.6f, 0.1f }, // K Super Giant
	{ 1.0f, 1.0f, 0.4f }, // G Super Giant
	{ 1.0f, 1.0f, 0.8f }, // F Super Giant
	{ 1.0f, 1.0f, 1.0f }, // A Super Giant
	{ 0.7f, 0.7f, 1.0f }, // B Super Giant
	{ 1.0f, 0.7f, 1.0f }, // O Super Giant
	{ 1.0f, 0.2f, 0.0f }, // M Hyper Giant
	{ 1.0f, 0.6f, 0.1f }, // K Hyper Giant
	{ 1.0f, 1.0f, 0.4f }, // G Hyper Giant
	{ 1.0f, 1.0f, 0.8f }, // F Hyper Giant
	{ 1.0f, 1.0f, 1.0f }, // A Hyper Giant
	{ 0.7f, 0.7f, 1.0f }, // B Hyper Giant
	{ 1.0f, 0.7f, 1.0f }, // O Hyper Giant
	{ 1.0f, 0.2f, 0.0f }, // Red/M Wolf Rayet Star
	{ 0.7f, 0.7f, 1.0f }, // Blue/B Wolf Rayet Star
	{ 1.0f, 0.7f, 1.0f }, // Purple-Blue/O Wolf Rayet Star
	{ 0.3f, 0.7f, 0.3f }, // Stellar Blackhole
	{ 0.2f, 0.9f, 0.2f }, // Intermediate mass Black-hole
	{ 0.0f, 1.0f, 0.0f }, // Super massive black hole
};

// indexed by enum type turd
float StarSystem::starRealColors[][3] = {
	{ 0, 0, 0 }, // gravpoint
	{ 0.5f, 0.0f, 0.0f }, // brown dwarf
	{ 1.0f, 1.0f, 1.0f }, // white dwarf
	{ 1.0f, 0.5f, 0.2f }, // M
	{ 1.0f, 1.0f, 0.4f }, // K
	{ 1.0f, 1.0f, 0.95f }, // G
	{ 1.0f, 1.0f, 1.0f }, // F
	{ 1.0f, 1.0f, 1.0f }, // A
	{ 0.8f, 0.8f, 1.0f }, // B
	{ 1.0f, 0.8f, 1.0f },  // O
	{ 1.0f, 0.5f, 0.2f }, // M Giant
	{ 1.0f, 1.0f, 0.4f }, // K Giant
	{ 1.0f, 1.0f, 0.95f }, // G Giant
	{ 1.0f, 1.0f, 1.0f }, // F Giant
	{ 1.0f, 1.0f, 1.0f }, // A Giant
	{ 0.8f, 0.8f, 1.0f }, // B Giant
	{ 1.0f, 0.8f, 1.0f },  // O Giant
	{ 1.0f, 0.5f, 0.2f }, // M Super Giant
	{ 1.0f, 1.0f, 0.4f }, // K Super Giant
	{ 1.0f, 1.0f, 0.95f }, // G Super Giant
	{ 1.0f, 1.0f, 1.0f }, // F Super Giant
	{ 1.0f, 1.0f, 1.0f }, // A Super Giant
	{ 0.8f, 0.8f, 1.0f }, // B Super Giant
	{ 1.0f, 0.8f, 1.0f },  // O Super Giant
	{ 1.0f, 0.5f, 0.2f }, // M Hyper Giant
	{ 1.0f, 1.0f, 0.4f }, // K Hyper Giant
	{ 1.0f, 1.0f, 0.95f }, // G Hyper Giant
	{ 1.0f, 1.0f, 1.0f }, // F Hyper Giant
	{ 1.0f, 1.0f, 1.0f }, // A Hyper Giant
	{ 0.8f, 0.8f, 1.0f }, // B Hyper Giant
	{ 1.0f, 0.8f, 1.0f },  // O Hyper Giant
	{ 1.0f, 0.6f, 0.6f }, // M WF
	{ 0.8f, 0.8f, 1.0f }, // B WF
	{ 1.0f, 0.8f, 1.0f },  // O WF
	{ 1.0f, 1.0f, 1.0f },  // small Black hole
	{ 0.06f, 0.0f, 0.08f }, // med BH
	{ 0.04f, 0.0f, 0.06f }, // massive BH
};

double StarSystem::starLuminosities[] = {
	0,
	0.0003, // brown dwarf
	0.1, // white dwarf
	0.08, // M0
	0.38, // K0
	1.2, // G0
	5.1, // F0
	24.0, // A0
	100.0, // B0
	200.0, // O5
	1000.0, // M0 Giant
	2000.0, // K0 Giant
	4000.0, // G0 Giant
	6000.0, // F0 Giant
	8000.0, // A0 Giant
	9000.0, // B0 Giant
	12000.0, // O5 Giant
	12000.0, // M0 Super Giant
	14000.0, // K0 Super Giant
	18000.0, // G0 Super Giant
	24000.0, // F0 Super Giant
	30000.0, // A0 Super Giant
	50000.0, // B0 Super Giant
	100000.0, // O5 Super Giant
	125000.0, // M0 Hyper Giant
	150000.0, // K0 Hyper Giant
	175000.0, // G0 Hyper Giant
	200000.0, // F0 Hyper Giant
	200000.0, // A0 Hyper Giant
	200000.0, // B0 Hyper Giant
	200000.0, // O5 Hyper Giant
	50000.0, // M WF
	100000.0, // B WF
	200000.0, // O WF
	0.0003, // Stellar Black hole
	0.00003, // IM Black hole
	0.000003, // Supermassive Black hole
};

float StarSystem::starScale[] = {  // Used in sector view
	0,
	0.6f, // brown dwarf
	0.5f, // white dwarf
	0.7f, // M
	0.8f, // K
	0.8f, // G
	0.9f, // F
	1.0f, // A
	1.1f, // B
	1.1f, // O
	1.3f, // M Giant
	1.2f, // K G
	1.2f, // G G
	1.2f, // F G
	1.1f, // A G
	1.1f, // B G
	1.2f, // O G
	1.8f, // M Super Giant
	1.6f, // K SG
	1.5f, // G SG
	1.5f, // F SG
	1.4f, // A SG
	1.3f, // B SG
	1.3f, // O SG
	2.5f, // M Hyper Giant
	2.2f, // K HG
	2.2f, // G HG
	2.1f, // F HG
	2.1f, // A HG
	2.0f, // B HG
	1.9f, // O HG
	1.1f, // M WF
	1.3f, // B WF
	1.6f, // O WF
	1.0f, // Black hole
	2.5f, // Intermediate-mass blackhole
	4.0f  // Supermassive blackhole
};

fixed StarSystem::starMetallicities[] = {
	fixed(1,1), // GRAVPOINT - for planets that orbit them
	fixed(9,10), // brown dwarf
	fixed(5,10), // white dwarf
	fixed(7,10), // M0
	fixed(6,10), // K0
	fixed(5,10), // G0
	fixed(4,10), // F0
	fixed(3,10), // A0
	fixed(2,10), // B0
	fixed(1,10), // O5
	fixed(8,10), // M0 Giant
	fixed(65,100), // K0 Giant
	fixed(55,100), // G0 Giant
	fixed(4,10), // F0 Giant
	fixed(3,10), // A0 Giant
	fixed(2,10), // B0 Giant
	fixed(1,10), // O5 Giant
	fixed(9,10), // M0 Super Giant
	fixed(7,10), // K0 Super Giant
	fixed(6,10), // G0 Super Giant
	fixed(4,10), // F0 Super Giant
	fixed(3,10), // A0 Super Giant
	fixed(2,10), // B0 Super Giant
	fixed(1,10), // O5 Super Giant
	fixed(1,1), // M0 Hyper Giant
	fixed(7,10), // K0 Hyper Giant
	fixed(6,10), // G0 Hyper Giant
	fixed(4,10), // F0 Hyper Giant
	fixed(3,10), // A0 Hyper Giant
	fixed(2,10), // B0 Hyper Giant
	fixed(1,10), // O5 Hyper Giant
	fixed(1,1), // M WF
	fixed(8,10), // B WF
	fixed(6,10), // O WF
	fixed(1,1), //  S BH	Blackholes, give them high metallicity,
	fixed(1,1), // IM BH	so any rocks that happen to be there
	fixed(1,1)  // SM BH	may be mining hotspots. FUN :)
};

static const struct StarTypeInfo {
	SystemBody::BodySuperType supertype;
	int mass[2]; // min,max % sol for stars, unused for planets
	int radius[2]; // min,max % sol radii for stars, % earth radii for planets
	int tempMin, tempMax;
} starTypeInfo[] = {
	{
		SystemBody::SUPERTYPE_NONE, {}, {},
        0, 0
	}, {
		SystemBody::SUPERTYPE_STAR, //Brown Dwarf
		{2,8}, {10,30},
		1000, 2000
	}, {
		SystemBody::SUPERTYPE_STAR,  //white dwarf
		{20,100}, {1,2},
		4000, 40000
	}, {
		SystemBody::SUPERTYPE_STAR, //M
		{10,47}, {30,60},
		2000, 3500
	}, {
		SystemBody::SUPERTYPE_STAR, //K
		{50,78}, {60,100},
		3500, 5000
	}, {
		SystemBody::SUPERTYPE_STAR, //G
		{80,110}, {80,120},
		5000, 6000
	}, {
		SystemBody::SUPERTYPE_STAR, //F
		{115,170}, {110,150},
		6000, 7500
	}, {
		SystemBody::SUPERTYPE_STAR, //A
		{180,320}, {120,220},
		7500, 10000
	}, {
		SystemBody::SUPERTYPE_STAR,  //B
		{200,300}, {120,290},
		10000, 30000
	}, {
		SystemBody::SUPERTYPE_STAR, //O
		{300,400}, {200,310},
		30000, 60000
	}, {
		SystemBody::SUPERTYPE_STAR, //M Giant
		{60,357}, {2000,5000},
		2500, 3500
	}, {
		SystemBody::SUPERTYPE_STAR, //K Giant
		{125,500}, {1500,3000},
		3500, 5000
	}, {
		SystemBody::SUPERTYPE_STAR, //G Giant
		{200,800}, {1000,2000},
		5000, 6000
	}, {
		SystemBody::SUPERTYPE_STAR, //F Giant
		{250,900}, {800,1500},
		6000, 7500
	}, {
		SystemBody::SUPERTYPE_STAR, //A Giant
		{400,1000}, {600,1000},
		7500, 10000
	}, {
		SystemBody::SUPERTYPE_STAR,  //B Giant
		{500,1000}, {600,1000},
		10000, 30000
	}, {
		SystemBody::SUPERTYPE_STAR, //O Giant
		{600,1200}, {600,1000},
		30000, 60000
	}, {
		SystemBody::SUPERTYPE_STAR, //M Super Giant
		{1050,5000}, {7000,15000},
		2500, 3500
	}, {
		SystemBody::SUPERTYPE_STAR, //K Super Giant
		{1100,5000}, {5000,9000},
		3500, 5000
	}, {
		SystemBody::SUPERTYPE_STAR, //G Super Giant
		{1200,5000}, {4000,8000},
		5000, 6000
	}, {
		SystemBody::SUPERTYPE_STAR, //F Super Giant
		{1500,6000}, {3500,7000},
		6000, 7500
	}, {
		SystemBody::SUPERTYPE_STAR, //A Super Giant
		{2000,8000}, {3000,6000},
		7500, 10000
	}, {
		SystemBody::SUPERTYPE_STAR,  //B Super Giant
		{3000,9000}, {2500,5000},
		10000, 30000
	}, {
		SystemBody::SUPERTYPE_STAR, //O Super Giant
		{5000,10000}, {2000,4000},
		30000, 60000
	}, {
		SystemBody::SUPERTYPE_STAR, //M Hyper Giant
		{5000,15000}, {20000,40000},
		2500, 3500
	}, {
		SystemBody::SUPERTYPE_STAR, //K Hyper Giant
		{5000,17000}, {17000,25000},
		3500, 5000
	}, {
		SystemBody::SUPERTYPE_STAR, //G Hyper Giant
		{5000,18000}, {14000,20000},
		5000, 6000
	}, {
		SystemBody::SUPERTYPE_STAR, //F Hyper Giant
		{5000,19000}, {12000,17500},
		6000, 7500
	}, {
		SystemBody::SUPERTYPE_STAR, //A Hyper Giant
		{5000,20000}, {10000,15000},
		7500, 10000
	}, {
		SystemBody::SUPERTYPE_STAR,  //B Hyper Giant
		{5000,23000}, {6000,10000},
		10000, 30000
	}, {
		SystemBody::SUPERTYPE_STAR, //O Hyper Giant
		{10000,30000}, {4000,7000},
		30000, 60000
	}, {
		SystemBody::SUPERTYPE_STAR,  // M WF
		{2000,5000}, {2500,5000},
		25000, 35000
	}, {
		SystemBody::SUPERTYPE_STAR,  // B WF
		{2000,7500}, {2500,5000},
		35000, 45000
	}, {
		SystemBody::SUPERTYPE_STAR,  // O WF
		{2000,10000}, {2500,5000},
		45000, 60000
	}, {
		SystemBody::SUPERTYPE_STAR,  // S BH
		{20,2000}, {0,0},	// XXX black holes are < 1 Sol radii big; this is clamped to a non-zero value later
		10, 24
	}, {
		SystemBody::SUPERTYPE_STAR,  // IM BH
		{900000,1000000}, {100,500},
		1, 10
	}, {
		SystemBody::SUPERTYPE_STAR,  // SM BH
		{2000000,5000000}, {10000,20000},
		10, 24
	}
/*	}, {
		SystemBody::SUPERTYPE_GAS_GIANT,
		{}, 950, Lang::MEDIUM_GAS_GIANT,
	}, {
		SystemBody::SUPERTYPE_GAS_GIANT,
		{}, 1110, Lang::LARGE_GAS_GIANT,
	}, {
		SystemBody::SUPERTYPE_GAS_GIANT,
		{}, 1500, Lang::VERY_LARGE_GAS_GIANT,
	}, {
		SystemBody::SUPERTYPE_ROCKY_PLANET,
		{}, 1, Lang::ASTEROID,
		"icons/object_planet_asteroid.png"
	}, {
		SystemBody::SUPERTYPE_ROCKY_PLANET,
		{}, 2, "Large asteroid",
	}, {
		SystemBody::SUPERTYPE_ROCKY_PLANET,
		{}, 26, "Small, rocky dwarf planet", // moon radius
	}, {
		SystemBody::SUPERTYPE_ROCKY_PLANET,
		{}, 26, "Small, rocky dwarf planet", // dwarf2 for moon-like colours
	}, {
		SystemBody::SUPERTYPE_ROCKY_PLANET,
		{}, 52, "Small, rocky planet with a thin atmosphere", // mars radius
	}, {
		SystemBody::SUPERTYPE_ROCKY_PLANET,
		{}, 100, "Rocky frozen planet with a thin nitrogen atmosphere", // earth radius
	}, {
		SystemBody::SUPERTYPE_ROCKY_PLANET,
		{}, 100, "Dead world that once housed it's own intricate ecosystem.", // earth radius
	}, {
		SystemBody::SUPERTYPE_ROCKY_PLANET,
		{}, 100, "Rocky planet with a carbon dioxide atmosphere",
	}, {
		SystemBody::SUPERTYPE_ROCKY_PLANET,
		{}, 100, "Rocky planet with a methane atmosphere",
	}, {
		SystemBody::SUPERTYPE_ROCKY_PLANET,
		{}, 100, "Water world with vast oceans and a thick nitrogen atmosphere",
	}, {
		SystemBody::SUPERTYPE_ROCKY_PLANET,
		{}, 100, "Rocky planet with a thick carbon dioxide atmosphere",
	}, {
		SystemBody::SUPERTYPE_ROCKY_PLANET,
		{}, 100, "Rocky planet with a thick methane atmosphere",
	}, {
		SystemBody::SUPERTYPE_ROCKY_PLANET,
		{}, 100, "Highly volcanic world",
	}, {
		SystemBody::SUPERTYPE_ROCKY_PLANET,
		{}, 100, "World with indigenous life and an oxygen atmosphere",
	}, {
		SystemBody::SUPERTYPE_ROCKY_PLANET,
		{}, 60, "Marginal terraformed world with minimal plant life",
	}, {
		SystemBody::SUPERTYPE_ROCKY_PLANET,
		{}, 90, "Fully terraformed world with introduced species from numerous successful colonies",
	}, {
		SystemBody::SUPERTYPE_STARPORT,
		{}, 0, Lang::ORBITAL_STARPORT,
	}, {
		SystemBody::SUPERTYPE_STARPORT,
		{}, 0, Lang::STARPORT,
	}*/
};

SystemBody::BodySuperType SystemBody::GetSuperType() const
{
	switch (type) {
		case TYPE_BROWN_DWARF:
		case TYPE_WHITE_DWARF:
		case TYPE_STAR_M:
		case TYPE_STAR_K:
		case TYPE_STAR_G:
		case TYPE_STAR_F:
		case TYPE_STAR_A:
		case TYPE_STAR_B:
		case TYPE_STAR_O:
		case TYPE_STAR_M_GIANT:
		case TYPE_STAR_K_GIANT:
		case TYPE_STAR_G_GIANT:
		case TYPE_STAR_F_GIANT:
		case TYPE_STAR_A_GIANT:
		case TYPE_STAR_B_GIANT:
		case TYPE_STAR_O_GIANT:
		case TYPE_STAR_M_SUPER_GIANT:
		case TYPE_STAR_K_SUPER_GIANT:
		case TYPE_STAR_G_SUPER_GIANT:
		case TYPE_STAR_F_SUPER_GIANT:
		case TYPE_STAR_A_SUPER_GIANT:
		case TYPE_STAR_B_SUPER_GIANT:
		case TYPE_STAR_O_SUPER_GIANT:
		case TYPE_STAR_M_HYPER_GIANT:
		case TYPE_STAR_K_HYPER_GIANT:
		case TYPE_STAR_G_HYPER_GIANT:
		case TYPE_STAR_F_HYPER_GIANT:
		case TYPE_STAR_A_HYPER_GIANT:
		case TYPE_STAR_B_HYPER_GIANT:
		case TYPE_STAR_O_HYPER_GIANT:
		case TYPE_STAR_M_WF:
		case TYPE_STAR_B_WF:
		case TYPE_STAR_O_WF:
		case TYPE_STAR_S_BH:
		case TYPE_STAR_IM_BH:
		case TYPE_STAR_SM_BH:
		     return SUPERTYPE_STAR;
		case TYPE_PLANET_GAS_GIANT:
		     return SUPERTYPE_GAS_GIANT;
		case TYPE_PLANET_ASTEROID:
		case TYPE_PLANET_TERRESTRIAL:
		     return SUPERTYPE_ROCKY_PLANET;
		case TYPE_STARPORT_ORBITAL:
		case TYPE_STARPORT_SURFACE:
		     return SUPERTYPE_STARPORT;
		case TYPE_GRAVPOINT:
             return SUPERTYPE_NONE;
        default:
             fprintf( stderr, "Warning: Invalid SuperBody Type found.\n");
             return SUPERTYPE_NONE;
	}
}

std::string SystemBody::GetAstroDescription() const
{
	switch (type) {
	case TYPE_BROWN_DWARF: return Lang::BROWN_DWARF;
	case TYPE_WHITE_DWARF: return Lang::WHITE_DWARF;
	case TYPE_STAR_M: return Lang::STAR_M;
	case TYPE_STAR_K: return Lang::STAR_K;
	case TYPE_STAR_G: return Lang::STAR_G;
	case TYPE_STAR_F: return Lang::STAR_F;
	case TYPE_STAR_A: return Lang::STAR_A;
	case TYPE_STAR_B: return Lang::STAR_B;
	case TYPE_STAR_O: return Lang::STAR_O;
	case TYPE_STAR_M_GIANT: return Lang::STAR_M_GIANT;
	case TYPE_STAR_K_GIANT: return Lang::STAR_K_GIANT;
	case TYPE_STAR_G_GIANT: return Lang::STAR_G_GIANT;
	case TYPE_STAR_F_GIANT: return Lang::STAR_AF_GIANT;
	case TYPE_STAR_A_GIANT: return Lang::STAR_AF_GIANT;
	case TYPE_STAR_B_GIANT: return Lang::STAR_B_GIANT;
	case TYPE_STAR_O_GIANT: return Lang::STAR_O_GIANT;
	case TYPE_STAR_M_SUPER_GIANT: return Lang::STAR_M_SUPER_GIANT;
	case TYPE_STAR_K_SUPER_GIANT: return Lang::STAR_K_SUPER_GIANT;
	case TYPE_STAR_G_SUPER_GIANT: return Lang::STAR_G_SUPER_GIANT;
	case TYPE_STAR_F_SUPER_GIANT: return Lang::STAR_AF_SUPER_GIANT;
	case TYPE_STAR_A_SUPER_GIANT: return Lang::STAR_AF_SUPER_GIANT;
	case TYPE_STAR_B_SUPER_GIANT: return Lang::STAR_B_SUPER_GIANT;
	case TYPE_STAR_O_SUPER_GIANT: return Lang::STAR_O_SUPER_GIANT;
	case TYPE_STAR_M_HYPER_GIANT: return Lang::STAR_M_HYPER_GIANT;
	case TYPE_STAR_K_HYPER_GIANT: return Lang::STAR_K_HYPER_GIANT;
	case TYPE_STAR_G_HYPER_GIANT: return Lang::STAR_G_HYPER_GIANT;
	case TYPE_STAR_F_HYPER_GIANT: return Lang::STAR_AF_HYPER_GIANT;
	case TYPE_STAR_A_HYPER_GIANT: return Lang::STAR_AF_HYPER_GIANT;
	case TYPE_STAR_B_HYPER_GIANT: return Lang::STAR_B_HYPER_GIANT;
	case TYPE_STAR_O_HYPER_GIANT: return Lang::STAR_O_HYPER_GIANT;
	case TYPE_STAR_M_WF: return Lang::STAR_M_WF;
	case TYPE_STAR_B_WF: return Lang::STAR_B_WF;
	case TYPE_STAR_O_WF: return Lang::STAR_O_WF;
	case TYPE_STAR_S_BH: return Lang::STAR_S_BH;
	case TYPE_STAR_IM_BH: return Lang::STAR_IM_BH;
	case TYPE_STAR_SM_BH: return Lang::STAR_SM_BH;
	case TYPE_PLANET_GAS_GIANT:
		if (mass > 800) return Lang::VERY_LARGE_GAS_GIANT;
		if (mass > 300) return Lang::LARGE_GAS_GIANT;
		if (mass > 80) return Lang::MEDIUM_GAS_GIANT;
		else return Lang::SMALL_GAS_GIANT;
	case TYPE_PLANET_ASTEROID: return Lang::ASTEROID;
	case TYPE_PLANET_TERRESTRIAL: {
		std::string s;
		if (mass > fixed(2,1)) s = Lang::MASSIVE;
		else if (mass > fixed(3,2)) s = Lang::LARGE;
		else if (mass < fixed(1,10)) s = Lang::TINY;
		else if (mass < fixed(1,5)) s = Lang::SMALL;

		if (m_volcanicity > fixed(7,10)) {
			if (s.size()) s += Lang::COMMA_HIGHLY_VOLCANIC;
			else s = Lang::HIGHLY_VOLCANIC;
		}

		if (m_volatileIces + m_volatileLiquid > fixed(4,5)) {
			if (m_volatileIces > m_volatileLiquid) {
				if (averageTemp < fixed(250)) {
					s += Lang::ICE_WORLD;
				} else s += Lang::ROCKY_PLANET;
			} else {
				if (averageTemp < fixed(250)) {
					s += Lang::ICE_WORLD;
				} else {
					s += Lang::OCEANICWORLD;
				}
			}
		} else if (m_volatileLiquid > fixed(2,5)){
			if (averageTemp > fixed(250)) {
				s += Lang::PLANET_CONTAINING_LIQUID_WATER;
			} else {
				s += Lang::PLANET_WITH_SOME_ICE;
			}
		} else if (m_volatileLiquid > fixed(1,5)){
			s += Lang::ROCKY_PLANET_CONTAINING_COME_LIQUIDS;
		} else {
			s += Lang::ROCKY_PLANET;
		}

		if (m_volatileGas < fixed(1,100)) {
			s += Lang::WITH_NO_SIGNIFICANT_ATMOSPHERE;
		} else {
			std::string thickness;
			if (m_volatileGas < fixed(1,10)) thickness = Lang::TENUOUS;
			else if (m_volatileGas < fixed(1,5)) thickness = Lang::THIN;
			else if (m_volatileGas < fixed(2,1)) {}
			else if (m_volatileGas < fixed(4,1)) thickness = Lang::THICK;
			else thickness = Lang::VERY_DENSE;

			if (m_atmosOxidizing > fixed(95,100)) {
				s += Lang::WITH_A+thickness+Lang::O2_ATMOSPHERE;
			} else if (m_atmosOxidizing > fixed(7,10)) {
				s += Lang::WITH_A+thickness+Lang::CO2_ATMOSPHERE;
			} else if (m_atmosOxidizing > fixed(65,100)) {
				s += Lang::WITH_A+thickness+Lang::CO_ATMOSPHERE;
			} else if (m_atmosOxidizing > fixed(55,100)) {
				s += Lang::WITH_A+thickness+Lang::CH4_ATMOSPHERE;
			} else if (m_atmosOxidizing > fixed(3,10)) {
				s += Lang::WITH_A+thickness+Lang::H_ATMOSPHERE;
			} else if (m_atmosOxidizing > fixed(2,10)) {
				s += Lang::WITH_A+thickness+Lang::HE_ATMOSPHERE;
			} else if (m_atmosOxidizing > fixed(15,100)) {
				s += Lang::WITH_A+thickness+Lang::AR_ATMOSPHERE;
			} else if (m_atmosOxidizing > fixed(1,10)) {
				s += Lang::WITH_A+thickness+Lang::S_ATMOSPHERE;
			} else {
				s += Lang::WITH_A+thickness+Lang::N_ATMOSPHERE;
			}
		}

		if (m_life > fixed(1,2)) {
			s += Lang::AND_HIGHLY_COMPLEX_ECOSYSTEM;
		} else if (m_life > fixed(1,10)) {
			s += Lang::AND_INDIGENOUS_PLANT_LIFE;
		} else if (m_life > fixed(0)) {
			s += Lang::AND_INDIGENOUS_MICROBIAL_LIFE;
		} else {
			s += ".";
		}

		return s;
	}
	case TYPE_STARPORT_ORBITAL:
		return Lang::ORBITAL_STARPORT;
	case TYPE_STARPORT_SURFACE:
		return Lang::STARPORT;
	case TYPE_GRAVPOINT:
    default:
        fprintf( stderr, "Warning: Invalid Astro Body Description found.\n");
        return Lang::UNKNOWN;
	}
}

const char *SystemBody::GetIcon() const
{
	switch (type) {
	case TYPE_BROWN_DWARF: return "icons/object_brown_dwarf.png";
	case TYPE_WHITE_DWARF: return "icons/object_white_dwarf.png";
	case TYPE_STAR_M: return "icons/object_star_m.png";
	case TYPE_STAR_K: return "icons/object_star_k.png";
	case TYPE_STAR_G: return "icons/object_star_g.png";
	case TYPE_STAR_F: return "icons/object_star_f.png";
	case TYPE_STAR_A: return "icons/object_star_a.png";
	case TYPE_STAR_B: return "icons/object_star_b.png";
	case TYPE_STAR_O: return "icons/object_star_b.png"; //shares B graphic for now
	case TYPE_STAR_M_GIANT: return "icons/object_star_m_giant.png";
	case TYPE_STAR_K_GIANT: return "icons/object_star_k_giant.png";
	case TYPE_STAR_G_GIANT: return "icons/object_star_g_giant.png";
	case TYPE_STAR_F_GIANT: return "icons/object_star_f_giant.png";
	case TYPE_STAR_A_GIANT: return "icons/object_star_a_giant.png";
	case TYPE_STAR_B_GIANT: return "icons/object_star_b_giant.png";
	case TYPE_STAR_O_GIANT: return "icons/object_star_o.png"; // uses old O type graphic
	case TYPE_STAR_M_SUPER_GIANT: return "icons/object_star_m_super_giant.png";
	case TYPE_STAR_K_SUPER_GIANT: return "icons/object_star_k_super_giant.png";
	case TYPE_STAR_G_SUPER_GIANT: return "icons/object_star_g_super_giant.png";
	case TYPE_STAR_F_SUPER_GIANT: return "icons/object_star_g_super_giant.png"; //shares G graphic for now
	case TYPE_STAR_A_SUPER_GIANT: return "icons/object_star_a_super_giant.png";
	case TYPE_STAR_B_SUPER_GIANT: return "icons/object_star_b_super_giant.png";
	case TYPE_STAR_O_SUPER_GIANT: return "icons/object_star_b_super_giant.png";// uses B type graphic for now
	case TYPE_STAR_M_HYPER_GIANT: return "icons/object_star_m_hyper_giant.png";
	case TYPE_STAR_K_HYPER_GIANT: return "icons/object_star_k_hyper_giant.png";
	case TYPE_STAR_G_HYPER_GIANT: return "icons/object_star_g_hyper_giant.png";
	case TYPE_STAR_F_HYPER_GIANT: return "icons/object_star_f_hyper_giant.png";
	case TYPE_STAR_A_HYPER_GIANT: return "icons/object_star_a_hyper_giant.png";
	case TYPE_STAR_B_HYPER_GIANT: return "icons/object_star_b_hyper_giant.png";
	case TYPE_STAR_O_HYPER_GIANT: return "icons/object_star_b_hyper_giant.png";// uses B type graphic for now
	case TYPE_STAR_M_WF: return "icons/object_star_m_wf.png";
	case TYPE_STAR_B_WF: return "icons/object_star_b_wf.png";
	case TYPE_STAR_O_WF: return "icons/object_star_o_wf.png";
	case TYPE_STAR_S_BH: return "icons/object_star_bh.png";
	case TYPE_STAR_IM_BH: return "icons/object_star_smbh.png";
	case TYPE_STAR_SM_BH: return "icons/object_star_smbh.png";
	case TYPE_PLANET_GAS_GIANT:
		if (mass > 800) {
			if (averageTemp > 1000) return "icons/object_planet_large_gas_giant_hot.png";
			else return "icons/object_planet_large_gas_giant.png";
		}
		if (mass > 300) {
			if (averageTemp > 1000) return "icons/object_planet_large_gas_giant_hot.png";
			else return "icons/object_planet_large_gas_giant.png";
		}
		if (mass > 80) {
			if (averageTemp > 1000) return "icons/object_planet_medium_gas_giant_hot.png";
			else return "icons/object_planet_medium_gas_giant.png";
		}
		else {
			if (averageTemp > 1000) return "icons/object_planet_small_gas_giant_hot.png";
			else return "icons/object_planet_small_gas_giant.png";
		}
	case TYPE_PLANET_ASTEROID:
		return "icons/object_planet_asteroid.png";
	case TYPE_PLANET_TERRESTRIAL:
		if (m_volatileLiquid > fixed(7,10)) {
			if (averageTemp > 250) return "icons/object_planet_water.png";
			else return "icons/object_planet_ice.png";
		}
		if ((m_life > fixed(9,10)) &&
			(m_volatileGas > fixed(6,10))) return "icons/object_planet_life.png";
		if ((m_life > fixed(8,10)) &&
			(m_volatileGas > fixed(5,10))) return "icons/object_planet_life6.png";
		if ((m_life > fixed(7,10)) &&
			(m_volatileGas > fixed(45,100))) return "icons/object_planet_life7.png";
		if ((m_life > fixed(6,10)) &&
			(m_volatileGas > fixed(4,10))) return "icons/object_planet_life8.png";
		if ((m_life > fixed(5,10)) &&
			(m_volatileGas > fixed(3,10))) return "icons/object_planet_life4.png";
		if ((m_life > fixed(4,10)) &&
			(m_volatileGas > fixed(2,10))) return "icons/object_planet_life5.png";
		if ((m_life > fixed(1,10)) &&
			(m_volatileGas > fixed(2,10))) return "icons/object_planet_life2.png";
		if (m_life > fixed(1,10)) return "icons/object_planet_life3.png";
		if (mass < fixed(1,100)) return "icons/object_planet_dwarf.png";
		if (mass < fixed(1,10)) return "icons/object_planet_small.png";
		if ((m_volatileLiquid < fixed(1,10)) &&
			(m_volatileGas > fixed(1,5))) return "icons/object_planet_desert.png";

		if (m_volatileIces + m_volatileLiquid > fixed(3,5)) {
			if (m_volatileIces > m_volatileLiquid) {
				if (averageTemp < 250)	return "icons/object_planet_ice.png";
			} else {
				if (averageTemp > 250) {
					return "icons/object_planet_water.png";
				} else return "icons/object_planet_ice.png";
			}
		}

		if (m_volatileGas > fixed(1,2)) {
			if (m_atmosOxidizing < fixed(1,2)) {
				if (averageTemp > 300) return "icons/object_planet_methane3.png";
				else if (averageTemp > 250) return "icons/object_planet_methane2.png";
				else return "icons/object_planet_methane.png";
			} else {
				if (averageTemp > 300) return "icons/object_planet_co2_2.png";
				else if (averageTemp > 250) {
					if ((m_volatileLiquid > fixed(3,10)) && (m_volatileGas > fixed(2,10)))
						return "icons/object_planet_co2_4.png";
					else return "icons/object_planet_co2_3.png";
				} else return "icons/object_planet_co2.png";
			}
		}

		if ((m_volatileLiquid > fixed(1,10)) &&
		   (m_volatileGas < fixed(1,10))) return "icons/object_planet_ice.png";
		if (m_volcanicity > fixed(7,10)) return "icons/object_planet_volcanic.png";
		return "icons/object_planet_small.png";
		/*
		"icons/object_planet_water_n1.png"
		"icons/object_planet_life3.png"
		"icons/object_planet_life2.png"
		*/
	case TYPE_STARPORT_ORBITAL:
		return "icons/object_orbital_starport.png";
	case TYPE_GRAVPOINT:
	case TYPE_STARPORT_SURFACE:
    default:
        fprintf( stderr, "Warning: Invalid body icon.\n");
		return 0;
	}
}

/*
 * Position a surface starport anywhere. Space.cpp::MakeFrameFor() ensures it
 * is on dry land (discarding this position if necessary)
 */
static void position_settlement_on_planet(SystemBody *b)
{
	MTRand r(b->seed);
	// used for orientation on planet surface
	double r2 = r.Double(); 	// function parameter evaluation order is implementation-dependent
	double r1 = r.Double();		// can't put two rands in the same expression
	b->orbit.rotMatrix = matrix4x4d::RotateZMatrix(2*M_PI*r1) *
			matrix4x4d::RotateYMatrix(2*M_PI*r2);
}

double SystemBody::GetMaxChildOrbitalDistance() const
{
	double max = 0;
	for (unsigned int i=0; i<children.size(); i++) {
		if (children[i]->orbMax.ToDouble() > max) {
			max = children[i]->orbMax.ToDouble();
		}
	}
	return AU * max;
}


/*
 * These are the nice floating point surface temp calculating turds.
 *
static const double boltzman_const = 5.6704e-8;
static double calcEnergyPerUnitAreaAtDist(double star_radius, double star_temp, double object_dist)
{
	const double total_solar_emission = boltzman_const *
		star_temp*star_temp*star_temp*star_temp*
		4*M_PI*star_radius*star_radius;

	return total_solar_emission / (4*M_PI*object_dist*object_dist);
}

// bond albedo, not geometric
static double CalcSurfaceTemp(double star_radius, double star_temp, double object_dist, double albedo, double greenhouse)
{
	const double energy_per_meter2 = calcEnergyPerUnitAreaAtDist(star_radius, star_temp, object_dist);
	const double surface_temp = pow(energy_per_meter2*(1-albedo)/(4*(1-greenhouse)*boltzman_const), 0.25);
	return surface_temp;
}
*/
/*
 * Instead we use these butt-ugly overflow-prone spat of ejaculate:
 */
/*
 * star_radius in sol radii
 * star_temp in kelvin,
 * object_dist in AU
 * return Watts/m^2
 */
static fixed calcEnergyPerUnitAreaAtDist(fixed star_radius, int star_temp, fixed object_dist)
{
	fixed temp = star_temp * fixed(1,10000);
	const fixed total_solar_emission =
		temp*temp*temp*temp*star_radius*star_radius;

	return fixed(1744665451,100000)*(total_solar_emission / (object_dist*object_dist));
}

static int CalcSurfaceTemp(const SystemBody *primary, fixed distToPrimary, fixed albedo, fixed greenhouse)
{
	fixed energy_per_meter2;
	if (primary->type == SystemBody::TYPE_GRAVPOINT) {
		// binary. take energies of both stars
		energy_per_meter2 = calcEnergyPerUnitAreaAtDist(primary->children[0]->radius,
			primary->children[0]->averageTemp, distToPrimary);
		energy_per_meter2 += calcEnergyPerUnitAreaAtDist(primary->children[1]->radius,
			primary->children[1]->averageTemp, distToPrimary);
	} else {
		energy_per_meter2 = calcEnergyPerUnitAreaAtDist(primary->radius, primary->averageTemp, distToPrimary);
	}
	const fixed surface_temp_pow4 = energy_per_meter2*(1-albedo)/(1-greenhouse);
	return int(isqrt(isqrt((surface_temp_pow4.v>>fixed::FRAC)*4409673)));
}


double SystemBody::CalcSurfaceGravity() const
{
	double r = GetRadius();
	if (r > 0.0) {
		return G * GetMass() / pow(r, 2);
	} else {
		return 0.0;
	}
}

vector3d Orbit::OrbitalPosAtTime(double t) const
{
	const double e = eccentricity;
	const double M_t0 = Orbit::orbitalPhaseAtStart; // mean anomaly at t = 0
	const double M = 2.0*M_PI*t / period + M_t0; // mean anomaly
	// eccentric anomaly
	// NR method to solve for E: M = E-sin(E)
	double E = M;
	for (int iter=5; iter > 0; --iter) {
		E = E - (E-e*(sin(E))-M) / (1.0 - e*cos(E));
	}
	// heliocentric distance
	double r = semiMajorAxis * (1.0 - e*cos(E));
	// true anomaly (angle of orbit position)
	double cos_v = (cos(E) - e) / (1.0 - e*cos(E));
	double sin_v = (sqrt(1.0-e*e)*sin(E))/ (1.0 - e*cos(E));

	vector3d pos = vector3d(-cos_v*r, sin_v*r, 0);
	pos = rotMatrix * pos;
	return pos;
}

// used for stepping through the orbit in small fractions
// therefore the orbital phase at game start (mean anomalty at t = 0)
// does not need to be taken into account
vector3d Orbit::EvenSpacedPosAtTime(double t) const
{
	const double e = eccentricity;
	const double M = 2*M_PI*t;
	const double v = 2*atan(sqrt((1+e)/(1-e)) * tan(M/2.0));
	const double r = semiMajorAxis * (1 - e*e) / (1 + e*cos(v));
	vector3d pos = vector3d(-cos(v)*r, sin(v)*r, 0);
	pos = rotMatrix * pos;
	return pos;
}

double calc_orbital_period(double semiMajorAxis, double centralMass)
{
	return 2.0*M_PI*sqrt((semiMajorAxis*semiMajorAxis*semiMajorAxis)/(G*centralMass));
}

SystemBody *StarSystem::GetBodyByPath(const SystemPath &path) const
{
	assert(m_path.IsSameSystem(path));
	assert(path.IsBodyPath());
	assert(path.bodyIndex < m_bodies.size());

	return m_bodies[path.bodyIndex];
}

SystemPath StarSystem::GetPathOf(const SystemBody *sbody) const
{
	return sbody->path;
}

void StarSystem::CustomGetKidsOf(SystemBody *parent, const std::vector<CustomSystemBody*> &children, int *outHumanInfestedness, MTRand &rand)
{
	for (std::vector<CustomSystemBody*>::const_iterator i = children.begin(); i != children.end(); ++i) {
		const CustomSystemBody *csbody = *i;

		SystemBody *kid = NewBody();
		kid->type = csbody->type;
		kid->parent = parent;
		kid->seed = csbody->want_rand_seed ? rand.Int32() : csbody->seed;
		kid->radius = csbody->radius;
		kid->averageTemp = csbody->averageTemp;
		kid->name = csbody->name;
		kid->isCustomBody = true;

		kid->mass = csbody->mass;
		if (kid->type == SystemBody::TYPE_PLANET_ASTEROID) kid->mass /= 100000;

		kid->m_metallicity    = csbody->metallicity;
		//multiple of Earth's surface density
		kid->m_volatileGas    = csbody->volatileGas*fixed(1225,1000);
		kid->m_volatileLiquid = csbody->volatileLiquid;
		kid->m_volatileIces   = csbody->volatileIces;
		kid->m_volcanicity    = csbody->volcanicity;
		kid->m_atmosOxidizing = csbody->atmosOxidizing;
		kid->m_life           = csbody->life;

		kid->rotationPeriod = csbody->rotationPeriod;
		kid->rotationalPhaseAtStart = csbody->rotationalPhaseAtStart;
		kid->eccentricity = csbody->eccentricity;
		kid->orbitalOffset = csbody->orbitalOffset;
		kid->orbitalPhaseAtStart = csbody->orbitalPhaseAtStart;
		kid->axialTilt = csbody->axialTilt;
		kid->semiMajorAxis = csbody->semiMajorAxis;
		kid->orbit.eccentricity = csbody->eccentricity.ToDouble();
		kid->orbit.semiMajorAxis = csbody->semiMajorAxis.ToDouble() * AU;
		kid->orbit.period = calc_orbital_period(kid->orbit.semiMajorAxis, parent->GetMass());
		kid->orbit.orbitalPhaseAtStart = csbody->orbitalPhaseAtStart.ToDouble();
		if (csbody->heightMapFilename.length() > 0) {
			kid->heightMapFilename = csbody->heightMapFilename.c_str();
			kid->heightMapFractal = csbody->heightMapFractal;
		}
		if (kid->type == SystemBody::TYPE_STARPORT_SURFACE) {
			kid->orbit.rotMatrix = matrix4x4d::RotateYMatrix(csbody->longitude) *
				matrix4x4d::RotateXMatrix(-0.5*M_PI + csbody->latitude);
		} else {
			if (kid->orbit.semiMajorAxis < 1.2 * parent->GetRadius()) {
				Error("%s's orbit is too close to its parent", csbody->name.c_str());
			}
			double offset = csbody->want_rand_offset ? rand.Double(2*M_PI) : (csbody->orbitalOffset.ToDouble()*M_PI);
			kid->orbit.rotMatrix = matrix4x4d::RotateYMatrix(offset) * matrix4x4d::RotateXMatrix(-0.5*M_PI + csbody->latitude);
		}
		if (kid->GetSuperType() == SystemBody::SUPERTYPE_STARPORT) {
			(*outHumanInfestedness)++;
            m_spaceStations.push_back(kid);
		}
		parent->children.push_back(kid);

		// perihelion and aphelion (in AUs)
		kid->orbMin = csbody->semiMajorAxis - csbody->eccentricity*csbody->semiMajorAxis;
		kid->orbMax = 2*csbody->semiMajorAxis - kid->orbMin;

		kid->PickAtmosphere();

		// pick or specify rings
		switch (csbody->ringStatus) {
			case CustomSystemBody::WANT_NO_RINGS:
				kid->m_rings.minRadius = fixed(0);
				kid->m_rings.maxRadius = fixed(0);
				break;
			case CustomSystemBody::WANT_RINGS:
				kid->PickRings(true);
				break;
			case CustomSystemBody::WANT_RANDOM_RINGS:
				kid->PickRings(false);
				break;
			case CustomSystemBody::WANT_CUSTOM_RINGS:
				kid->m_rings.minRadius = csbody->ringInnerRadius;
				kid->m_rings.maxRadius = csbody->ringOuterRadius;
				kid->m_rings.baseColor = csbody->ringColor;
				break;
		}

		CustomGetKidsOf(kid, csbody->children, outHumanInfestedness, rand);
	}

}

void StarSystem::GenerateFromCustom(const CustomSystem *customSys, MTRand &rand)
{
	const CustomSystemBody *csbody = customSys->sBody;

	rootBody = NewBody();
	rootBody->type = csbody->type;
	rootBody->parent = NULL;
	rootBody->seed = csbody->want_rand_seed ? rand.Int32() : csbody->seed;
	rootBody->seed = rand.Int32();
	rootBody->radius = csbody->radius;
	rootBody->mass = csbody->mass;
	rootBody->averageTemp = csbody->averageTemp;
	rootBody->name = csbody->name;
	rootBody->isCustomBody = true;

	rootBody->rotationalPhaseAtStart = csbody->rotationalPhaseAtStart;
	rootBody->orbitalPhaseAtStart = csbody->orbitalPhaseAtStart;

	int humanInfestedness = 0;
	CustomGetKidsOf(rootBody, csbody->children, &humanInfestedness, rand);
	Populate(false);

}

void StarSystem::MakeStarOfType(SystemBody *sbody, SystemBody::BodyType type, MTRand &rand)
{
	sbody->type = type;
	sbody->seed = rand.Int32();
	sbody->radius = fixed(rand.Int32(starTypeInfo[type].radius[0],
				starTypeInfo[type].radius[1]), 100);
	sbody->mass = fixed(rand.Int32(starTypeInfo[type].mass[0],
				starTypeInfo[type].mass[1]), 100);
	sbody->averageTemp = rand.Int32(starTypeInfo[type].tempMin,
				starTypeInfo[type].tempMax);
}

void StarSystem::MakeRandomStar(SystemBody *sbody, MTRand &rand)
{
	SystemBody::BodyType type = SystemBody::BodyType(rand.Int32(SystemBody::TYPE_STAR_MIN, SystemBody::TYPE_STAR_MAX));
	MakeStarOfType(sbody, type, rand);
}

void StarSystem::MakeStarOfTypeLighterThan(SystemBody *sbody, SystemBody::BodyType type, fixed maxMass, MTRand &rand)
{
	int tries = 16;
	do {
		MakeStarOfType(sbody, type, rand);
	} while ((sbody->mass > maxMass) && (--tries));
}

void StarSystem::MakeBinaryPair(SystemBody *a, SystemBody *b, fixed minDist, MTRand &rand)
{
	fixed m = a->mass + b->mass;
	fixed a0 = b->mass / m;
	fixed a1 = a->mass / m;
	a->eccentricity = rand.NFixed(3);
	int mul = 1;

	do {
		switch (rand.Int32(3)) {
			case 2: a->semiMajorAxis = fixed(rand.Int32(100,10000), 100); break;
			case 1: a->semiMajorAxis = fixed(rand.Int32(10,1000), 100); break;
			default:
			case 0: a->semiMajorAxis = fixed(rand.Int32(1,100), 100); break;
		}
		a->semiMajorAxis *= mul;
		mul *= 2;
	} while (a->semiMajorAxis < minDist);

	a->orbit.eccentricity = a->eccentricity.ToDouble();
	a->orbit.semiMajorAxis = AU * (a->semiMajorAxis * a0).ToDouble();
	a->orbit.period = 60*60*24*365* a->semiMajorAxis.ToDouble() * sqrt(a->semiMajorAxis.ToDouble() / m.ToDouble());

	const float rotX = -0.5f*float(M_PI);//(float)(rand.Double()*M_PI/2.0);
	const float rotY = static_cast<float>(rand.Double(M_PI));
	a->orbit.rotMatrix = matrix4x4d::RotateYMatrix(rotY) * matrix4x4d::RotateXMatrix(rotX);
	b->orbit.rotMatrix = matrix4x4d::RotateYMatrix(rotY-M_PI) * matrix4x4d::RotateXMatrix(rotX);

	b->orbit.eccentricity = a->eccentricity.ToDouble();
	b->orbit.semiMajorAxis = AU * (a->semiMajorAxis * a1).ToDouble();
	b->orbit.period = a->orbit.period;

	fixed orbMin = a->semiMajorAxis - a->eccentricity*a->semiMajorAxis;
	fixed orbMax = 2*a->semiMajorAxis - orbMin;
	a->orbMin = orbMin;
	b->orbMin = orbMin;
	a->orbMax = orbMax;
	b->orbMax = orbMax;
}

SystemBody::SystemBody()
{
	heightMapFilename = 0;
	heightMapFractal = 0;
	rotationalPhaseAtStart = fixed(0);
	orbitalPhaseAtStart = fixed(0);
	isCustomBody = false;
}

bool SystemBody::HasAtmosphere() const
{
	return (m_volatileGas > fixed(1,100));
}

bool SystemBody::IsScoopable() const
{
	return (GetSuperType() == SUPERTYPE_GAS_GIANT);
}

void SystemBody::PickAtmosphere()
{
	/* Alpha value isn't real alpha. in the shader fog depth is determined
	 * by density*alpha, so that we can have very dense atmospheres
	 * without having them a big stinking solid color obscuring everything

	  These are our atmosphere colours, for terrestrial planets we use m_atmosOxidizing
	  for some variation to atmosphere colours
	 */
	switch (type) {
		case SystemBody::TYPE_PLANET_GAS_GIANT:
			m_atmosColor = Color(1.0f, 1.0f, 1.0f, 0.0005f);
			m_atmosDensity = 14.0;
			break;
		case SystemBody::TYPE_PLANET_ASTEROID:
			m_atmosColor = Color(0.0f, 0.0f, 0.0f, 0.0f);
			m_atmosDensity = 0.0;
			break;
		default:
		case SystemBody::TYPE_PLANET_TERRESTRIAL:
			double r = 0, g = 0, b = 0;
			double atmo = m_atmosOxidizing.ToDouble();
			if (m_volatileGas.ToDouble() > 0.001) {
				if (atmo > 0.95) {
					// o2
					r = 1.0f + ((0.95f-atmo)*15.0f);
					g = 0.95f + ((0.95f-atmo)*10.0f);
					b = atmo*atmo*atmo*atmo*atmo;
				} else if (atmo > 0.7) {
					// co2
					r = atmo+0.05f;
					g = 1.0f + (0.7f-atmo);
					b = 0.8f;
				} else if (atmo > 0.65) {
					// co
					r = 1.0f + (0.65f-atmo);
					g = 0.8f;
					b = atmo + 0.25f;
				} else if (atmo > 0.55) {
					// ch4
					r = 1.0f + ((0.55f-atmo)*5.0);
					g = 0.35f - ((0.55f-atmo)*5.0);
					b = 0.4f;
				} else if (atmo > 0.3) {
					// h
					r = 1.0f;
					g = 1.0f;
					b = 1.0f;
				} else if (atmo > 0.2) {
					// he
					r = 1.0f;
					g = 1.0f;
					b = 1.0f;
				} else if (atmo > 0.15) {
					// ar
					r = 0.5f - ((0.15f-atmo)*5.0);
					g = 0.0f;
					b = 0.5f + ((0.15f-atmo)*5.0);
				} else if (atmo > 0.1) {
					// s
					r = 0.8f - ((0.1f-atmo)*4.0);
					g = 1.0f;
					b = 0.5f - ((0.1f-atmo)*10.0);
				} else {
					// n
					r = 1.0f;
					g = 1.0f;
					b = 1.0f;
				}
				m_atmosColor = Color(r, g, b, 1.0f);
			} else {
				m_atmosColor = Color(0.0, 0.0, 0.0, 0.0f);
			}
			m_atmosDensity = m_volatileGas.ToDouble();
			//printf("| Atmosphere :\n|      red   : [%f] \n|      green : [%f] \n|      blue  : [%f] \n", r, g, b);
			//printf("-------------------------------\n");
			break;
		/*default:
			m_atmosColor = Color(0.6f, 0.6f, 0.6f, 1.0f);
			m_atmosDensity = m_body->m_volatileGas.ToDouble();
			break;*/
	}
}

static const unsigned char RANDOM_RING_COLORS[][4] = {
	{ 156, 122,  98, 217 }, // jupiter-like
	{ 156, 122,  98, 217 }, // saturn-like
	{ 181, 173, 174, 217 }, // neptune-like
	{ 130, 122,  98, 217 }, // uranus-like
	{ 207, 122,  98, 217 }  // brown dwarf-like
};

void SystemBody::PickRings(bool forceRings)
{
	m_rings.minRadius = fixed(0);
	m_rings.maxRadius = fixed(0);
	m_rings.baseColor = Color4ub(255,255,255,255);

	if (type == SystemBody::TYPE_PLANET_GAS_GIANT) {
		MTRand ringRng(seed + 965467);

		// today's forecast: 50% chance of rings
		double rings_die = ringRng.Double();
		if (forceRings || (rings_die < 0.5)) {
			const unsigned char * const baseCol
				= RANDOM_RING_COLORS[ringRng.Int32(COUNTOF(RANDOM_RING_COLORS))];
			m_rings.baseColor.r = Clamp(baseCol[0] + ringRng.Int32(-20,20), 0, 255);
			m_rings.baseColor.g = Clamp(baseCol[1] + ringRng.Int32(-20,20), 0, 255);
			m_rings.baseColor.b = Clamp(baseCol[2] + ringRng.Int32(-20,10), 0, 255);
			m_rings.baseColor.a = Clamp(baseCol[3] + ringRng.Int32(-5,5), 0, 255);

			// from wikipedia: http://en.wikipedia.org/wiki/Roche_limit
			// basic Roche limit calculation assuming a rigid satellite
			// d = R (2 p_M / p_m)^{1/3}
			//
			// where R is the radius of the primary, p_M is the density of
			// the primary and p_m is the density of the satellite
			//
			// I assume a satellite density of 500 kg/m^3
			// (which Wikipedia says is an average comet density)
			//
			// also, I can't be bothered to think about unit conversions right now,
			// so I'm going to ignore the real density of the primary and take it as 1100 kg/m^3
			// (note: density of Saturn is ~687, Jupiter ~1,326, Neptune ~1,638, Uranus ~1,318)
			//
			// This gives: d = 1.638642 * R
			fixed innerMin = fixed(110, 100);
			fixed innerMax = fixed(145, 100);
			fixed outerMin = fixed(150, 100);
			fixed outerMax = fixed(168642, 100000);

			m_rings.minRadius = innerMin + (innerMax - innerMin)*ringRng.Fixed();
			m_rings.maxRadius = outerMin + (outerMax - outerMin)*ringRng.Fixed();
		}
	}
}

// Calculate parameters used in the atmospheric model for shaders
// used by both LmrModels and Geosphere
SystemBody::AtmosphereParameters SystemBody::CalcAtmosphereParams() const
{
	AtmosphereParameters params;

	double atmosDensity;

	GetAtmosphereFlavor(&params.atmosCol, &atmosDensity);
	// adjust global atmosphere opacity
	atmosDensity *= 1e-5;

	params.atmosDensity = static_cast<float>(atmosDensity);

	// Calculate parameters used in the atmospheric model for shaders
	// Isothermal atmospheric model
	// See http://en.wikipedia.org/wiki/Atmospheric_pressure#Altitude_atmospheric_pressure_variation
	// This model features an exponential decrease in pressure and density with altitude.
	// The scale height is 1/the exponential coefficient.

	// The equation for pressure is:
	// Pressure at height h = Pressure surface * e^((-Mg/RT)*h)

	// calculate (inverse) atmosphere scale height
	// The formula for scale height is:
	// h = RT / Mg
	// h is height above the surface in meters
	// R is the universal gas constant
	// T is the surface temperature in Kelvin
	// g is the gravity in m/s^2
	// M is the molar mass of air in kg/mol

	// calculate gravity
	// radius of the planet
	const double radiusPlanet_in_m = (radius.ToDouble()*EARTH_RADIUS);
	const double massPlanet_in_kg = (mass.ToDouble()*EARTH_MASS);
	const double g = G*massPlanet_in_kg/(radiusPlanet_in_m*radiusPlanet_in_m);

	const double T = static_cast<double>(averageTemp);

	// XXX just use earth's composition for now
	const double M = 0.02897f; // in kg/mol

	const float atmosScaleHeight = static_cast<float>(GAS_CONSTANT_R*T/(M*g));

	// min of 2.0 corresponds to a scale height of 1/20 of the planet's radius,
	params.atmosInvScaleHeight = std::max(20.0f, static_cast<float>(GetRadius() / atmosScaleHeight));
	// integrate atmospheric density between surface and this radius. this is 10x the scale
	// height, which should be a height at which the atmospheric density is negligible
	params.atmosRadius = 1.0f + static_cast<float>(10.0f * atmosScaleHeight) / GetRadius();

	params.planetRadius = static_cast<float>(radiusPlanet_in_m);

	return params;
}


/*
 * As my excellent comrades have pointed out, choices that depend on floating
 * point crap will result in different universes on different platforms.
 *
 * We must be sneaky and avoid floating point in these places.
 */
StarSystem::StarSystem(const SystemPath &path) : m_path(path), m_factionIdx(Faction::BAD_FACTION_IDX)
{
	assert(path.IsSystemPath());
	memset(m_tradeLevel, 0, sizeof(m_tradeLevel));
	rootBody = 0;

	Sector s = Sector(m_path.sectorX, m_path.sectorY, m_path.sectorZ);
	assert(m_path.systemIndex >= 0 && m_path.systemIndex < s.m_systems.size());

	m_seed = s.m_systems[m_path.systemIndex].seed;
	m_name = s.m_systems[m_path.systemIndex].name;

	unsigned long _init[6] = { m_path.systemIndex, Uint32(m_path.sectorX), Uint32(m_path.sectorY), Uint32(m_path.sectorZ), UNIVERSE_SEED, Uint32(m_seed) };
	MTRand rand(_init, 6);

	/*
	 * 0 - ~500ly from sol: explored
	 * ~500ly - ~700ly (65-90 sectors): gradual
	 * ~700ly+: unexplored
	 */
	int dist = isqrt(1 + m_path.sectorX*m_path.sectorX + m_path.sectorY*m_path.sectorY + m_path.sectorZ*m_path.sectorZ);
	m_unexplored = (dist > 90) || (dist > 65 && rand.Int32(dist) > 40);

	m_isCustom = m_hasCustomBodies = false;
	if (s.m_systems[m_path.systemIndex].customSys) {
		m_isCustom = true;
		const CustomSystem *custom = s.m_systems[m_path.systemIndex].customSys;
		m_numStars = custom->numStars;
		if (custom->shortDesc.length() > 0) m_shortDesc = custom->shortDesc;
		if (custom->longDesc.length() > 0) m_longDesc = custom->longDesc;
		if (!custom->want_rand_explored) m_unexplored = !custom->explored;
		if (!custom->IsRandom()) {
			m_hasCustomBodies = true;
			GenerateFromCustom(s.m_systems[m_path.systemIndex].customSys, rand);
			return;
		}
	}

	SystemBody *star[4];
	SystemBody *centGrav1(NULL), *centGrav2(NULL);

	const int numStars = s.m_systems[m_path.systemIndex].numStars;
	assert((numStars >= 1) && (numStars <= 4));

	if (numStars == 1) {
		SystemBody::BodyType type = s.m_systems[m_path.systemIndex].starType[0];
		star[0] = NewBody();
		star[0]->parent = NULL;
		star[0]->name = s.m_systems[m_path.systemIndex].name;
		star[0]->orbMin = 0;
		star[0]->orbMax = 0;
		MakeStarOfType(star[0], type, rand);
		rootBody = star[0];
		m_numStars = 1;
	} else {
		centGrav1 = NewBody();
		centGrav1->type = SystemBody::TYPE_GRAVPOINT;
		centGrav1->parent = NULL;
		centGrav1->name = s.m_systems[m_path.systemIndex].name+" A,B";
		rootBody = centGrav1;

		SystemBody::BodyType type = s.m_systems[m_path.systemIndex].starType[0];
		star[0] = NewBody();
		star[0]->name = s.m_systems[m_path.systemIndex].name+" A";
		star[0]->parent = centGrav1;
		MakeStarOfType(star[0], type, rand);

		star[1] = NewBody();
		star[1]->name = s.m_systems[m_path.systemIndex].name+" B";
		star[1]->parent = centGrav1;
		MakeStarOfTypeLighterThan(star[1], s.m_systems[m_path.systemIndex].starType[1],
				star[0]->mass, rand);

		centGrav1->mass = star[0]->mass + star[1]->mass;
		centGrav1->children.push_back(star[0]);
		centGrav1->children.push_back(star[1]);
		const fixed minDist1 = (star[0]->radius + star[1]->radius) * AU_SOL_RADIUS;
try_that_again_guvnah:
		MakeBinaryPair(star[0], star[1], minDist1, rand);

		m_numStars = 2;

		if (numStars > 2) {
			if (star[0]->orbMax > fixed(100,1)) {
				// reduce to < 100 AU...
				goto try_that_again_guvnah;
			}
			// 3rd and maybe 4th star
			if (numStars == 3) {
				star[2] = NewBody();
				star[2]->name = s.m_systems[m_path.systemIndex].name+" C";
				star[2]->orbMin = 0;
				star[2]->orbMax = 0;
				MakeStarOfTypeLighterThan(star[2], s.m_systems[m_path.systemIndex].starType[2],
					star[0]->mass, rand);
				centGrav2 = star[2];
				m_numStars = 3;
			} else {
				centGrav2 = NewBody();
				centGrav2->type = SystemBody::TYPE_GRAVPOINT;
				centGrav2->name = s.m_systems[m_path.systemIndex].name+" C,D";
				centGrav2->orbMax = 0;

				star[2] = NewBody();
				star[2]->name = s.m_systems[m_path.systemIndex].name+" C";
				star[2]->parent = centGrav2;
				MakeStarOfTypeLighterThan(star[2], s.m_systems[m_path.systemIndex].starType[2],
					star[0]->mass, rand);

				star[3] = NewBody();
				star[3]->name = s.m_systems[m_path.systemIndex].name+" D";
				star[3]->parent = centGrav2;
				MakeStarOfTypeLighterThan(star[3], s.m_systems[m_path.systemIndex].starType[3],
					star[2]->mass, rand);

				const fixed minDist2 = (star[2]->radius + star[3]->radius) * AU_SOL_RADIUS;
				MakeBinaryPair(star[2], star[3], minDist2, rand);
				centGrav2->mass = star[2]->mass + star[3]->mass;
				centGrav2->children.push_back(star[2]);
				centGrav2->children.push_back(star[3]);
				m_numStars = 4;
			}
			SystemBody *superCentGrav = NewBody();
			superCentGrav->type = SystemBody::TYPE_GRAVPOINT;
			superCentGrav->parent = NULL;
			superCentGrav->name = s.m_systems[m_path.systemIndex].name;
			centGrav1->parent = superCentGrav;
			centGrav2->parent = superCentGrav;
			rootBody = superCentGrav;
			const fixed minDistSuper = star[0]->orbMax + star[2]->orbMax;
			MakeBinaryPair(centGrav1, centGrav2, 4*minDistSuper, rand);
			superCentGrav->children.push_back(centGrav1);
			superCentGrav->children.push_back(centGrav2);

		}
	}

	// used in MakeShortDescription
	// XXX except this does not reflect the actual mining happening in this system
	m_metallicity = starMetallicities[rootBody->type];

	for (int i=0; i<m_numStars; i++) MakePlanetsAround(star[i], rand);

	if (m_numStars > 1) MakePlanetsAround(centGrav1, rand);
	if (m_numStars == 4) MakePlanetsAround(centGrav2, rand);

	Populate(true);

#ifdef DEBUG_DUMP
	Dump();
#endif /* DEBUG_DUMP */
}

#ifdef DEBUG_DUMP
struct thing_t {
	SystemBody* obj;
	vector3d pos;
	vector3d vel;
};
void StarSystem::Dump()
{
	std::vector<SystemBody*> obj_stack;
	std::vector<vector3d> pos_stack;
	std::vector<thing_t> output;

	SystemBody *obj = rootBody;
	vector3d pos = vector3d(0.0);

	while (obj) {
		vector3d p2 = pos;
		if (obj->parent) {
			p2 = pos + obj->orbit.OrbitalPosAtTime(1.0);
			pos = pos + obj->orbit.OrbitalPosAtTime(0.0);
		}

		if ((obj->type != SystemBody::TYPE_GRAVPOINT) &&
		    (obj->GetSuperType() != SystemBody::SUPERTYPE_STARPORT)) {
			struct thing_t t;
			t.obj = obj;
			t.pos = pos;
			t.vel = (p2-pos);
			output.push_back(t);
		}
		for (std::vector<SystemBody*>::iterator i = obj->children.begin();
				i != obj->children.end(); ++i) {
			obj_stack.push_back(*i);
			pos_stack.push_back(pos);
		}
		if (obj_stack.size() == 0) break;
		pos = pos_stack.back();
		obj = obj_stack.back();
		pos_stack.pop_back();
		obj_stack.pop_back();
	}

	FILE *f = fopen("starsystem.dump", "w");
	fprintf(f, "%d bodies\n", output.size());
	fprintf(f, "0 steps\n");
	for (std::vector<thing_t>::iterator i = output.begin();
			i != output.end(); ++i) {
		fprintf(f, "B:%lf,%lf:%lf,%lf,%lf,%lf:%lf:%d:%lf,%lf,%lf\n",
				(*i).pos.x, (*i).pos.y, (*i).pos.z,
				(*i).vel.x, (*i).vel.y, (*i).vel.z,
				(*i).obj->GetMass(), 0,
				1.0, 1.0, 1.0);
	}
	fclose(f);
	printf("Junk dumped to starsystem.dump\n");
}
#endif /* DEBUG_DUMP */

/*
 * http://en.wikipedia.org/wiki/Hill_sphere
 */
fixed SystemBody::CalcHillRadius() const
{
	if (GetSuperType() <= SUPERTYPE_STAR) {
		return fixed(0);
	} else {
		// playing with precision since these numbers get small
		// masses in earth masses
		fixedf<32> mprimary = parent->GetMassInEarths();

		fixedf<48> a = semiMajorAxis;
		fixedf<48> e = eccentricity;

		return fixed(a * (fixedf<48>(1,1)-e) *
				fixedf<48>::CubeRootOf(fixedf<48>(
						mass / (fixedf<32>(3,1)*mprimary))));

		//fixed hr = semiMajorAxis*(fixed(1,1) - eccentricity) *
		//  fixedcuberoot(mass / (3*mprimary));
	}
}

static fixed mass_from_disk_area(fixed a, fixed b, fixed max)
{
	// so, density of the disk with distance from star goes like so: 1 - x/discMax
	//
	// ---
	//    ---
	//       --- <- zero at discMax
	//
	// Which turned into a disc becomes 2*pi*x - (2*pi*x*x)/discMax
	// Integral of which is: pi*x*x - (2/(3*discMax))*pi*x*x*x
	//
	// Because get_disc_density divides total_mass by
	// mass_from_disk_area(0, discMax, discMax) to find density, the
	// constant factors (pi) in this equation drop out.
	//
	b = (b > max ? max : b);
	assert(b>=a);
	assert(a<=max);
	assert(b<=max);
	assert(a>=0);
	fixed one_over_3max = fixed(2,1)/(3*max);
	return (b*b - one_over_3max*b*b*b) -
		(a*a - one_over_3max*a*a*a);
}

static fixed get_disc_density(SystemBody *primary, fixed discMin, fixed discMax, fixed percentOfPrimaryMass)
{
	discMax = std::max(discMax, discMin);
	fixed total = mass_from_disk_area(discMin, discMax, discMax);
	return primary->GetMassInEarths() * percentOfPrimaryMass / total;
}

void StarSystem::MakePlanetsAround(SystemBody *primary, MTRand &rand)
{
	fixed discMin = fixed(0);
	fixed discMax = fixed(5000,1);
	fixed discDensity;

	SystemBody::BodySuperType superType = primary->GetSuperType();

	if (superType <= SystemBody::SUPERTYPE_STAR) {
		if (primary->type == SystemBody::TYPE_GRAVPOINT) {
			/* around a binary */
			discMin = primary->children[0]->orbMax * SAFE_DIST_FROM_BINARY;
		} else {
			/* correct thing is roche limit, but lets ignore that because
			 * it depends on body densities and gives some strange results */
			discMin = 4 * primary->radius * AU_SOL_RADIUS;
		}
		if (primary->type == SystemBody::TYPE_WHITE_DWARF) {
			// white dwarfs will have started as stars < 8 solar
			// masses or so, so pick discMax according to that
			// We give it a larger discMin because it used to be a much larger star
			discMin = 1000 * primary->radius * AU_SOL_RADIUS;
			discMax = 100 * rand.NFixed(2);		// rand-splitting again
			discMax *= fixed::SqrtOf(fixed(1,2) + fixed(8,1)*rand.Fixed());
		} else {
			discMax = 100 * rand.NFixed(2)*fixed::SqrtOf(primary->mass);
		}
		// having limited discMin by bin-separation/fake roche, and
		// discMax by some relation to star mass, we can now compute
		// disc density
		discDensity = rand.Fixed() * get_disc_density(primary, discMin, discMax, fixed(2,100));

		if ((superType == SystemBody::SUPERTYPE_STAR) && (primary->parent)) {
			// limit planets out to 10% distance to star's binary companion
			discMax = std::min(discMax, primary->orbMin * fixed(1,10));
		}

		/* in trinary and quaternary systems don't bump into other pair... */
		if (m_numStars >= 3) {
			discMax = std::min(discMax, fixed(5,100)*rootBody->children[0]->orbMin);
		}
	} else {
		fixed primary_rad = primary->radius * AU_EARTH_RADIUS;
		discMin = 4 * primary_rad;
		/* use hill radius to find max size of moon system. for stars botch it.
		   And use planets orbit around its primary as a scaler to a moon's orbit*/
		discMax = std::min(discMax, fixed(1,20)*
			primary->CalcHillRadius()*primary->orbMin*fixed(1,10));

		discDensity = rand.Fixed() * get_disc_density(primary, discMin, discMax, fixed(1,500));
	}

	//fixed discDensity = 20*rand.NFixed(4);

	//printf("Around %s: Range %f -> %f AU\n", primary->name.c_str(), discMin.ToDouble(), discMax.ToDouble());

	fixed initialJump = rand.NFixed(5);
	fixed pos = (fixed(1,1) - initialJump)*discMin + (initialJump*discMax);

	while (pos < discMax) {
		// periapsis, apoapsis = closest, farthest distance in orbit
		fixed periapsis = pos + pos*fixed(1,2)*rand.NFixed(2);/* + jump */;
		fixed ecc = rand.NFixed(3);
		fixed semiMajorAxis = periapsis / (fixed(1,1) - ecc);
		fixed apoapsis = 2*semiMajorAxis - periapsis;
		if (apoapsis > discMax) break;

		fixed mass;
		{
			const fixed a = pos;
			const fixed b = fixed(135,100)*apoapsis;
			mass = mass_from_disk_area(a, b, discMax);
			mass *= rand.Fixed() * discDensity;
		}
		if (mass < 0) {// hack around overflow
			fprintf(stderr, "WARNING: planetary mass has overflowed! (child of %s)\n", primary->name.c_str());
			mass = fixed(Sint64(0x7fFFffFFffFFffFFull));
		}
		assert(mass >= 0);

		SystemBody *planet = NewBody();
		planet->eccentricity = ecc;
		planet->axialTilt = fixed(100,157)*rand.NFixed(2);
		planet->semiMajorAxis = semiMajorAxis;
		planet->type = SystemBody::TYPE_PLANET_TERRESTRIAL;
		planet->seed = rand.Int32();
		planet->tmp = 0;
		planet->parent = primary;
		planet->mass = mass;
		planet->rotationPeriod = fixed(rand.Int32(1,200), 24);

		planet->orbit.eccentricity = ecc.ToDouble();
		planet->orbit.semiMajorAxis = semiMajorAxis.ToDouble() * AU;
		planet->orbit.period = calc_orbital_period(planet->orbit.semiMajorAxis, primary->GetMass());

		double r1 = rand.Double(2*M_PI);		// function parameter evaluation order is implementation-dependent
		double r2 = rand.NDouble(5);			// can't put two rands in the same expression
		planet->orbit.rotMatrix = matrix4x4d::RotateYMatrix(r1) *
			matrix4x4d::RotateXMatrix(-0.5*M_PI + r2*M_PI/2.0);

		planet->orbMin = periapsis;
		planet->orbMax = apoapsis;
		primary->children.push_back(planet);

		/* minimum separation between planets of 1.35 */
		pos = apoapsis * fixed(135,100);
	}

	int idx=0;
	bool make_moons = superType <= SystemBody::SUPERTYPE_STAR;

	for (std::vector<SystemBody*>::iterator i = primary->children.begin(); i != primary->children.end(); ++i) {
		// planets around a binary pair [gravpoint] -- ignore the stars...
		if ((*i)->GetSuperType() == SystemBody::SUPERTYPE_STAR) continue;
		// Turn them into something!!!!!!!
		char buf[8];
		if (superType <= SystemBody::SUPERTYPE_STAR) {
			// planet naming scheme
			snprintf(buf, sizeof(buf), " %c", 'a'+idx);
		} else {
			// moon naming scheme
			snprintf(buf, sizeof(buf), " %d", 1+idx);
		}
		(*i)->name = primary->name+buf;
		(*i)->PickPlanetType(rand);
		if (make_moons) MakePlanetsAround(*i, rand);
		idx++;
	}
}

/*
 * For moons distance from star is not orbMin, orbMax.
 */
const SystemBody *SystemBody::FindStarAndTrueOrbitalRange(fixed &orbMin_, fixed &orbMax_)
{
	const SystemBody *planet = this;
	const SystemBody *star = this->parent;

	assert(star);

	/* while not found star yet.. */
	while (star->GetSuperType() > SystemBody::SUPERTYPE_STAR) {
		planet = star;
		star = star->parent;
	}

	orbMin_ = planet->orbMin;
	orbMax_ = planet->orbMax;
	return star;
}

void SystemBody::PickPlanetType(MTRand &rand)
{
	fixed albedo = fixed(0);
	fixed greenhouse = fixed(0);

	fixed minDistToStar, maxDistToStar, averageDistToStar;
	const SystemBody *star = FindStarAndTrueOrbitalRange(minDistToStar, maxDistToStar);
	averageDistToStar = (minDistToStar+maxDistToStar)>>1;

	/* first calculate blackbody temp (no greenhouse effect, zero albedo) */
	int bbody_temp = CalcSurfaceTemp(star, averageDistToStar, albedo, greenhouse);

	averageTemp = bbody_temp;

	// radius is just the cube root of the mass. we get some more fractional
	// bits for small bodies otherwise we can easily end up with 0 radius
	// which breaks stuff elsewhere
	if (mass <= fixed(1,1))
		radius = fixed(fixedf<48>::CubeRootOf(fixedf<48>(mass)));
	else
		radius = fixed::CubeRootOf(mass);
	// enforce minimum size of 10km
	radius = std::max(radius, fixed(1,630));

	if (parent->type <= TYPE_STAR_MAX)
		// get it from the table now rather than setting it on stars/gravpoints as
		// currently nothing else needs them to have metallicity
		m_metallicity = StarSystem::starMetallicities[parent->type] * rand.Fixed();
	else
		// this assumes the parent's parent is a star/gravpoint, which is currently always true
		m_metallicity = StarSystem::starMetallicities[parent->parent->type] * rand.Fixed();
	// harder to be volcanic when you are tiny (you cool down)
	m_volcanicity = std::min(fixed(1,1), mass) * rand.Fixed();
	m_atmosOxidizing = rand.Fixed();
	m_life = fixed(0);
	m_volatileGas = fixed(0);
	m_volatileLiquid = fixed(0);
	m_volatileIces = fixed(0);

	// pick body type
	if (mass > 317*13) {
		// more than 13 jupiter masses can fuse deuterium - is a brown dwarf
		type = SystemBody::TYPE_BROWN_DWARF;
		averageTemp = averageTemp + rand.Int32(starTypeInfo[type].tempMin,
					starTypeInfo[type].tempMax);
		// prevent mass exceeding 65 jupiter masses or so, when it becomes a star
		// XXX since TYPE_BROWN_DWARF is supertype star, mass is now in
		// solar masses. what a fucking mess
		mass = std::min(mass, fixed(317*65, 1)) / 332998;
		//Radius is too high as it now uses the planetary calculations to work out radius (Cube root of mass)
		// So tell it to use the star data instead:
		radius = fixed(rand.Int32(starTypeInfo[type].radius[0],
				starTypeInfo[type].radius[1]), 100);
	} else if (mass > 6) {
		type = SystemBody::TYPE_PLANET_GAS_GIANT;
	} else if (mass > fixed(1, 15000)) {
		type = SystemBody::TYPE_PLANET_TERRESTRIAL;

		fixed amount_volatiles = fixed(2,1)*rand.Fixed();
		if (rand.Int32(3)) amount_volatiles *= mass;
		// total atmosphere loss
		if (rand.Fixed() > mass) amount_volatiles = fixed(0);

		//printf("Amount volatiles: %f\n", amount_volatiles.ToFloat());
		// fudge how much of the volatiles are in which state
		greenhouse = fixed(0);
		albedo = fixed(0);
		// CO2 sublimation
		if (averageTemp > 195) greenhouse += amount_volatiles * fixed(1,3);
		else albedo += fixed(2,6);
		// H2O liquid
		if (averageTemp > 273) greenhouse += amount_volatiles * fixed(1,5);
		else albedo += fixed(3,6);
		// H2O boils
		if (averageTemp > 373) greenhouse += amount_volatiles * fixed(1,3);

		averageTemp = CalcSurfaceTemp(star, averageDistToStar, albedo, greenhouse);

		const fixed proportion_gas = averageTemp / (fixed(100,1) + averageTemp);
		m_volatileGas = proportion_gas * amount_volatiles;

		const fixed proportion_liquid = (fixed(1,1)-proportion_gas) * (averageTemp / (fixed(50,1) + averageTemp));
		m_volatileLiquid = proportion_liquid * amount_volatiles;

		const fixed proportion_ices = fixed(1,1) - (proportion_gas + proportion_liquid);
		m_volatileIces = proportion_ices * amount_volatiles;

		//printf("temp %dK, gas:liquid:ices %f:%f:%f\n", averageTemp, proportion_gas.ToFloat(),
		//		proportion_liquid.ToFloat(), proportion_ices.ToFloat());

		if ((m_volatileLiquid > fixed(0)) &&
		    (averageTemp > CELSIUS-60) &&
		    (averageTemp < CELSIUS+200)) {
			// try for life
			int minTemp = CalcSurfaceTemp(star, maxDistToStar, albedo, greenhouse);
			int maxTemp = CalcSurfaceTemp(star, minDistToStar, albedo, greenhouse);

			if ((star->type != TYPE_BROWN_DWARF) &&
			    (star->type != TYPE_WHITE_DWARF) &&
			    (star->type != TYPE_STAR_O) &&
			    (minTemp > CELSIUS-10) && (minTemp < CELSIUS+90) &&
			    (maxTemp > CELSIUS-10) && (maxTemp < CELSIUS+90)) {
				m_life = rand.Fixed();
			}
		}
	} else {
		type = SystemBody::TYPE_PLANET_ASTEROID;
	}

    PickAtmosphere();
	PickRings();
}

void StarSystem::MakeShortDescription(MTRand &rand)
{
	m_econType = 0;
	if ((m_industrial > m_metallicity) && (m_industrial > m_agricultural)) {
		m_econType = ECON_INDUSTRY;
	} else if (m_metallicity > m_agricultural) {
		m_econType = ECON_MINING;
	} else {
		m_econType = ECON_AGRICULTURE;
	}

	if (m_unexplored) {
		m_shortDesc = Lang::UNEXPLORED_SYSTEM_NO_DATA;
	}

	/* Total population is in billions */
	else if(m_totalPop == 0) {
		m_shortDesc = Lang::SMALL_SCALE_PROSPECTING_NO_SETTLEMENTS;
	} else if (m_totalPop < fixed(1,10)) {
		switch (m_econType) {
			case ECON_INDUSTRY: m_shortDesc = Lang::SMALL_INDUSTRIAL_OUTPOST; break;
			case ECON_MINING: m_shortDesc = Lang::SOME_ESTABLISHED_MINING; break;
			case ECON_AGRICULTURE: m_shortDesc = Lang::YOUNG_FARMING_COLONY; break;
		}
	} else if (m_totalPop < fixed(1,2)) {
		switch (m_econType) {
			case ECON_INDUSTRY: m_shortDesc = Lang::INDUSTRIAL_COLONY; break;
			case ECON_MINING: m_shortDesc = Lang::MINING_COLONY; break;
			case ECON_AGRICULTURE: m_shortDesc = Lang::OUTDOOR_AGRICULTURAL_WORLD; break;
		}
	} else if (m_totalPop < fixed(5,1)) {
		switch (m_econType) {
			case ECON_INDUSTRY: m_shortDesc = Lang::HEAVY_INDUSTRY; break;
			case ECON_MINING: m_shortDesc = Lang::EXTENSIVE_MINING; break;
			case ECON_AGRICULTURE: m_shortDesc = Lang::THRIVING_OUTDOOR_WORLD; break;
		}
	} else {
		switch (m_econType) {
			case ECON_INDUSTRY: m_shortDesc = Lang::INDUSTRIAL_HUB_SYSTEM; break;
			case ECON_MINING: m_shortDesc = Lang::VAST_STRIP_MINE; break;
			case ECON_AGRICULTURE: m_shortDesc = Lang::HIGH_POPULATION_OUTDOOR_WORLD; break;
		}
	}
}

const Color StarSystem::GetFactionColour() const
{
	if (m_factionIdx != Faction::BAD_FACTION_IDX) {
		const Faction *fac = Faction::GetFaction(m_factionIdx);
		if( fac ) {
			return fac->colour;
		}
	}
	return Color(0.8f,0.8f,0.8f,0.5f);
}

/* percent */
#define MAX_COMMODITY_BASE_PRICE_ADJUSTMENT 25

void StarSystem::Populate(bool addSpaceStations)
{
	unsigned long _init[5] = { m_path.systemIndex, Uint32(m_path.sectorX), Uint32(m_path.sectorY), Uint32(m_path.sectorZ), UNIVERSE_SEED };
	MTRand rand;
	rand.seed(_init, 5);

	/* Various system-wide characteristics */
	// This is 1 in sector (0,0,0) and approaches 0 farther out
	// (1,0,0) ~ .688, (1,1,0) ~ .557, (1,1,1) ~ .48
	m_humanProx = fixed(3,1) / isqrt(9 + 10*(m_path.sectorX*m_path.sectorX + m_path.sectorY*m_path.sectorY + m_path.sectorZ*m_path.sectorZ));
	m_econType = ECON_INDUSTRY;
	m_industrial = rand.Fixed();
	m_agricultural = 0;

	// find the faction we're probably aligned with
	m_factionIdx = Faction::GetNearestFactionIndex(m_path);

	/* system attributes */
	m_totalPop = fixed(0);
	rootBody->PopulateStage1(this, m_totalPop);

//	printf("Trading rates:\n");
	// So now we have balances of trade of various commodities.
	// Lets use black magic to turn these into percentage base price
	// alterations
	int maximum = 0;
	for (int i=Equip::FIRST_COMMODITY; i<=Equip::LAST_COMMODITY; i++) {
		maximum = std::max(abs(m_tradeLevel[i]), maximum);
	}
	if (maximum) for (int i=Equip::FIRST_COMMODITY; i<=Equip::LAST_COMMODITY; i++) {
		m_tradeLevel[i] = (m_tradeLevel[i] * MAX_COMMODITY_BASE_PRICE_ADJUSTMENT) / maximum;
		m_tradeLevel[i] += rand.Int32(-5, 5);
	}

// Unused?
//	for (int i=(int)Equip::FIRST_COMMODITY; i<=(int)Equip::LAST_COMMODITY; i++) {
//		Equip::Type t = (Equip::Type)i;
//		const EquipType &type = Equip::types[t];
//		printf("%s: %d%%\n", type.name, m_tradeLevel[t]);
//	}
//	printf("System total population %.3f billion\n", m_totalPop.ToFloat());
	Polit::GetSysPolitStarSystem(this, m_totalPop, m_polit);

	if (addSpaceStations) {
		rootBody->PopulateAddStations(this);
	}

	if (!m_shortDesc.size())
		MakeShortDescription(rand);
}

/*
 * Set natural resources, tech level, industry strengths and population levels
 */
void SystemBody::PopulateStage1(StarSystem *system, fixed &outTotalPop)
{
	for (unsigned int i=0; i<children.size(); i++) {
		children[i]->PopulateStage1(system, outTotalPop);
	}

	// unexplored systems have no population (that we know about)
	if (system->m_unexplored) {
		m_population = outTotalPop = fixed(0);
		return;
	}

	// grav-points have no population themselves
	if (type == SystemBody::TYPE_GRAVPOINT) {
		m_population = fixed(0);
		return;
	}

	unsigned long _init[6] = { system->m_path.systemIndex, Uint32(system->m_path.sectorX),
			Uint32(system->m_path.sectorY), Uint32(system->m_path.sectorZ), UNIVERSE_SEED, Uint32(this->seed) };

	MTRand rand, namerand;
	rand.seed(_init, 6);
	namerand.seed(_init, 6);

	m_population = fixed(0);

	/* Bad type of planet for settlement */
	if ((averageTemp > CELSIUS+100) || (averageTemp < 100) ||
	    (type != SystemBody::TYPE_PLANET_TERRESTRIAL && type != SystemBody::TYPE_PLANET_ASTEROID)) {

        // orbital starports should carry a small amount of population
        if (type == SystemBody::TYPE_STARPORT_ORBITAL) {
			m_population = fixed(1,100000);
			outTotalPop += m_population;
        }

		return;
	}

	m_agricultural = fixed(0);

	if (m_life > fixed(9,10)) {
		m_agricultural = Clamp(fixed(1,1) - fixed(CELSIUS+25-averageTemp, 40), fixed(0), fixed(1,1));
		system->m_agricultural += 2*m_agricultural;
	} else if (m_life > fixed(1,2)) {
		m_agricultural = Clamp(fixed(1,1) - fixed(CELSIUS+30-averageTemp, 50), fixed(0), fixed(1,1));
		system->m_agricultural += 1*m_agricultural;
	} else {
		// don't bother populating crap planets
		if (m_metallicity < fixed(5,10) &&
			m_metallicity < (fixed(1,1) - system->m_humanProx)) return;
	}

	const int NUM_CONSUMABLES = 10;
	const Equip::Type consumables[NUM_CONSUMABLES] = {
		Equip::AIR_PROCESSORS,
		Equip::GRAIN,
		Equip::FRUIT_AND_VEG,
		Equip::ANIMAL_MEAT,
		Equip::LIQUOR,
		Equip::CONSUMER_GOODS,
		Equip::MEDICINES,
		Equip::HAND_WEAPONS,
		Equip::NARCOTICS,
		Equip::LIQUID_OXYGEN
	};

	/* Commodities we produce (mining and agriculture) */
	for (int i=Equip::FIRST_COMMODITY; i<Equip::LAST_COMMODITY; i++) {
		Equip::Type t = Equip::Type(i);
		const EquipType &itype = Equip::types[t];

		fixed affinity = fixed(1,1);
		if (itype.econType & ECON_AGRICULTURE) {
			affinity *= 2*m_agricultural;
		}
		if (itype.econType & ECON_INDUSTRY) affinity *= system->m_industrial;
		// make industry after we see if agriculture and mining are viable
		if (itype.econType & ECON_MINING) {
			affinity *= m_metallicity;
		}
		affinity *= rand.Fixed();
		// producing consumables is wise
		for (int j=0; j<NUM_CONSUMABLES; j++) {
			if (i == consumables[j]) {
				affinity *= 2;
				break;
			}
		}
		assert(affinity >= 0);
		/* workforce... */
		m_population += affinity * system->m_humanProx;

		int howmuch = (affinity * 256).ToInt32();

		system->m_tradeLevel[t] += -2*howmuch;
		for (int j=0; j<EQUIP_INPUTS; j++) {
			if (!itype.inputs[j]) continue;
			system->m_tradeLevel[itype.inputs[j]] += howmuch;
		}
	}

	if (!system->m_hasCustomBodies && m_population > 0)
		name = Pi::luaNameGen->BodyName(this, namerand);

	// Add a bunch of things people consume
	for (int i=0; i<NUM_CONSUMABLES; i++) {
		Equip::Type t = consumables[i];
		if (m_life > fixed(1,2)) {
			// life planets can make this jizz probably
			if ((t == Equip::AIR_PROCESSORS) ||
			    (t == Equip::LIQUID_OXYGEN) ||
			    (t == Equip::GRAIN) ||
			    (t == Equip::FRUIT_AND_VEG) ||
			    (t == Equip::ANIMAL_MEAT)) {
				continue;
			}
		}
		system->m_tradeLevel[t] += rand.Int32(32,128);
	}
	// well, outdoor worlds should have way more people
	m_population = fixed(1,10)*m_population + m_population*m_agricultural;

//	printf("%s: pop %.3f billion\n", name.c_str(), m_population.ToFloat());

	outTotalPop += m_population;
}

void SystemBody::PopulateAddStations(StarSystem *system)
{
	for (unsigned int i=0; i<children.size(); i++) {
		children[i]->PopulateAddStations(system);
	}

	unsigned long _init[6] = { system->m_path.systemIndex, Uint32(system->m_path.sectorX),
			Uint32(system->m_path.sectorY), Uint32(system->m_path.sectorZ), this->seed, UNIVERSE_SEED };

	MTRand rand, namerand;
	rand.seed(_init, 6);
	namerand.seed(_init, 6);

	if (m_population < fixed(1,1000)) return;

	fixed pop = m_population + rand.Fixed();

	fixed orbMaxS = fixed(1,4)*this->CalcHillRadius();
	fixed orbMinS = 4 * this->radius * AU_EARTH_RADIUS;
	if (children.size()) orbMaxS = std::min(orbMaxS, fixed(1,2) * children[0]->orbMin);

	// starports - orbital
	pop -= rand.Fixed();
	if ((orbMinS < orbMaxS) && (pop >= 0)) {

		SystemBody *sp = system->NewBody();
		sp->type = SystemBody::TYPE_STARPORT_ORBITAL;
		sp->seed = rand.Int32();
		sp->tmp = 0;
		sp->parent = this;
		sp->rotationPeriod = fixed(1,3600);
		sp->averageTemp = this->averageTemp;
		sp->mass = 0;
		/* just always plonk starports in near orbit */
		sp->semiMajorAxis = orbMinS;
		sp->eccentricity = fixed(0);
		sp->axialTilt = fixed(0);
		sp->orbit.eccentricity = 0;
		sp->orbit.semiMajorAxis = sp->semiMajorAxis.ToDouble()*AU;
		sp->orbit.period = calc_orbital_period(sp->orbit.semiMajorAxis, this->mass.ToDouble() * EARTH_MASS);
		sp->orbit.rotMatrix = matrix4x4d::Identity();
		children.insert(children.begin(), sp);
		system->m_spaceStations.push_back(sp);
		sp->orbMin = sp->semiMajorAxis;
		sp->orbMax = sp->semiMajorAxis;

		sp->name = Pi::luaNameGen->BodyName(sp, namerand);

		pop -= rand.Fixed();
		if (pop > 0) {
			SystemBody *sp2 = system->NewBody();
			SystemPath path2 = sp2->path;
			*sp2 = *sp;
			sp2->path = path2;
			sp2->orbit.rotMatrix = matrix4x4d::RotateZMatrix(M_PI);
			sp2->name = Pi::luaNameGen->BodyName(sp2, namerand);
			children.insert(children.begin(), sp2);
			system->m_spaceStations.push_back(sp2);
		}
	}
	// starports - surface
	pop = m_population + rand.Fixed();
	int max = 6;
	while (max-- > 0) {
		pop -= rand.Fixed();
		if (pop < 0) break;

		SystemBody *sp = system->NewBody();
		sp->type = SystemBody::TYPE_STARPORT_SURFACE;
		sp->seed = rand.Int32();
		sp->tmp = 0;
		sp->parent = this;
		sp->averageTemp = this->averageTemp;
		sp->mass = 0;
		sp->name = Pi::luaNameGen->BodyName(sp, namerand);
		memset(&sp->orbit, 0, sizeof(Orbit));
		position_settlement_on_planet(sp);
		children.insert(children.begin(), sp);
		system->m_spaceStations.push_back(sp);
	}
}

StarSystem::~StarSystem()
{
	if (rootBody) delete rootBody;
}

SystemBody::~SystemBody()
{
	for (std::vector<SystemBody*>::iterator i = children.begin(); i != children.end(); ++i) {
		delete (*i);
	}
}

void StarSystem::Serialize(Serializer::Writer &wr, StarSystem *s)
{
	if (s) {
		wr.Byte(1);
		wr.Int32(s->m_path.sectorX);
		wr.Int32(s->m_path.sectorY);
		wr.Int32(s->m_path.sectorZ);
		wr.Int32(s->m_path.systemIndex);
	} else {
		wr.Byte(0);
	}
}

RefCountedPtr<StarSystem> StarSystem::Unserialize(Serializer::Reader &rd)
{
	if (rd.Byte()) {
		int sec_x = rd.Int32();
		int sec_y = rd.Int32();
		int sec_z = rd.Int32();
		int sys_idx = rd.Int32();
		return StarSystem::GetCached(SystemPath(sec_x, sec_y, sec_z, sys_idx));
	} else {
		return RefCountedPtr<StarSystem>(0);
	}
}

typedef std::map<SystemPath,StarSystem*> SystemCacheMap;
static SystemCacheMap s_cachedSystems;

RefCountedPtr<StarSystem> StarSystem::GetCached(const SystemPath &path)
{
	SystemPath sysPath(path.SystemOnly());

	StarSystem *s = 0;
	std::pair<SystemCacheMap::iterator, bool>
		ret = s_cachedSystems.insert(SystemCacheMap::value_type(sysPath, static_cast<StarSystem*>(0)));
	if (ret.second) {
		s = new StarSystem(sysPath);
		ret.first->second = s;
		s->IncRefCount(); // the cache owns one reference
	} else {
		s = ret.first->second;
	}
	return RefCountedPtr<StarSystem>(s);
}

void StarSystem::ShrinkCache()
{
	std::map<SystemPath,StarSystem*>::iterator i = s_cachedSystems.begin();
	while (i != s_cachedSystems.end()) {
		StarSystem *s = (*i).second;
		assert(s->GetRefCount() >= 1); // sanity check
		// if the cache is the only owner, then delete it
		if (s->GetRefCount() == 1) {
			delete s;
			s_cachedSystems.erase(i++);
		}
		else
			i++;
	}
}
