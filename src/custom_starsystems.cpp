#include "libs.h"
#include "custom_starsystems.h"
#include "StarSystem.h"

#define FIXED_DEG2RAD(n,d) (fixed(n,d) * fixed(31416, 1800000))

const CustomSBody sol_system[] = {
	{ "Sol", SBody::TYPE_STAR_G,
	  -1, fixed(1,1), fixed(1,1), 5700 },
	{ "Mercury", SBody::TYPE_PLANET_TERRESTRIAL,
	  0, fixed(38,100), fixed(55,1000), 340,
	  fixed(387,1000), fixed(205,1000), { (float)DEG2RAD(7.0) }, fixed(59,1),
       	  FIXED_DEG2RAD(1,100)	},
	{ "Venus", SBody::TYPE_PLANET_TERRESTRIAL,
	  0, fixed(95,100), fixed(815,1000), 735,
	  fixed(723,1000), fixed(7,1000), { (float)DEG2RAD(3.39) }, fixed(243,1),
          FIXED_DEG2RAD(26,10)	},
	{ "Earth", SBody::TYPE_PLANET_TERRESTRIAL,
	  0, fixed(1,1), fixed(1,1), 288,
	  fixed(1,1), fixed(167,10000), {}, fixed(1,1), FIXED_DEG2RAD(2344,100), {}, ECON_INDUSTRY,
	  "data/earth.hmap"
	},
	{ "Shanghai", SBody::TYPE_STARPORT_SURFACE,
	  3, 0, 0, 0, 0, 0, { (float)DEG2RAD(31), (float)DEG2RAD(-121) } },
	{ "Mexico City", SBody::TYPE_STARPORT_SURFACE,
	  3, 0, 0, 0, 0, 0, { (float)DEG2RAD(19), (float)DEG2RAD(99) } },
	{ "London", SBody::TYPE_STARPORT_SURFACE,
	  3, 0, 0, 0, 0, 0, { (float)DEG2RAD(51), 0 } },
	{ "Moscow", SBody::TYPE_STARPORT_SURFACE,
	  3, 0, 0, 0, 0, 0, { (float)DEG2RAD(55), (float)DEG2RAD(-37.5) } },
	{ "Brasilia", SBody::TYPE_STARPORT_SURFACE,
	  3, 0, 0, 0, 0, 0, { (float)DEG2RAD(-15.5), (float)DEG2RAD(48.0) } },
	{ "Los Angeles", SBody::TYPE_STARPORT_SURFACE,
	  3, 0, 0, 0, 0, 0, { (float)DEG2RAD(34), (float)DEG2RAD(118) } },
	{ "Gates Spaceport", SBody::TYPE_STARPORT_ORBITAL,
	  3, 0, 0, 0, fixed(100,100000), 0, { (float)DEG2RAD(0), (float)DEG2RAD(0) },
	  fixed(1,24*60*3) },
	{ "Moon", SBody::TYPE_PLANET_TERRESTRIAL,
	  3, fixed(273,1000), fixed(12,1000), 220,
	  fixed(257,100000), fixed(549,10000), { (float)DEG2RAD(5.145) }, fixed(273,10),
          FIXED_DEG2RAD(668,100) },
	{ "Lunar City", SBody::TYPE_STARPORT_SURFACE,
	  11, 0, 0, 0, 0, 0, { (float)DEG2RAD(19), (float)DEG2RAD(99) } },
	{ "Mars", SBody::TYPE_PLANET_TERRESTRIAL,
	  0, fixed(533,1000), fixed(107,1000), 274,
	  fixed(152,100), fixed(933,10000), { (float)DEG2RAD(1.85) }, fixed(1027,1000), 
	  FIXED_DEG2RAD(2519,100), {}, ECON_INDUSTRY, },
	{ "Cydonia", SBody::TYPE_STARPORT_SURFACE,
	  13, 0, 0, 0, 0, 0, { (float)DEG2RAD(-29), (float)DEG2RAD(124) } },
	{ "Olympus Mons", SBody::TYPE_STARPORT_SURFACE,
	  13, 0, 0, 0, 0, 0, { (float)DEG2RAD(30), (float)DEG2RAD(-37) } },
	{ "Mars High", SBody::TYPE_STARPORT_ORBITAL,
	  13, 0, 0, 0, fixed(5068,100000000), 0, { (float)DEG2RAD(0), (float)DEG2RAD(0) },
	  fixed(11,24) },
	{ "Phobos", SBody::TYPE_PLANET_ASTEROID,
	  13, fixed(21,10000), fixed(18,10000000000LL), 233,
	  fixed(6268,100000000), fixed(151,10000), { (float)DEG2RAD(1.093) }, fixed(11,24) },
	{ "Deimos", SBody::TYPE_PLANET_ASTEROID,
	  13, fixed(12,10000), fixed(25,100000000000LL), 233,
	  fixed(1568,10000000), fixed(2,10000), { (float)DEG2RAD(0.93) }, fixed(30,24) },
	{ "Phobos Base", SBody::TYPE_STARPORT_SURFACE,
	  17, 0, 0, 0, 0, 0, { (float)DEG2RAD(5), (float)DEG2RAD(-5) } },
	{ "Tomm's Sanctuary", SBody::TYPE_STARPORT_SURFACE,
	  18, 0, 0, 0, 0, 0, { (float)DEG2RAD(0), (float)DEG2RAD(0) } },
	{ "Jupiter", SBody::TYPE_PLANET_GAS_GIANT,
	  0, fixed(11,1), fixed(3178,10), 165,
	  fixed(5204,1000), fixed(488,10000), { (float)DEG2RAD(1.305) }, fixed(4,10),
          FIXED_DEG2RAD(313,100) },
	{ "Saturn", SBody::TYPE_PLANET_GAS_GIANT,
	  0, fixed(9,1), fixed(95152,1000), 134,
	  fixed(9582,1000), fixed(557,10000), { (float)DEG2RAD(2.485) }, fixed(4,10),
          FIXED_DEG2RAD(2673,100) },
	{ "Uranus", SBody::TYPE_PLANET_GAS_GIANT,
	  0, fixed(4,1), fixed(145,10), 76,
	  fixed(19229,1000), fixed(444,10000), { (float)DEG2RAD(0.772) }, fixed(7,10),
          FIXED_DEG2RAD(9777,100) },
	{ "Neptune", SBody::TYPE_PLANET_GAS_GIANT,
	  0, fixed(38,10), fixed(17147,100), 72,
	  fixed(30104,1000), fixed(112,10000), { (float)DEG2RAD(1.768) }, fixed(75,100),
          FIXED_DEG2RAD(2832,100) },
	// moons of jupiter
	{ "Io", SBody::TYPE_PLANET_TERRESTRIAL,
	  21, fixed(286,1000), fixed(15,1000), 130,
	  fixed(282,100000), fixed(41,10000), { (float)DEG2RAD(2.21) }, fixed(177,100) },
	{ "Europa", SBody::TYPE_PLANET_TERRESTRIAL,
	  21, fixed(245,1000), fixed(8,1000), 102,
	  fixed(441,100000), fixed(9,1000), {}, fixed(355,100) },
	{ "Clarke's Station", SBody::TYPE_STARPORT_ORBITAL,
	  26, 0, 0, 0, fixed(12,500000), 0, { (float)DEG2RAD(0), (float)DEG2RAD(0) },
	  fixed(1,24*60*3) },
	{ "Ganymede", SBody::TYPE_PLANET_TERRESTRIAL,
	  21, fixed(413,1000), fixed(25,1000), 110,
	  fixed(72,10000), fixed(13,10000), { (float)DEG2RAD(0.2) }, fixed(72,10) },
	{ "Callisto", SBody::TYPE_PLANET_TERRESTRIAL,
	  21, fixed(378,1000), fixed(18,1000), 134,
	  fixed(126,10000), fixed(74,10000), { (float)DEG2RAD(0.192) }, fixed(167,10) },
	// moons of saturn
	{ "Titan", SBody::TYPE_PLANET_TERRESTRIAL,
	  //22, fixed(400,1000), fixed(225,10000), 94, 
	  //fixed(82,10000), fixed(288,10000), { (float)DEG2RAD(0.34854) }, fixed(15945,1000) },
	  22, fixed(400,1000), fixed(225,10000), 94,  //Oddness/ When correct Radius setting is used (0.4ER), planet has no atmosphere or gravity !?!?!
	  fixed(82,10000), fixed(288,10000), { (float)DEG2RAD(0.34854) }, fixed(15945,1000) },
	{ "Oasis City", SBody::TYPE_STARPORT_SURFACE,
	  30, 0, 0, 0, 0, 0, { (float)DEG2RAD(18.4), (float)DEG2RAD(196) } },
	{ "Port Makenzie", SBody::TYPE_STARPORT_SURFACE,
	  30, 0, 0, 0, 0, 0, { (float)DEG2RAD(1), (float)DEG2RAD(14) } },
	{ "Rhea", SBody::TYPE_PLANET_TERRESTRIAL,
	  22, fixed(12,100), fixed(39,100000), 81,
	  fixed(441,100000), fixed(126,100000), { (float)DEG2RAD(0.345) }, fixed(452,100) },
	{ "Iapetus", SBody::TYPE_PLANET_TERRESTRIAL,
	  22, fixed(1155,10000), fixed(3,10000), 115,
	  fixed(238,10000), fixed(29,1000), { (float)DEG2RAD(15.47) }, fixed(7932,100) },
	{ "Dione", SBody::TYPE_PLANET_TERRESTRIAL,
	  22, fixed(881,10000), fixed(328,1000000), 87,
	  fixed(252,100000), fixed(22,10000), { (float)DEG2RAD(0.019) }, fixed(2737,1000) },
	// moons up uranus
	{ "Titania", SBody::TYPE_PLANET_TERRESTRIAL,
	  23, fixed(1235,10000), fixed(5908,10000000), 70,
	  fixed(2913,1000000), fixed(11,10000), { (float)DEG2RAD(0.34) }, fixed(87,10) },
	{ "Oberon", SBody::TYPE_PLANET_TERRESTRIAL,
	  23, fixed(1194,10000), fixed(5046,10000000), 75,
	  fixed(39,10000), fixed(14,10000), { (float)DEG2RAD(0.058) }, fixed(135,10) },
	{ "Umbriel", SBody::TYPE_PLANET_TERRESTRIAL,
	  23, fixed(92,1000), fixed(2,10000), 75,
	  fixed(178,100000), fixed(39,10000), { (float)DEG2RAD(0.128) }, fixed(4144,1000) },
	{ "Ariel", SBody::TYPE_PLANET_TERRESTRIAL,
	  23, fixed(908,10000), fixed(226,1000000), 60,
	  fixed(1277,1000000), fixed(12,10000), { (float)DEG2RAD(0.26) }, fixed(252,100) },
	// moons of neptune
	{ "Triton", SBody::TYPE_PLANET_TERRESTRIAL,
	  24, fixed(2122,10000), fixed(359,100000), 38,
	  fixed(2371,100000), fixed(16,1000000), { (float)DEG2RAD(156.885) }, fixed(141,24) },
	{ "Nereid", SBody::TYPE_PLANET_ASTEROID,
	  24, fixed(267,10000), fixed(519,100000000), 50,
	  fixed(3685,100000), fixed(75,100), { (float)DEG2RAD(32.55) }, fixed(115,240) },
	{ "Proteus", SBody::TYPE_PLANET_ASTEROID,
	  24, fixed(310,10000), fixed(710,100000000), 51,
	  fixed(786,1000000), fixed(53,100000), { (float)DEG2RAD(0.524) }, fixed(1122,1000) },
	// you know you want to call pluto a planet, you really do
	{ "Pluto", SBody::TYPE_PLANET_TERRESTRIAL,
	  0, fixed(18,100), fixed(21,10000), 44,
	  fixed(394,10), fixed(249,1000), { (float)DEG2RAD(11.88) }, fixed(153,24),
          FIXED_DEG2RAD(296,10)	},
	// Extra bases down here, save re-ordering the primary.
	{ "Enki Catena", SBody::TYPE_STARPORT_SURFACE,
	  28, 0, 0, 0, 0, 0, { (float)DEG2RAD(84), (float)DEG2RAD(96) } },
	{ "Pluto Research Base", SBody::TYPE_STARPORT_SURFACE,
	  43, 0, 0, 0, 0, 0, { (float)DEG2RAD(84), (float)DEG2RAD(96) } },
	{ "Poseidon Station", SBody::TYPE_STARPORT_ORBITAL,
	  40, 0, 0, 0, fixed(12,500000), 0, { (float)DEG2RAD(0), (float)DEG2RAD(0) },
	  fixed(11,7) },
	//{ "Hotel Sol", SBody::TYPE_STARPORT_ORBITAL,
	 // 0, 0, 0, 0, fixed(10,0), fixed(500,1000), { (float)DEG2RAD(0), (float)DEG2RAD(0) },
	 // fixed(11,24) },
	{ "Daniel's Haven", SBody::TYPE_STARPORT_ORBITAL,
	  30, 0, 0, 0, fixed(12,500000), fixed(50,1000), { (float)DEG2RAD(0), (float)DEG2RAD(0) },
	  fixed(11,9) },
	{ "Discovery Base", SBody::TYPE_STARPORT_ORBITAL,
	  21, 0, 0, 0, fixed(7,1000), 0, { (float)DEG2RAD(0), (float)DEG2RAD(0) },
	  fixed(11,1) },
	{ "Dante's Base", SBody::TYPE_STARPORT_SURFACE,
	  25, 0, 0, 0, 0, 0, { (float)DEG2RAD(-0.5), (float)DEG2RAD(26.2) } },  //49
	  
	{ 0 }
};

