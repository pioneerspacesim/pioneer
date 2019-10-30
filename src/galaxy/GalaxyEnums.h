#ifndef GALAXYENUMS_H_INCLUDED
#define GALAXYENUMS_H_INCLUDED

#include "Color.h"

namespace GalaxyEnums {

	enum BodyType { // <enum scope='SystemBody' prefix=TYPE_ public>
		TYPE_GRAVPOINT = 0,
		TYPE_BROWN_DWARF = 1, //  L+T Class Brown Dwarfs
		TYPE_WHITE_DWARF = 2,
		TYPE_STAR_M = 3, //red
		TYPE_STAR_K = 4, //orange
		TYPE_STAR_G = 5, //yellow
		TYPE_STAR_F = 6, //white
		TYPE_STAR_A = 7, //blue/white
		TYPE_STAR_B = 8, //blue
		TYPE_STAR_O = 9, //blue/purple/white
		TYPE_STAR_M_GIANT = 10,
		TYPE_STAR_K_GIANT = 11,
		TYPE_STAR_G_GIANT = 12,
		TYPE_STAR_F_GIANT = 13,
		TYPE_STAR_A_GIANT = 14,
		TYPE_STAR_B_GIANT = 15,
		TYPE_STAR_O_GIANT = 16,
		TYPE_STAR_M_SUPER_GIANT = 17,
		TYPE_STAR_K_SUPER_GIANT = 18,
		TYPE_STAR_G_SUPER_GIANT = 19,
		TYPE_STAR_F_SUPER_GIANT = 20,
		TYPE_STAR_A_SUPER_GIANT = 21,
		TYPE_STAR_B_SUPER_GIANT = 22,
		TYPE_STAR_O_SUPER_GIANT = 23,
		TYPE_STAR_M_HYPER_GIANT = 24,
		TYPE_STAR_K_HYPER_GIANT = 25,
		TYPE_STAR_G_HYPER_GIANT = 26,
		TYPE_STAR_F_HYPER_GIANT = 27,
		TYPE_STAR_A_HYPER_GIANT = 28,
		TYPE_STAR_B_HYPER_GIANT = 29,
		TYPE_STAR_O_HYPER_GIANT = 30, // these various stars do exist = they are transitional states and are rare
		TYPE_STAR_M_WF = 31, //Wolf-Rayet star
		TYPE_STAR_B_WF = 32, // while you do not specifically get class M,B or O WF stars,
		TYPE_STAR_O_WF = 33, //  you do get red = blue and purple from the colour of the gasses = so spectral class is an easy way to define them.
		TYPE_STAR_S_BH = 34, //stellar blackhole
		TYPE_STAR_IM_BH = 35, //Intermediate-mass blackhole
		TYPE_STAR_SM_BH = 36, //Supermassive blackhole
		TYPE_PLANET_GAS_GIANT = 37,
		TYPE_PLANET_ASTEROID = 38,
		TYPE_PLANET_TERRESTRIAL = 39,
		TYPE_STARPORT_ORBITAL = 40,
		TYPE_STARPORT_SURFACE = 41,
		TYPE_MIN = TYPE_BROWN_DWARF, // <enum skip>
		TYPE_MAX = TYPE_STARPORT_SURFACE, // <enum skip>
		TYPE_STAR_MIN = TYPE_BROWN_DWARF, // <enum skip>
		TYPE_STAR_MAX = TYPE_STAR_SM_BH, // <enum skip>
		// XXX need larger atmosphereless thing
	};

	enum BodySuperType { // <enum scope='SystemBody' prefix=SUPERTYPE_ public>
		SUPERTYPE_NONE = 0,
		SUPERTYPE_STAR = 1,
		SUPERTYPE_ROCKY_PLANET = 2,
		SUPERTYPE_GAS_GIANT = 3,
		SUPERTYPE_STARPORT = 4,
	};

