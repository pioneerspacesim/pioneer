#include "libs.h"
#include "custom_starsystems.h"
#include "StarSystem.h"


const CustomSBody sol_system[] = {
	{ "Sol", SBody::TYPE_STAR_G,
	  -1, fixed(1,1), fixed(1,1), 5700 },
	{ "Mercury", SBody::TYPE_PLANET_SMALL,
	  0, fixed(38,100), fixed(55,1000), 340,
	  fixed(387,1000), fixed(205,1000), DEG2RAD(7.0), fixed(59,1) },
	{ "Venus", SBody::TYPE_PLANET_CO2_THICK_ATMOS,
	  0, fixed(95,100), fixed(815,1000), 735,
	  fixed(723,1000), fixed(7,1000), DEG2RAD(3.39), fixed(243,1) },
	{ "Earth", SBody::TYPE_PLANET_INDIGENOUS_LIFE,
	  0, fixed(1,1), fixed(1,1), 288,
	  fixed(1,1), fixed(167,10000), 0, fixed(1,1), ECON_INDUSTRY },
	{ "Mexico City", SBody::TYPE_STARPORT_SURFACE,
	  3, 0, 0, 0, 0, 0, DEG2RAD(19) },
	{ "Shanghai", SBody::TYPE_STARPORT_SURFACE,
	  3, 0, 0, 0, 0, 0, DEG2RAD(31) },
	{ "London", SBody::TYPE_STARPORT_SURFACE,
	  3, 0, 0, 0, 0, 0, DEG2RAD(51) },
	{ "Moscow", SBody::TYPE_STARPORT_SURFACE,
	  3, 0, 0, 0, 0, 0, DEG2RAD(55) },
	{ "Brasilia", SBody::TYPE_STARPORT_SURFACE,
	  3, 0, 0, 0, 0, 0, DEG2RAD(-15.5) },
	{ "Los Angeles", SBody::TYPE_STARPORT_SURFACE,
	  3, 0, 0, 0, 0, 0, DEG2RAD(34) },
	{ "Moon", SBody::TYPE_PLANET_DWARF,
	  3, fixed(273,1000), fixed(12,1000), 220,
	  fixed(257,100000), fixed(549,10000), DEG2RAD(5.145), fixed(273,10) },
	{ "Mars", SBody::TYPE_PLANET_SMALL,
	  0, fixed(533,1000), fixed(107,1000), 227,
	  fixed(152,100), fixed(933,10000), DEG2RAD(1.85), fixed(1027,1000) },
	{ "Jupiter", SBody::TYPE_PLANET_LARGE_GAS_GIANT,
	  0, fixed(11,1), fixed(3178,10), 165,
	  fixed(5204,1000), fixed(488,10000), DEG2RAD(1.305), fixed(4,10) },
	{ "Saturn", SBody::TYPE_PLANET_MEDIUM_GAS_GIANT,
	  0, fixed(9,1), fixed(95152,1000), 134,
	  fixed(9582,1000), fixed(557,10000), DEG2RAD(2.485), fixed(4,10) },
	{ "Uranus", SBody::TYPE_PLANET_SMALL_GAS_GIANT,
	  0, fixed(4,1), fixed(145,10), 76,
	  fixed(19229,1000), fixed(444,10000), DEG2RAD(0.772), fixed(7,10) },
	{ "Neptune", SBody::TYPE_PLANET_SMALL_GAS_GIANT,
	  0, fixed(38,10), fixed(17147,100), 72,
	  fixed(30104,1000), fixed(112,10000), DEG2RAD(1.768), fixed(75,100) },
	// moons of jupiter
	{ "Io", SBody::TYPE_PLANET_HIGHLY_VOLCANIC,
	  12, fixed(286,1000), fixed(15,1000), 130,
	  fixed(282,100000), fixed(41,10000), DEG2RAD(2.21), fixed(177,100) },
	{ "Europa", SBody::TYPE_PLANET_WATER,
	  12, fixed(245,1000), fixed(8,1000), 102,
	  fixed(441,100000), fixed(9,1000), 0.0, fixed(355,100) },
	  
	{ 0 }
};