const CustomSystem custom_systems[] = {
	{ "Sol", sol_system, {SBody::TYPE_STAR_G}, 0, 0, vector3f(.5f, .5f, 0), 0,
       "The historical birthplace of humankind",
	"Sol is a fine joint"	},
{ "Alioth", 0, {SBody::TYPE_STAR_K}, 0, 4, vector3f(0.629f,0.880f,1.045f), 5/*5=2 earths*/,
"Liberal democracy",
"Alioth comes from the ancient Arabic word alyat - fat tail of sheep.\n"
"The two massive ark ships bound for Alioth were the first of many such"
" sub-light vessels designed specifically for the kind of lengthy journeys"
" required to spread to the stars.\n"
"The twin ships departed on the year 2354 to commemorate the journey to Epsilon Eridani."
"  The journey was uneventful and was completed in a little over 127 years, owing to the fact that"
" propulsion technology was far geater than it had been in the past.\n"
"The colonists had a hard life, high gravity slowed down any progress that should have been made."
"  Communication with Earth was non-existent, politics and the 80 year time-lag made it impossible."
"  By the late 26th Century Alioth had progressed a long way and any ties to Earth had long been lost."
"  However with the advent of Hyper-Space Technology in the late 26th Century,"
" Earth began to show an interest in Alioth once more and"
" Alioth was no longer a self-governed world.\n"
"The system remained under Earth control until the great wars of the 30th Century"
" and the formation of the Confederation of Independent Worlds.\n"
"Since then Alioth has become one of the most important worlds in the CIS.",
Polit::GOV_CISLIBDEM
}, // Components: K2
{ "Epsilon Indi", 0, {SBody::TYPE_STAR_K}, -1, 0, vector3f(0.629f,0.880f,-1.045f) }, // Components: K5
{ "Luyten 1159-016", 0, {SBody::TYPE_STAR_M}, 2, 0, vector3f(0.316f,0.651f,0.008f) }, // Components: M8
{ "Wolf 424", 0, {SBody::TYPE_STAR_M, SBody::TYPE_STAR_M}, 0, -2, vector3f(0.189f,0.951f,0.771f) }, // Components: M5, M7
{ "DX Cancri", 0, {SBody::TYPE_STAR_M}, 1, 0, vector3f(0.701f,0.862f,0.784f) }, // Components: M6
{ "LTT 17897", 0, {SBody::TYPE_STAR_M, SBody::TYPE_STAR_K, SBody::TYPE_STAR_M, SBody::TYPE_STAR_M}, 2, 1, vector3f(0.460f,0.384f,-0.402f) }, // Components: M4, K, M, M6
{ "EV Lacertae", 0, {SBody::TYPE_STAR_M}, 0, -2, vector3f(0.864f,0.870f,1.249f) }, // Components: M4
{ "Luyten 354-89", 0, {SBody::TYPE_STAR_M}, -1, 0, vector3f(0.204f,0.746f,-1.365f) }, // Components: M1
{ "Gliese   1", 0, {SBody::TYPE_STAR_M}, -1, 1, vector3f(0.657f,0.679f,-1.203f) }, // Components: M4
{ "Gliese 570", 0, {SBody::TYPE_STAR_K, SBody::TYPE_STAR_M}, -2, 0, vector3f(0.174f,0.358f,-0.281f) }, // Components: K5, M2
{ "Kruger 60", 0, {SBody::TYPE_STAR_M, SBody::TYPE_STAR_M}, 0, -2, vector3f(0.903f,0.932f,0.004f) }, // Components: M2, M6
{ "Gliese 205", 0, {SBody::TYPE_STAR_M}, 2, 1, vector3f(0.488f,0.496f,-0.805f) }, // Components: M1
{ "Proxima", 0, {SBody::TYPE_STAR_M}, 0, 0, vector3f(0.135f,0.882f,-0.017f), 17,
"Military dictatorship",
"Closest star in the sky to early human civilisation.\n"
"Home of the famous wright Shipyards."
"Also home to a Federation Naval Base on the 3rd world, which houses the largest"
"known Fleet dock in the Galaxy.",
Polit::GOV_EARTHMILDICT
}, // Components: M5
{ "Gliese 825", 0, {SBody::TYPE_STAR_M}, -1, 0, vector3f(0.365f,0.424f,-1.093f) }, // Components: M0
{ "Gliese 876", 0, {SBody::TYPE_STAR_M}, -2, 0, vector3f(0.603f,0.900f,-0.112f) }, // Components: M5
{ "Gliese 725", 0, {SBody::TYPE_STAR_M, SBody::TYPE_STAR_M}, 0, -1, vector3f(0.482f,0.203f,0.590f), 4,
"Earth Federation Democracy",
"Gliese 725 is the fifteenth star system closest to the Sun ."
"It is located within 12 light years , in the south - west of the constellation of the Dragon, "
"in north -west of the star ? Draconis , south- east of d Draconis .",
Polit::GOV_EARTHDEMOC
 }, // Components: M4, M5
{ "Sigma Draconis", 0, {SBody::TYPE_STAR_K}, 0, -2, vector3f(0.914f,0.420f,0.859f), 2,
"Social democracy",
"Sigma Draconis (s Dra, s Draconis) is a star 18.8 light-years away from Earth. "
"Its traditional name is Alsafi. It is in the constellation Draco. Its visual magnitude is 4.68. "
"The traditional name 'Alsafi' (also Athafi) supposedly derives from an Arabic word al-athafi 'the cooking tripods'. "
"This refers to the tripods used by nomads for open-air cooking.\n"
"The star is a main sequence dwarf of spectral type K0. ",
Polit::GOV_SOCDEM

}, // Components: K0
{ "Gliese 380", 0, {SBody::TYPE_STAR_K}, 1, -1, vector3f(0.979f,0.462f,0.631f) }, // Components: K2
{ "Sirius", 0, {SBody::TYPE_STAR_A, SBody::TYPE_WHITE_DWARF}, 1, 1, vector3f(0.222f,0.273f,-0.173f), 5,
"Corporate system",
"The Sirius system is home to Sirius Corporation, market leader in Robotics, Neural Computing, "
"Security and Defence Systems, to name but a few of its endeavours. Sirius research and development "
"institutes are at the very cutting edge of galactic science. The young, bright and ambitious from "
"worlds all over galaxy flock to Sirius to make a name for themselves.\n"
"Above text all rights reserved Sirius Corporation.",
Polit::GOV_CORPORATE
}, // Components: A1, DA2
{ "Gliese 682", 0, {SBody::TYPE_STAR_M}, -2, 0, vector3f(0.643f,0.971f,-0.213f) }, // Components: M3
{ "AD Leonis", 0, {SBody::TYPE_STAR_M}, 2, 0, vector3f(0.483f,0.012f,-0.071f) }, // Components: M4
{ "Luyten 372-58", 0, {SBody::TYPE_STAR_M}, 0, 1, vector3f(0.822f,0.495f,-1.402f) }, // Components: M4
{ "Gliese 191", 0, {SBody::TYPE_STAR_M}, 0, 1, vector3f(0.925f,0.696f,-0.938f) }, // Components: M0
{ "Gliese 663", 0, {SBody::TYPE_STAR_K}, -2, 0, vector3f(0.279f,0.572f,0.278f) }, // Components: K5
{ "70 Ophiuchi", 0, {SBody::TYPE_STAR_K, SBody::TYPE_STAR_K}, -2, -1, vector3f(0.758f,0.509f,0.420f) }, // Components: K0, K5
{ "Ross 614", 0, {SBody::TYPE_STAR_M}, 1, 1, vector3f(0.908f,0.402f,-0.195f) }, // Components: M4
{ "GJ 1245", 0, {SBody::TYPE_STAR_M, SBody::TYPE_STAR_M}, 0, -2, vector3f(0.126f,0.637f,0.295f) }, // Components: M5, M
{ "Gliese 526", 0, {SBody::TYPE_STAR_M}, -1, -2, vector3f(0.418f,0.839f,0.993f) }, // Components: M4
{ "Ross 986", 0, {SBody::TYPE_STAR_M}, 2, 0, vector3f(0.835f,0.449f,0.829f) }, // Components: M4
{ "ADS 10417", 0, {SBody::TYPE_STAR_K, SBody::TYPE_STAR_K}, -2, 0, vector3f(0.346f,0.576f,0.274f) }, // Components: K1, K1
{ "GJ 1002", 0, {SBody::TYPE_STAR_M}, -1, 2, vector3f(0.778f,0.260f,-0.521f) }, // Components: M5
{ "Ross 154", 0, {SBody::TYPE_STAR_M}, -1, 0, vector3f(0.344f,0.274f,-0.205f) }, // Components: M4
{ "L 674-015", 0, {SBody::TYPE_STAR_M}, 1, 2, vector3f(0.885f,0.442f,-0.508f) }, // Components: M4
{ "Wolf 359", 0, {SBody::TYPE_STAR_M}, 0, 0, vector3f(0.746f,0.990f,0.806f) }, // Components: M6
{ "AC+79:3888", 0, {SBody::TYPE_STAR_M}, 1, -1, vector3f(0.511f,0.157f,1.308f) }, // Components: M4
{ "Luyten 789-006", 0, {SBody::TYPE_STAR_M}, -1, 0, vector3f(0.118f,0.781f,0.059f) }, // Components: M5
{ "Gliese 674", 0, {SBody::TYPE_STAR_M}, -2, 1, vector3f(0.739f,0.046f,-0.209f) }, // Components: M3
{ "Procyon", 0, {SBody::TYPE_STAR_F, SBody::TYPE_WHITE_DWARF}, 1, 1, vector3f(0.663f,0.267f,0.311f) }, // Components: F5, DA
{ "Van Maanen's Star", 0, {SBody::TYPE_WHITE_DWARF}, 2, 0, vector3f(0.279f,0.482f,-0.330f) }, // Components: DZ7
{ "Delta Pavonis", 0, {SBody::TYPE_STAR_G}, -2, 1, vector3f(0.796f,0.492f,-1.240f) }, // Components: G8
{ "Gliese 752", 0, {SBody::TYPE_STAR_M, SBody::TYPE_STAR_M}, -2, -1, vector3f(0.739f,0.013f,-0.114f) }, // Components: M3, M5
{ "Gliese 687", 0, {SBody::TYPE_STAR_M}, 0, -2, vector3f(0.744f,0.895f,1.020f) }, // Components: M3
{ "40 Eridani", 0, {SBody::TYPE_STAR_K, SBody::TYPE_WHITE_DWARF, SBody::TYPE_STAR_M}, 1, 1, vector3f(0.943f,0.037f,-1.224f) }, // Components: K1, DA4, M4
{ "AC+12:1800-213", 0, {SBody::TYPE_STAR_M}, 2, 1, vector3f(0.848f,0.058f,-0.410f) }, // Components: M4
{ "Eta Cassiopeia", 0, {SBody::TYPE_STAR_G, SBody::TYPE_STAR_K}, 1, -2, vector3f(0.913f,0.976f,1.272f) }, // Components: G3, K7
{ "Hei 299", 0, {SBody::TYPE_STAR_M}, -1, 2, vector3f(0.813f,0.384f,-0.843f) }, // Components: M4
{ "Ross 128", 0, {SBody::TYPE_STAR_M}, 1, 0, vector3f(0.838f,0.638f,-0.344f) }, // Components: M4
{ "Gliese 628", 0, {SBody::TYPE_STAR_M}, -2, 0, vector3f(0.982f,0.421f,0.680f) }, // Components: M3
{ "Gliese 229", 0, {SBody::TYPE_STAR_M}, 1, 2, vector3f(0.964f,0.149f,-0.754f) }, // Components: M1
{ "Luyten 145-141", 0, {SBody::TYPE_WHITE_DWARF}, -1, 2, vector3f(0.691f,0.179f,-0.095f) }, // Components: DQ6
{ "EI Cancri", 0, {SBody::TYPE_STAR_M, SBody::TYPE_STAR_M}, 2, 0, vector3f(0.621f,0.679f,0.148f) }, // Components: M, M
{ "Altair", 0, {SBody::TYPE_STAR_A}, -1, -1, vector3f(0.142f,0.022f,-0.299f) }, // Components: A7
{ "Gliese 887", 0, {SBody::TYPE_STAR_M}, -1, 1, vector3f(0.413f,0.547f,-0.115f) }, // Components: M2
{ "Gliese 908", 0, {SBody::TYPE_STAR_M}, -1, -2, vector3f(0.406f,0.751f,0.998f) }, // Components: M2
{ "Gliese 588", 0, {SBody::TYPE_STAR_M}, -2, 0, vector3f(0.185f,0.968f,-0.790f) }, // Components: M3
{ "Tau Ceti", 0, {SBody::TYPE_STAR_G}, 1, 1, vector3f(0.127f,0.778f,-0.324f), 2 }, // Components: G8
{ "LTT 12976", 0, {SBody::TYPE_STAR_M, SBody::TYPE_STAR_M}, 1, -1, vector3f(0.986f,0.133f,0.773f) }, // Components: M2, M6
{ "Sm 50", 0, {SBody::TYPE_STAR_M}, -2, 0, vector3f(0.411f,0.693f,-1.341f) }, // Components: M0
{ "61 Cygni", 0, {SBody::TYPE_STAR_K, SBody::TYPE_STAR_K}, 0, -1, vector3f(0.302f,0.108f,-0.136f) }, // Components: K5, K7
{ "L 347-014", 0, {SBody::TYPE_STAR_M}, -2, 0, vector3f(0.390f,0.787f,-0.927f) }, // Components: M4
{ "Wolf 629", 0, {SBody::TYPE_STAR_M}, -2, 0, vector3f(0.334f,0.090f,0.872f) }, // Components: M4
{ "Luyten's Star", 0, {SBody::TYPE_STAR_M}, 1, 1, vector3f(0.788f,0.305f,0.266f) }, // Components: M3
{ "Gliese 783", 0, {SBody::TYPE_STAR_K, SBody::TYPE_STAR_M}, -2, 0, vector3f(0.522f,0.324f,-1.169f) }, // Components: K3, M3
{ "Barnard's star", 0, {SBody::TYPE_STAR_M}, -1, 0, vector3f(0.877f,0.131f,0.186f), 0 ,
"Earth Federation Colonial Rule",
"Federation Prison Camps.",
Polit::GOV_EARTHCOLONIAL
}, // Components: M5
{ "Epsilon Eridani", 0, {SBody::TYPE_STAR_K}, 1, 0, vector3f(0.351f,0.735f,-0.999f), 5/*5=2 earths*/,
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
"Reproduced with the kind permission of Enrique Watson, New Hope University, 2992",
Polit::GOV_CISLIBDEM
}, // Components: K2
{ "UV Ceti", 0, {SBody::TYPE_STAR_M, SBody::TYPE_STAR_M}, 0, 0, vector3f(0.754f,0.473f,-1.040f) }, // Components: M5, M5
{ "GX Andromedae", 0, {SBody::TYPE_STAR_M, SBody::TYPE_STAR_M}, 1, -1, vector3f(0.269f,0.470f,0.580f) }, // Components: M2, M6
{ "Stein 2051", 0, {SBody::TYPE_STAR_M, SBody::TYPE_WHITE_DWARF}, 2, -1, vector3f(0.383f,0.316f,0.278f) }, // Components: M4, DC5
{ "AC+33:25644", 0, {SBody::TYPE_STAR_M}, 2, 0, vector3f(0.770f,0.607f,0.598f) }, // Components: M4
{ "Ross 248", 0, {SBody::TYPE_STAR_M}, 0, -1, vector3f(0.914f,0.335f,-0.374f) }, // Components: M6
{ "L 205-128 Sm 3", 0, {SBody::TYPE_STAR_M}, -2, 1, vector3f(0.332f,0.504f,-0.605f) }, // Components: M3
{ "Alpha Centauri", 0, {SBody::TYPE_STAR_G, SBody::TYPE_STAR_K}, 0, 0, vector3f(0.112f,0.882f,-0.006f) }, // Components: G2, K0
{ "Lalande 21185", 0, {SBody::TYPE_STAR_M}, 0, 0, vector3f(0.933f,0.539f,0.930f) }, // Components: M2
{ "YZ Ceti", 0, {SBody::TYPE_STAR_M}, 0, 1, vector3f(0.903f,0.723f,-0.826f) }, // Components: M5

	{ 0 }
};
