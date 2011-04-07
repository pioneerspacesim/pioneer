#include "mylua.h"
#include "PiLuaConstants.h"
#include "StarSystem.h"
#include "Polit.h"
#include "EquipType.h"

#define _setfield(v,k) do { lua_pushinteger(L, v); lua_setfield(L, -2, k); } while(0)

void PiLuaConstants::RegisterConstants(lua_State *L)
{
	lua_newtable(L); // Body

	lua_newtable(L); // Type
	_setfield(SBody::TYPE_GRAVPOINT,          "GRAVPOINT");
	_setfield(SBody::TYPE_BROWN_DWARF,        "BROWN_DWARF");
	_setfield(SBody::TYPE_STAR_M,             "STAR_M");
	_setfield(SBody::TYPE_STAR_K,             "STAR_K");
	_setfield(SBody::TYPE_STAR_G,             "STAR_G");
	_setfield(SBody::TYPE_STAR_F,             "STAR_F");
	_setfield(SBody::TYPE_STAR_A,             "STAR_A");
	_setfield(SBody::TYPE_STAR_B,             "STAR_B");
	_setfield(SBody::TYPE_STAR_O,             "STAR_O");
	_setfield(SBody::TYPE_STAR_M_GIANT,       "STAR_M_GIANT");
	_setfield(SBody::TYPE_STAR_K_GIANT,       "STAR_K_GIANT");
	_setfield(SBody::TYPE_STAR_G_GIANT,       "STAR_G_GIANT");
	_setfield(SBody::TYPE_STAR_F_GIANT,       "STAR_F_GIANT");
	_setfield(SBody::TYPE_STAR_A_GIANT,       "STAR_A_GIANT");
	_setfield(SBody::TYPE_STAR_B_GIANT,       "STAR_B_GIANT");
	_setfield(SBody::TYPE_STAR_O_GIANT,       "STAR_O_GIANT");
	_setfield(SBody::TYPE_STAR_M_SUPER_GIANT, "STAR_M_SUPER_GIANT");
	_setfield(SBody::TYPE_STAR_K_SUPER_GIANT, "STAR_K_SUPER_GIANT");
	_setfield(SBody::TYPE_STAR_G_SUPER_GIANT, "STAR_G_SUPER_GIANT");
	_setfield(SBody::TYPE_STAR_F_SUPER_GIANT, "STAR_F_SUPER_GIANT");
	_setfield(SBody::TYPE_STAR_A_SUPER_GIANT, "STAR_A_SUPER_GIANT");
	_setfield(SBody::TYPE_STAR_B_SUPER_GIANT, "STAR_B_SUPER_GIANT");
	_setfield(SBody::TYPE_STAR_O_SUPER_GIANT, "STAR_O_SUPER_GIANT");
	_setfield(SBody::TYPE_STAR_M_HYPER_GIANT, "STAR_M_HYPER_GIANT");
	_setfield(SBody::TYPE_STAR_K_HYPER_GIANT, "STAR_K_HYPER_GIANT");
	_setfield(SBody::TYPE_STAR_G_HYPER_GIANT, "STAR_G_HYPER_GIANT");
	_setfield(SBody::TYPE_STAR_F_HYPER_GIANT, "STAR_F_HYPER_GIANT");
	_setfield(SBody::TYPE_STAR_A_HYPER_GIANT, "STAR_A_HYPER_GIANT");
	_setfield(SBody::TYPE_STAR_B_HYPER_GIANT, "STAR_B_HYPER_GIANT");
	_setfield(SBody::TYPE_STAR_O_HYPER_GIANT, "STAR_O_HYPER_GIANT");
	_setfield(SBody::TYPE_STAR_M_WF,		  "STAR_M_WF");
	_setfield(SBody::TYPE_STAR_B_WF,		  "STAR_B_WF");
	_setfield(SBody::TYPE_STAR_O_WF,		  "STAR_O_WF");
	_setfield(SBody::TYPE_STAR_S_BH,		  "STAR_S_BH");
	_setfield(SBody::TYPE_STAR_IM_BH,		  "STAR_IM_BH");
	_setfield(SBody::TYPE_STAR_SM_BH,		  "STAR_SM_BH");
	_setfield(SBody::TYPE_WHITE_DWARF,        "WHITE_DWARF");
	_setfield(SBody::TYPE_PLANET_GAS_GIANT,   "PLANET_GAS_GIANT");
	_setfield(SBody::TYPE_PLANET_ASTEROID,    "PLANET_ASTEROID");
	_setfield(SBody::TYPE_PLANET_TERRESTRIAL, "PLANET_TERRESTRIAL");
	_setfield(SBody::TYPE_STARPORT_ORBITAL,   "STARPORT_ORBITAL");
	_setfield(SBody::TYPE_STARPORT_SURFACE,   "STARPORT_SURFACE");
	lua_setfield(L, -2, "Type");

	lua_newtable(L); // SuperType
	_setfield(SBody::SUPERTYPE_NONE,         "NONE");
	_setfield(SBody::SUPERTYPE_STAR,         "STAR");
	_setfield(SBody::SUPERTYPE_ROCKY_PLANET, "ROCKY_PLANET");
	_setfield(SBody::SUPERTYPE_GAS_GIANT,    "GAS_GIANT");
	_setfield(SBody::SUPERTYPE_STARPORT,     "STARPORT");
	lua_setfield(L, -2, "SuperType");

	lua_setfield(L, LUA_GLOBALSINDEX, "Body");


	lua_newtable(L); // Polit

	lua_newtable(L); // Crime
	_setfield(Polit::CRIME_TRADING_ILLEGAL_GOODS, "TRADING_ILLEGAL_GOODS");
	_setfield(Polit::CRIME_WEAPON_DISCHARGE,      "WEAPON_DISCHARGE");
	_setfield(Polit::CRIME_PIRACY,                "PIRACY");
	_setfield(Polit::CRIME_MURDER,                "MURDER");
	lua_setfield(L, -2, "Crime");

	lua_newtable(L); // Bloc
	_setfield(Polit::BLOC_NONE,     "NONE");
	_setfield(Polit::BLOC_EARTHFED, "EARTHFED");
	_setfield(Polit::BLOC_CIS,      "CIS");
	_setfield(Polit::BLOC_EMPIRE,   "EMPIRE");
	lua_setfield(L, -2, "Bloc");

	lua_newtable(L); // Econ
	_setfield(Polit::ECON_NONE,            "NONE");
	_setfield(Polit::ECON_VERY_CAPITALIST, "VERY_CAPITALIST");
	_setfield(Polit::ECON_CAPITALIST,      "CAPITALIST");
	_setfield(Polit::ECON_MIXED,           "MIXED");
	_setfield(Polit::ECON_PLANNED,         "PLANNED");
	lua_setfield(L, -2, "Econ");

	lua_newtable(L); // Gov
	_setfield(Polit::GOV_INVALID,       "INVALID");
	_setfield(Polit::GOV_NONE,          "NONE");
	_setfield(Polit::GOV_EARTHCOLONIAL, "EARTHCOLONIAL");
	_setfield(Polit::GOV_EARTHDEMOC,    "EARTHDEMOC");
	_setfield(Polit::GOV_EMPIRERULE,    "EMPIRERULE");
	_setfield(Polit::GOV_CISLIBDEM,     "CISLIBDEM");
	_setfield(Polit::GOV_CISSOCDEM,     "CISSOCDEM");
	_setfield(Polit::GOV_LIBDEM,        "LIBDEM");
	_setfield(Polit::GOV_CORPORATE,     "CORPORATE");
	_setfield(Polit::GOV_SOCDEM,        "SOCDEM");
	_setfield(Polit::GOV_EARTHMILDICT,  "EARTHMILDICT");
	_setfield(Polit::GOV_MILDICT1,      "MILDICT1");
	_setfield(Polit::GOV_MILDICT2,      "MILDICT2");
	_setfield(Polit::GOV_EMPIREMILDICT, "EMPIREMILDICT");
	_setfield(Polit::GOV_COMMUNIST,     "COMMUNIST");
	_setfield(Polit::GOV_PLUTOCRATIC,   "PLUTOCRATIC");
	_setfield(Polit::GOV_DISORDER,      "DISORDER");
	lua_setfield(L, -2, "GovType");

	lua_setfield(L, LUA_GLOBALSINDEX, "Polit");


	lua_newtable(L); // Equip

	lua_newtable(L); // Slot
	_setfield(Equip::SLOT_CARGO,            "CARGO");
	_setfield(Equip::SLOT_ENGINE,           "ENGINE");
	_setfield(Equip::SLOT_LASER,            "LASER");
	_setfield(Equip::SLOT_MISSILE,          "MISSILE");
	_setfield(Equip::SLOT_ECM,              "ECM");
	_setfield(Equip::SLOT_SCANNER,          "SCANNER");
	_setfield(Equip::SLOT_RADARMAPPER,      "RADARMAPPER");
	_setfield(Equip::SLOT_HYPERCLOUD,       "HYPERCLOUD");
	_setfield(Equip::SLOT_HULLAUTOREPAIR,   "HULLAUTOREPAIR");
	_setfield(Equip::SLOT_ENERGYBOOSTER,    "ENERGYBOOSTER");
	_setfield(Equip::SLOT_ATMOSHIELD,       "ATMOSHIELD");
	_setfield(Equip::SLOT_FUELSCOOP,        "FUELSCOOP");
	_setfield(Equip::SLOT_LASERCOOLER,      "LASERCOOLER");
	_setfield(Equip::SLOT_CARGOLIFESUPPORT, "CARGOLIFESUPPORT");
	_setfield(Equip::SLOT_AUTOPILOT,        "AUTOPILOT");
	lua_setfield(L, -2, "Slot");

	lua_newtable(L); // Type
	_setfield(Equip::NONE,                  "NONE");
	_setfield(Equip::HYDROGEN,              "HYDROGEN");
	_setfield(Equip::LIQUID_OXYGEN,         "LIQUID_OXYGEN");
	_setfield(Equip::METAL_ORE,             "METAL_ORE");
	_setfield(Equip::CARBON_ORE,            "CARBON_ORE");
	_setfield(Equip::METAL_ALLOYS,          "METAL_ALLOYS");
	_setfield(Equip::PLASTICS,              "PLASTICS");
	_setfield(Equip::FRUIT_AND_VEG,         "FRUIT_AND_VEG");
	_setfield(Equip::ANIMAL_MEAT,           "ANIMAL_MEAT");
	_setfield(Equip::LIVE_ANIMALS,          "LIVE_ANIMALS");
	_setfield(Equip::LIQUOR,                "LIQUOR");
	_setfield(Equip::GRAIN,                 "GRAIN");
	_setfield(Equip::TEXTILES,              "TEXTILES");
	_setfield(Equip::FERTILIZER,            "FERTILIZER");
	_setfield(Equip::WATER,                 "WATER");
	_setfield(Equip::MEDICINES,             "MEDICINES");
	_setfield(Equip::CONSUMER_GOODS,        "CONSUMER_GOODS");
	_setfield(Equip::COMPUTERS,             "COMPUTERS");
	_setfield(Equip::ROBOTS,                "ROBOTS");
	_setfield(Equip::PRECIOUS_METALS,       "PRECIOUS_METALS");
	_setfield(Equip::INDUSTRIAL_MACHINERY,  "INDUSTRIAL_MACHINERY");
	_setfield(Equip::FARM_MACHINERY,        "FARM_MACHINERY");
	_setfield(Equip::MINING_MACHINERY,      "MINING_MACHINERY");
	_setfield(Equip::AIR_PROCESSORS,        "AIR_PROCESSORS");
	_setfield(Equip::SLAVES,                "SLAVES");
	_setfield(Equip::HAND_WEAPONS,          "HAND_WEAPONS");
	_setfield(Equip::BATTLE_WEAPONS,        "BATTLE_WEAPONS");
	_setfield(Equip::NERVE_GAS,             "NERVE_GAS");
	_setfield(Equip::NARCOTICS,             "NARCOTICS");
	_setfield(Equip::MILITARY_FUEL,         "MILITARY_FUEL");
	_setfield(Equip::RUBBISH,               "RUBBISH");
	_setfield(Equip::RADIOACTIVES,          "RADIOACTIVES");

	_setfield(Equip::MISSILE_UNGUIDED,      "MISSILE_UNGUIDED");
	_setfield(Equip::MISSILE_GUIDED,        "MISSILE_GUIDED");
	_setfield(Equip::MISSILE_SMART,         "MISSILE_SMART");
	_setfield(Equip::MISSILE_NAVAL,         "MISSILE_NAVAL");
	_setfield(Equip::ATMOSPHERIC_SHIELDING, "ATMOSPHERIC_SHIELDING");
	_setfield(Equip::ECM_BASIC,             "ECM_BASIC");
	_setfield(Equip::SCANNER,               "SCANNER");
	_setfield(Equip::ECM_ADVANCED,          "ECM_ADVANCED");
	_setfield(Equip::SHIELD_GENERATOR,      "SHIELD_GENERATOR");
	_setfield(Equip::LASER_COOLING_BOOSTER, "LASER_COOLING_BOOSTER");
	_setfield(Equip::CARGO_LIFE_SUPPORT,    "CARGO_LIFE_SUPPORT");
	_setfield(Equip::AUTOPILOT,             "AUTOPILOT");
	_setfield(Equip::RADAR_MAPPER,          "RADAR_MAPPER");
	_setfield(Equip::FUEL_SCOOP,            "FUEL_SCOOP");
	_setfield(Equip::HYPERCLOUD_ANALYZER,   "HYPERCLOUD_ANALYZER");
	_setfield(Equip::HULL_AUTOREPAIR,       "HULL_AUTOREPAIR");
	_setfield(Equip::SHIELD_ENERGY_BOOSTER, "SHIELD_ENERGY_BOOSTER");

	_setfield(Equip::DRIVE_CLASS1,          "DRIVE_CLASS1");
	_setfield(Equip::DRIVE_CLASS2,          "DRIVE_CLASS2");
	_setfield(Equip::DRIVE_CLASS3,          "DRIVE_CLASS3");
	_setfield(Equip::DRIVE_CLASS4,          "DRIVE_CLASS4");
	_setfield(Equip::DRIVE_CLASS5,          "DRIVE_CLASS5");
	_setfield(Equip::DRIVE_CLASS6,          "DRIVE_CLASS6");
	_setfield(Equip::DRIVE_CLASS7,          "DRIVE_CLASS7");
	_setfield(Equip::DRIVE_CLASS8,          "DRIVE_CLASS8");
	_setfield(Equip::DRIVE_CLASS9,          "DRIVE_CLASS9");
	_setfield(Equip::DRIVE_MIL1,            "DRIVE_MIL1");
	_setfield(Equip::DRIVE_MIL2,            "DRIVE_MIL2");
	_setfield(Equip::DRIVE_MIL3,            "DRIVE_MIL3");
	_setfield(Equip::DRIVE_MIL4,            "DRIVE_MIL4");

	_setfield(Equip::PULSECANNON_1MW,       "PULSECANNON_1MW");
	_setfield(Equip::PULSECANNON_DUAL_1MW,  "PULSECANNON_DUAL_1MW");
	_setfield(Equip::PULSECANNON_2MW,       "PULSECANNON_2MW");
	_setfield(Equip::PULSECANNON_RAPID_2MW, "PULSECANNON_RAPID_2MW");
	_setfield(Equip::PULSECANNON_4MW,       "PULSECANNON_4MW");
	_setfield(Equip::PULSECANNON_10MW,      "PULSECANNON_10MW");
	_setfield(Equip::PULSECANNON_20MW,      "PULSECANNON_20MW");
	_setfield(Equip::MININGCANNON_17MW,     "MININGCANNON_17MW");
	_setfield(Equip::SMALL_PLASMA_ACCEL,    "SMALL_PLASMA_ACCEL");
	_setfield(Equip::LARGE_PLASMA_ACCEL,    "LARGE_PLASMA_ACCEL");

	_setfield(Equip::FIRST_COMMODITY,       "FIRST_COMMODITY");
	_setfield(Equip::LAST_COMMODITY,        "LAST_COMMODITY");
	_setfield(Equip::FIRST_SHIPEQUIP,       "FIRST_SHIPEQUIP");
	_setfield(Equip::LAST_SHIPEQUIP,        "LAST_SHIPEQUIP");
	lua_setfield(L, -2, "Type");

	lua_setfield(L, LUA_GLOBALSINDEX, "Equip");
}