const CustomSystem custom_systems[] = {
	{ "Sol", sol_system, {SBody::TYPE_STAR_G}, 0, 0, vector3f(.5, .5, 0), 0,
       "The historical birthplace of humankind",
	"Sol is a fine joint"	},
{ "Epsilon Indi", 0, {SBody::TYPE_STAR_K}, -1, 0, vector3f(0.629,0.880,-1.045) }, // Components: K5
{ "Luyten 1159-016", 0, {SBody::TYPE_STAR_M}, 2, 0, vector3f(0.316,0.651,0.008) }, // Components: M8
{ "Wolf 424", 0, {SBody::TYPE_STAR_M, SBody::TYPE_STAR_M}, 0, -2, vector3f(0.189,0.951,0.771) }, // Components: M5, M7
{ "DX Cancri", 0, {SBody::TYPE_STAR_M}, 1, 0, vector3f(0.701,0.862,0.784) }, // Components: M6
{ "LTT 17897", 0, {SBody::TYPE_STAR_M, SBody::TYPE_STAR_K, SBody::TYPE_STAR_M, SBody::TYPE_STAR_M}, 2, 1, vector3f(0.460,0.384,-0.402) }, // Components: M4, K, M, M6
{ "EV Lacertae", 0, {SBody::TYPE_STAR_M}, 0, -2, vector3f(0.864,0.870,1.249) }, // Components: M4
{ "Luyten 354-89", 0, {SBody::TYPE_STAR_M}, -1, 0, vector3f(0.204,0.746,-1.365) }, // Components: M1
{ "Gliese   1", 0, {SBody::TYPE_STAR_M}, -1, 1, vector3f(0.657,0.679,-1.203) }, // Components: M4
{ "Gliese 570", 0, {SBody::TYPE_STAR_K, SBody::TYPE_STAR_M}, -2, 0, vector3f(0.174,0.358,-0.281) }, // Components: K5, M2
{ "Kruger 60", 0, {SBody::TYPE_STAR_M, SBody::TYPE_STAR_M}, 0, -2, vector3f(0.903,0.932,0.004) }, // Components: M2, M6
{ "Gliese 205", 0, {SBody::TYPE_STAR_M}, 2, 1, vector3f(0.488,0.496,-0.805) }, // Components: M1
{ "Proxima", 0, {SBody::TYPE_STAR_M}, 0, 0, vector3f(0.135,0.882,-0.017) }, // Components: M5
{ "Gliese 825", 0, {SBody::TYPE_STAR_M}, -1, 0, vector3f(0.365,0.424,-1.093) }, // Components: M0
{ "Gliese 876", 0, {SBody::TYPE_STAR_M}, -2, 0, vector3f(0.603,0.900,-0.112) }, // Components: M5
{ "Gliese 725", 0, {SBody::TYPE_STAR_M, SBody::TYPE_STAR_M}, 0, -1, vector3f(0.482,0.203,0.590) }, // Components: M4, M5
{ "Sigma Draconis", 0, {SBody::TYPE_STAR_K}, 0, -2, vector3f(0.914,0.420,0.859) }, // Components: K0
{ "Gliese 380", 0, {SBody::TYPE_STAR_K}, 1, -1, vector3f(0.979,0.462,0.631) }, // Components: K2
{ "Sirius", 0, {SBody::TYPE_STAR_A, SBody::TYPE_WHITE_DWARF}, 1, 1, vector3f(0.222,0.273,-0.173), 0,
"Corporate system",
"The Sirius system is home to Sirius Corporation, market leader in Robotics, Neural Computing, "
"Security and Defence Systems, to name but a few of its endeavours. Sirius research and development "
"institutes are at the very cutting edge of galactic science. The young, bright and ambitious from "
"worlds all over galaxy flock to Sirius to make a name for themselves.\n"
"Above text all rights reserved Sirius Corporation."

}, // Components: A1, DA2
{ "Gliese 682", 0, {SBody::TYPE_STAR_M}, -2, 0, vector3f(0.643,0.971,-0.213) }, // Components: M3
{ "AD Leonis", 0, {SBody::TYPE_STAR_M}, 2, 0, vector3f(0.483,0.012,-0.071) }, // Components: M4
{ "Luyten 372-58", 0, {SBody::TYPE_STAR_M}, 0, 1, vector3f(0.822,0.495,-1.402) }, // Components: M4
{ "Gliese 191", 0, {SBody::TYPE_STAR_M}, 0, 1, vector3f(0.925,0.696,-0.938) }, // Components: M0
{ "Gliese 663", 0, {SBody::TYPE_STAR_K}, -2, 0, vector3f(0.279,0.572,0.278) }, // Components: K5
{ "70 Ophiuchi", 0, {SBody::TYPE_STAR_K, SBody::TYPE_STAR_K}, -2, -1, vector3f(0.758,0.509,0.420) }, // Components: K0, K5
{ "Ross 614", 0, {SBody::TYPE_STAR_M}, 1, 1, vector3f(0.908,0.402,-0.195) }, // Components: M4
{ "GJ 1245", 0, {SBody::TYPE_STAR_M, SBody::TYPE_STAR_M}, 0, -2, vector3f(0.126,0.637,0.295) }, // Components: M5, M
{ "Gliese 526", 0, {SBody::TYPE_STAR_M}, -1, -2, vector3f(0.418,0.839,0.993) }, // Components: M4
{ "Ross 986", 0, {SBody::TYPE_STAR_M}, 2, 0, vector3f(0.835,0.449,0.829) }, // Components: M4
{ "ADS 10417", 0, {SBody::TYPE_STAR_K, SBody::TYPE_STAR_K}, -2, 0, vector3f(0.346,0.576,0.274) }, // Components: K1, K1
{ "GJ 1002", 0, {SBody::TYPE_STAR_M}, -1, 2, vector3f(0.778,0.260,-0.521) }, // Components: M5
{ "Ross 154", 0, {SBody::TYPE_STAR_M}, -1, 0, vector3f(0.344,0.274,-0.205) }, // Components: M4
{ "L 674-015", 0, {SBody::TYPE_STAR_M}, 1, 2, vector3f(0.885,0.442,-0.508) }, // Components: M4
{ "Wolf 359", 0, {SBody::TYPE_STAR_M}, 0, 0, vector3f(0.746,0.990,0.806) }, // Components: M6
{ "AC+79:3888", 0, {SBody::TYPE_STAR_M}, 1, -1, vector3f(0.511,0.157,1.308) }, // Components: M4
{ "Luyten 789-006", 0, {SBody::TYPE_STAR_M}, -1, 0, vector3f(0.118,0.781,0.059) }, // Components: M5
{ "Gliese 674", 0, {SBody::TYPE_STAR_M}, -2, 1, vector3f(0.739,0.046,-0.209) }, // Components: M3
{ "Procyon", 0, {SBody::TYPE_STAR_F, SBody::TYPE_WHITE_DWARF}, 1, 1, vector3f(0.663,0.267,0.311) }, // Components: F5, DA
{ "Van Maanen's Star", 0, {SBody::TYPE_WHITE_DWARF}, 2, 0, vector3f(0.279,0.482,-0.330) }, // Components: DZ7
{ "Delta Pavonis", 0, {SBody::TYPE_STAR_G}, -2, 1, vector3f(0.796,0.492,-1.240) }, // Components: G8
{ "Gliese 752", 0, {SBody::TYPE_STAR_M, SBody::TYPE_STAR_M}, -2, -1, vector3f(0.739,0.013,-0.114) }, // Components: M3, M5
{ "Gliese 687", 0, {SBody::TYPE_STAR_M}, 0, -2, vector3f(0.744,0.895,1.020) }, // Components: M3
{ "40 Eridani", 0, {SBody::TYPE_STAR_K, SBody::TYPE_WHITE_DWARF, SBody::TYPE_STAR_M}, 1, 1, vector3f(0.943,0.037,-1.224) }, // Components: K1, DA4, M4
{ "AC+12:1800-213", 0, {SBody::TYPE_STAR_M}, 2, 1, vector3f(0.848,0.058,-0.410) }, // Components: M4
{ "Eta Cassiopeia", 0, {SBody::TYPE_STAR_G, SBody::TYPE_STAR_K}, 1, -2, vector3f(0.913,0.976,1.272) }, // Components: G3, K7
{ "Hei 299", 0, {SBody::TYPE_STAR_M}, -1, 2, vector3f(0.813,0.384,-0.843) }, // Components: M4
{ "Ross 128", 0, {SBody::TYPE_STAR_M}, 1, 0, vector3f(0.838,0.638,-0.344) }, // Components: M4
{ "Gliese 628", 0, {SBody::TYPE_STAR_M}, -2, 0, vector3f(0.982,0.421,0.680) }, // Components: M3
{ "Gliese 229", 0, {SBody::TYPE_STAR_M}, 1, 2, vector3f(0.964,0.149,-0.754) }, // Components: M1
{ "Luyten 145-141", 0, {SBody::TYPE_WHITE_DWARF}, -1, 2, vector3f(0.691,0.179,-0.095) }, // Components: DQ6
{ "EI Cancri", 0, {SBody::TYPE_STAR_M, SBody::TYPE_STAR_M}, 2, 0, vector3f(0.621,0.679,0.148) }, // Components: M, M
{ "Altair", 0, {SBody::TYPE_STAR_A}, -1, -1, vector3f(0.142,0.022,-0.299) }, // Components: A7
{ "Gliese 887", 0, {SBody::TYPE_STAR_M}, -1, 1, vector3f(0.413,0.547,-0.115) }, // Components: M2
{ "Gliese 908", 0, {SBody::TYPE_STAR_M}, -1, -2, vector3f(0.406,0.751,0.998) }, // Components: M2
{ "Gliese 588", 0, {SBody::TYPE_STAR_M}, -2, 0, vector3f(0.185,0.968,-0.790) }, // Components: M3
{ "Tau Ceti", 0, {SBody::TYPE_STAR_G}, 1, 1, vector3f(0.127,0.778,-0.324) }, // Components: G8
{ "LTT 12976", 0, {SBody::TYPE_STAR_M, SBody::TYPE_STAR_M}, 1, -1, vector3f(0.986,0.133,0.773) }, // Components: M2, M6
{ "Sm 50", 0, {SBody::TYPE_STAR_M}, -2, 0, vector3f(0.411,0.693,-1.341) }, // Components: M0
{ "61 Cygni", 0, {SBody::TYPE_STAR_K, SBody::TYPE_STAR_K}, 0, -1, vector3f(0.302,0.108,-0.136) }, // Components: K5, K7
{ "L 347-014", 0, {SBody::TYPE_STAR_M}, -2, 0, vector3f(0.390,0.787,-0.927) }, // Components: M4
{ "Wolf 629", 0, {SBody::TYPE_STAR_M}, -2, 0, vector3f(0.334,0.090,0.872) }, // Components: M4
{ "Luyten's Star", 0, {SBody::TYPE_STAR_M}, 1, 1, vector3f(0.788,0.305,0.266) }, // Components: M3
{ "Gliese 783", 0, {SBody::TYPE_STAR_K, SBody::TYPE_STAR_M}, -2, 0, vector3f(0.522,0.324,-1.169) }, // Components: K3, M3
{ "Barnard's star", 0, {SBody::TYPE_STAR_M}, -1, 0, vector3f(0.877,0.131,0.186) }, // Components: M5
{ "Epsilon Eridani", 0, {SBody::TYPE_STAR_K}, 1, 0, vector3f(0.351,0.735,-0.999), 5/*5=2 earths*/,
"First off-earth colony. Industrial world with indigenous life.",
"Epsilon Eridani was the first star system beyond Sol to be colonised by humanity. "
"The New Hope colony on the life-bearing planet of the same name was founded in 2279. "
"Its 1520 initial inhabitants completed their pre-hyperspace voyage of 10.7 lightyears "
"from Sol in just under 25 years.\n"
"Mass emigration from Earth in the 27th century drove a population explosion and today "
"Epsilon Eridani counts itself among the most populous of inhabited systems.\n"
"The system's history has been marked by political friction between Epsilon Eridani and "
"the Earth government. This began with the advent of hyperspace around the end of the 26th "
"century. While previously the communications lag of 20 years had prevented exertion of "
"Earth's power, suddenly the rulers of Epsilon Eridani found themselves constantly subject "
"to the interference of Earth.\n"
"This conflict flared up in 2714 when the pro-Earth president of Epsilon Eridani was toppled "
"amid strikes and civil disorder over the unfair tax and trade conditions imposed by "
"Earth. The 'Free Republic' then established survived nine months until Earth rule "
"was re-imposed by force, including the notorious use of orbital lasers on population centres.\n"
"Independence was not finally won until the wars of the 30th century, and the formation "
"of the Confederation of Independent Worlds, of which Epsilon Eridani was a founding member.\n"
"Epsilon Eridani is today a thriving centre of industry, cutting-edge technology and "
"tourism.\n"
"Reproduced with the kind permission of Enrique Watson, New Hope University, 2992"
}, // Components: K2
{ "UV Ceti", 0, {SBody::TYPE_STAR_M, SBody::TYPE_STAR_M}, 0, 0, vector3f(0.754,0.473,-1.040) }, // Components: M5, M5
{ "GX Andromedae", 0, {SBody::TYPE_STAR_M, SBody::TYPE_STAR_M}, 1, -1, vector3f(0.269,0.470,0.580) }, // Components: M2, M6
{ "Stein 2051", 0, {SBody::TYPE_STAR_M, SBody::TYPE_WHITE_DWARF}, 2, -1, vector3f(0.383,0.316,0.278) }, // Components: M4, DC5
{ "AC+33:25644", 0, {SBody::TYPE_STAR_M}, 2, 0, vector3f(0.770,0.607,0.598) }, // Components: M4
{ "Ross 248", 0, {SBody::TYPE_STAR_M}, 0, -1, vector3f(0.914,0.335,-0.374) }, // Components: M6
{ "L 205-128 Sm 3", 0, {SBody::TYPE_STAR_M}, -2, 1, vector3f(0.332,0.504,-0.605) }, // Components: M3
{ "Alpha Centauri", 0, {SBody::TYPE_STAR_G, SBody::TYPE_STAR_K}, 0, 0, vector3f(0.112,0.882,-0.006) }, // Components: G2, K0
{ "Lalande 21185", 0, {SBody::TYPE_STAR_M}, 0, 0, vector3f(0.933,0.539,0.930) }, // Components: M2
{ "YZ Ceti", 0, {SBody::TYPE_STAR_M}, 0, 1, vector3f(0.903,0.723,-0.826) }, // Components: M5

	{ 0 }
};