	// indexed by enum type turd
	const Color starColors[] = {
		{ 0, 0, 0 }, // gravpoint
		{ 128, 0, 0 }, // brown dwarf
		{ 102, 102, 204 }, // white dwarf
		{ 255, 51, 0 }, // M
		{ 255, 153, 26 }, // K
		{ 255, 255, 102 }, // G
		{ 255, 255, 204 }, // F
		{ 255, 255, 255 }, // A
		{ 178, 178, 255 }, // B
		{ 255, 178, 255 }, // O
		{ 255, 51, 0 }, // M Giant
		{ 255, 153, 26 }, // K Giant
		{ 255, 255, 102 }, // G Giant
		{ 255, 255, 204 }, // F Giant
		{ 255, 255, 255 }, // A Giant
		{ 178, 178, 255 }, // B Giant
		{ 255, 178, 255 }, // O Giant
		{ 255, 51, 0 }, // M Super Giant
		{ 255, 153, 26 }, // K Super Giant
		{ 255, 255, 102 }, // G Super Giant
		{ 255, 255, 204 }, // F Super Giant
		{ 255, 255, 255 }, // A Super Giant
		{ 178, 178, 255 }, // B Super Giant
		{ 255, 178, 255 }, // O Super Giant
		{ 255, 51, 0 }, // M Hyper Giant
		{ 255, 153, 26 }, // K Hyper Giant
		{ 255, 255, 102 }, // G Hyper Giant
		{ 255, 255, 204 }, // F Hyper Giant
		{ 255, 255, 255 }, // A Hyper Giant
		{ 178, 178, 255 }, // B Hyper Giant
		{ 255, 178, 255 }, // O Hyper Giant
		{ 255, 51, 0 }, // Red/M Wolf Rayet Star
		{ 178, 178, 255 }, // Blue/B Wolf Rayet Star
		{ 255, 178, 255 }, // Purple-Blue/O Wolf Rayet Star
		{ 76, 178, 76 }, // Stellar Blackhole
		{ 51, 230, 51 }, // Intermediate mass Black-hole
		{ 0, 255, 0 } // Super massive black hole
	};

	// indexed by enum type turd
	const Color starRealColors[] = {
		{ 0, 0, 0 }, // gravpoint
		{ 128, 0, 0 }, // brown dwarf
		{ 255, 255, 255 }, // white dwarf
		{ 255, 128, 51 }, // M
		{ 255, 255, 102 }, // K
		{ 255, 255, 242 }, // G
		{ 255, 255, 255 }, // F
		{ 255, 255, 255 }, // A
		{ 204, 204, 255 }, // B
		{ 255, 204, 255 }, // O
		{ 255, 128, 51 }, // M Giant
		{ 255, 255, 102 }, // K Giant
		{ 255, 255, 242 }, // G Giant
		{ 255, 255, 255 }, // F Giant
		{ 255, 255, 255 }, // A Giant
		{ 204, 204, 255 }, // B Giant
		{ 255, 204, 255 }, // O Giant
		{ 255, 128, 51 }, // M Super Giant
		{ 255, 255, 102 }, // K Super Giant
		{ 255, 255, 242 }, // G Super Giant
		{ 255, 255, 255 }, // F Super Giant
		{ 255, 255, 255 }, // A Super Giant
		{ 204, 204, 255 }, // B Super Giant
		{ 255, 204, 255 }, // O Super Giant
		{ 255, 128, 51 }, // M Hyper Giant
		{ 255, 255, 102 }, // K Hyper Giant
		{ 255, 255, 242 }, // G Hyper Giant
		{ 255, 255, 255 }, // F Hyper Giant
		{ 255, 255, 255 }, // A Hyper Giant
		{ 204, 204, 255 }, // B Hyper Giant
		{ 255, 204, 255 }, // O Hyper Giant
		{ 255, 153, 153 }, // M WF
		{ 204, 204, 255 }, // B WF
		{ 255, 204, 255 }, // O WF
		{ 22, 0, 24 }, // small Black hole
		{ 16, 0, 20 }, // med BH
		{ 10, 0, 16 } // massive BH
	};

	const double starLuminosities[] = {
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

	const float starScale[] = {
		// Used in sector view
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
		4.0f // Supermassive blackhole
	};
}

#endif // GALAXYENUMS_H_INCLUDED
