#include "LuaConstants.h"
#include "LuaManager.h"
#include "LuaUtils.h"
#include "LuaObject.h"
#include "StarSystem.h"
#include "Polit.h"
#include "EquipType.h"
#include "Player.h"
#include "ShipType.h"
#include "Ship.h"

struct pi_lua_constant_t {
	const char *name;
	int value;
};

int LuaConstants::GetConstant(lua_State *l, const char *ns, const char *name)
{
	lua_getfield(l, LUA_REGISTRYINDEX, "PiConstants");
	assert(lua_istable(l, -1));

	lua_getfield(l, -1, ns);
	assert(lua_istable(l, -1));

	lua_getfield(l, -1, name);
	if (lua_isnil(l, -1))
		luaL_error(l, "couldn't find constant with name '%s' in namespace '%s'\n", name, ns);
	assert(lua_isnumber(l, -1));

	int value = lua_tointeger(l, -1);

	lua_pop(l, 3);

	return value;
}

const char *LuaConstants::GetConstantString(lua_State *l, const char *ns, int value)
{
	lua_getfield(l, LUA_REGISTRYINDEX, "PiConstants");
	assert(lua_istable(l, -1));

	lua_getfield(l, -1, ns);
	assert(lua_istable(l, -1));

	lua_pushinteger(l, value);
	lua_rawget(l, -2);
	if (lua_isnil(l, -1))
		luaL_error(l, "couldn't find constant with value %d in namespace '%s'\n", value, ns);
	assert(lua_isstring(l, -1));

	const char *name = lua_tostring(l, -1);

	lua_pop(l, 3);

	return name;
}

static inline void _create_constant_table(lua_State *l, const char *ns, const pi_lua_constant_t *c)
{
	LUA_DEBUG_START(l);

	lua_getfield(l, LUA_GLOBALSINDEX, "Constants");
	if (lua_isnil(l, -1)) {
		lua_pop(l, 1);
		lua_newtable(l);
        pi_lua_table_ro(l);
		lua_pushstring(l, "Constants");
		lua_pushvalue(l, -2);
		lua_rawset(l, LUA_GLOBALSINDEX);
	}
	assert(lua_istable(l, -1));

	lua_newtable(l);
	pi_lua_table_ro(l);
	lua_pushstring(l, ns);
	lua_pushvalue(l, -2);
	lua_rawset(l, -4);

	lua_getfield(l, LUA_REGISTRYINDEX, "PiConstants");
	if (lua_isnil(l, -1)) {
		lua_pop(l, 1);
		lua_newtable(l);
		lua_pushstring(l, "PiConstants");
		lua_pushvalue(l, -2);
		lua_rawset(l, LUA_REGISTRYINDEX);
	}
	assert(lua_istable(l, -1));

	lua_newtable(l);
	pi_lua_table_ro(l);
	lua_pushstring(l, ns);
	lua_pushvalue(l, -2);
	lua_rawset(l, -4);

	for (; c->name; c++) {
		pi_lua_settable(l, c->name, c->value);
		pi_lua_settable(l, c->value, c->name);

		lua_pushinteger(l, lua_objlen(l, -3)+1);
		lua_pushstring(l, c->name);
		lua_rawset(l, -5);
	}

	lua_pop(l, 4);

	LUA_DEBUG_END(l, 0);
}

void LuaConstants::Register(lua_State *l)
{
	LUA_DEBUG_START(l);

	static const pi_lua_constant_t body_type_constants[] = {
		{ "GRAVPOINT",          SBody::TYPE_GRAVPOINT },
		{ "BROWN_DWARF",        SBody::TYPE_BROWN_DWARF },
		{ "STAR_M",             SBody::TYPE_STAR_M },
		{ "STAR_K",             SBody::TYPE_STAR_K },
		{ "STAR_G",             SBody::TYPE_STAR_G },
		{ "STAR_F",             SBody::TYPE_STAR_F },
		{ "STAR_A",             SBody::TYPE_STAR_A },
		{ "STAR_B",             SBody::TYPE_STAR_B },
		{ "STAR_O",             SBody::TYPE_STAR_O },
		{ "STAR_M_GIANT",       SBody::TYPE_STAR_M_GIANT },
		{ "STAR_K_GIANT",       SBody::TYPE_STAR_K_GIANT },
		{ "STAR_G_GIANT",       SBody::TYPE_STAR_G_GIANT },
		{ "STAR_F_GIANT",       SBody::TYPE_STAR_F_GIANT },
		{ "STAR_A_GIANT",       SBody::TYPE_STAR_A_GIANT },
		{ "STAR_B_GIANT",       SBody::TYPE_STAR_B_GIANT },
		{ "STAR_O_GIANT",       SBody::TYPE_STAR_O_GIANT },
		{ "STAR_M_SUPER_GIANT", SBody::TYPE_STAR_M_SUPER_GIANT },
		{ "STAR_K_SUPER_GIANT", SBody::TYPE_STAR_K_SUPER_GIANT },
		{ "STAR_G_SUPER_GIANT", SBody::TYPE_STAR_G_SUPER_GIANT },
		{ "STAR_F_SUPER_GIANT", SBody::TYPE_STAR_F_SUPER_GIANT },
		{ "STAR_A_SUPER_GIANT", SBody::TYPE_STAR_A_SUPER_GIANT },
		{ "STAR_B_SUPER_GIANT", SBody::TYPE_STAR_B_SUPER_GIANT },
		{ "STAR_O_SUPER_GIANT", SBody::TYPE_STAR_O_SUPER_GIANT },
		{ "STAR_M_HYPER_GIANT", SBody::TYPE_STAR_M_HYPER_GIANT },
		{ "STAR_K_HYPER_GIANT", SBody::TYPE_STAR_K_HYPER_GIANT },
		{ "STAR_G_HYPER_GIANT", SBody::TYPE_STAR_G_HYPER_GIANT },
		{ "STAR_F_HYPER_GIANT", SBody::TYPE_STAR_F_HYPER_GIANT },
		{ "STAR_A_HYPER_GIANT", SBody::TYPE_STAR_A_HYPER_GIANT },
		{ "STAR_B_HYPER_GIANT", SBody::TYPE_STAR_B_HYPER_GIANT },
		{ "STAR_O_HYPER_GIANT", SBody::TYPE_STAR_O_HYPER_GIANT },
		{ "STAR_M_WF",          SBody::TYPE_STAR_M_WF },
		{ "STAR_B_WF",          SBody::TYPE_STAR_B_WF },
		{ "STAR_O_WF",          SBody::TYPE_STAR_O_WF },
		{ "STAR_S_BH",          SBody::TYPE_STAR_S_BH },
		{ "STAR_IM_BH",         SBody::TYPE_STAR_IM_BH },
		{ "STAR_SM_BH",         SBody::TYPE_STAR_SM_BH },
		{ "WHITE_DWARF",        SBody::TYPE_WHITE_DWARF },
		{ "PLANET_GAS_GIANT",   SBody::TYPE_PLANET_GAS_GIANT },
		{ "PLANET_ASTEROID",    SBody::TYPE_PLANET_ASTEROID },
		{ "PLANET_TERRESTRIAL", SBody::TYPE_PLANET_TERRESTRIAL },
		{ "STARPORT_ORBITAL",   SBody::TYPE_STARPORT_ORBITAL },
		{ "STARPORT_SURFACE",   SBody::TYPE_STARPORT_SURFACE },
		{ 0, 0 }
	};
	_create_constant_table(l, "BodyType", body_type_constants);

	static const pi_lua_constant_t body_super_type_constants[] = {
		{ "NONE",         SBody::SUPERTYPE_NONE },
		{ "STAR",         SBody::SUPERTYPE_STAR },
		{ "ROCKY_PLANET", SBody::SUPERTYPE_ROCKY_PLANET },
		{ "GAS_GIANT",    SBody::SUPERTYPE_GAS_GIANT },
		{ "STARPORT",     SBody::SUPERTYPE_STARPORT },
		{ 0, 0 }
	};
	_create_constant_table(l, "BodySuperType", body_super_type_constants);


	static const pi_lua_constant_t polit_crime_constants[] = {
		{ "TRADING_ILLEGAL_GOODS", Polit::CRIME_TRADING_ILLEGAL_GOODS },
		{ "WEAPON_DISCHARGE",      Polit::CRIME_WEAPON_DISCHARGE },
		{ "PIRACY",                Polit::CRIME_PIRACY },
		{ "MURDER",                Polit::CRIME_MURDER },
		{ 0, 0 }
	};
	_create_constant_table(l, "PolitCrime", polit_crime_constants);

	static const pi_lua_constant_t polit_bloc_constants[] = {
		{ "NONE",     Polit::BLOC_NONE },
		{ "EARTHFED", Polit::BLOC_EARTHFED },
		{ "CIS",      Polit::BLOC_CIS },
		{ "EMPIRE",   Polit::BLOC_EMPIRE },
		{ 0, 0 }
	};
	_create_constant_table(l, "PolitBloc", polit_bloc_constants);

	static const pi_lua_constant_t polit_econ_constants[] = {
		{ "NONE",            Polit::ECON_NONE },
		{ "VERY_CAPITALIST", Polit::ECON_VERY_CAPITALIST },
		{ "CAPITALIST",      Polit::ECON_CAPITALIST },
		{ "MIXED",           Polit::ECON_MIXED },
		{ "PLANNED",         Polit::ECON_PLANNED },
		{ 0, 0 }
	};
	_create_constant_table(l, "PolitEcon", polit_econ_constants);

	static const pi_lua_constant_t polit_gov_type_constants[] = {
		{ "INVALID",       Polit::GOV_INVALID },
		{ "NONE",          Polit::GOV_NONE },
		{ "EARTHCOLONIAL", Polit::GOV_EARTHCOLONIAL },
		{ "EARTHDEMOC",    Polit::GOV_EARTHDEMOC },
		{ "EMPIRERULE",    Polit::GOV_EMPIRERULE },
		{ "CISLIBDEM",     Polit::GOV_CISLIBDEM },
		{ "CISSOCDEM",     Polit::GOV_CISSOCDEM },
		{ "LIBDEM",        Polit::GOV_LIBDEM },
		{ "CORPORATE",     Polit::GOV_CORPORATE },
		{ "SOCDEM",        Polit::GOV_SOCDEM },
		{ "EARTHMILDICT",  Polit::GOV_EARTHMILDICT },
		{ "MILDICT1",      Polit::GOV_MILDICT1 },
		{ "MILDICT2",      Polit::GOV_MILDICT2 },
		{ "EMPIREMILDICT", Polit::GOV_EMPIREMILDICT },
		{ "COMMUNIST",     Polit::GOV_COMMUNIST },
		{ "PLUTOCRATIC",   Polit::GOV_PLUTOCRATIC },
		{ "DISORDER",      Polit::GOV_DISORDER },
		{ 0, 0 }
	};
	_create_constant_table(l, "PolitGovType", polit_gov_type_constants);


	static const pi_lua_constant_t equip_slot_constants[] = {
		{ "CARGO",            Equip::SLOT_CARGO },
		{ "ENGINE",           Equip::SLOT_ENGINE },
		{ "LASER",            Equip::SLOT_LASER },
		{ "MISSILE",          Equip::SLOT_MISSILE },
		{ "ECM",              Equip::SLOT_ECM },
		{ "SCANNER",          Equip::SLOT_SCANNER },
		{ "RADARMAPPER",      Equip::SLOT_RADARMAPPER },
		{ "HYPERCLOUD",       Equip::SLOT_HYPERCLOUD },
		{ "HULLAUTOREPAIR",   Equip::SLOT_HULLAUTOREPAIR },
		{ "ENERGYBOOSTER",    Equip::SLOT_ENERGYBOOSTER },
		{ "ATMOSHIELD",       Equip::SLOT_ATMOSHIELD },
		{ "FUELSCOOP",        Equip::SLOT_FUELSCOOP },
		{ "LASERCOOLER",      Equip::SLOT_LASERCOOLER },
		{ "CARGOLIFESUPPORT", Equip::SLOT_CARGOLIFESUPPORT },
		{ "AUTOPILOT",        Equip::SLOT_AUTOPILOT },
		{ 0, 0 }
	};
	_create_constant_table(l, "EquipSlot", equip_slot_constants);

	static const pi_lua_constant_t equip_type_constants[] = {
		{ "NONE",                 Equip::NONE },
		{ "HYDROGEN",             Equip::HYDROGEN },
		{ "LIQUID_OXYGEN",        Equip::LIQUID_OXYGEN },
		{ "METAL_ORE",            Equip::METAL_ORE },
		{ "CARBON_ORE",           Equip::CARBON_ORE },
		{ "METAL_ALLOYS",         Equip::METAL_ALLOYS },
		{ "PLASTICS",             Equip::PLASTICS },
		{ "FRUIT_AND_VEG",        Equip::FRUIT_AND_VEG },
		{ "ANIMAL_MEAT",          Equip::ANIMAL_MEAT },
		{ "LIVE_ANIMALS",         Equip::LIVE_ANIMALS },
		{ "LIQUOR",               Equip::LIQUOR },
		{ "GRAIN",                Equip::GRAIN },
		{ "TEXTILES",             Equip::TEXTILES },
		{ "FERTILIZER",           Equip::FERTILIZER },
		{ "WATER",                Equip::WATER },
		{ "MEDICINES",            Equip::MEDICINES },
		{ "CONSUMER_GOODS",       Equip::CONSUMER_GOODS },
		{ "COMPUTERS",            Equip::COMPUTERS },
		{ "ROBOTS",               Equip::ROBOTS },
		{ "PRECIOUS_METALS",      Equip::PRECIOUS_METALS },
		{ "INDUSTRIAL_MACHINERY", Equip::INDUSTRIAL_MACHINERY },
		{ "FARM_MACHINERY",       Equip::FARM_MACHINERY },
		{ "MINING_MACHINERY",     Equip::MINING_MACHINERY },
		{ "AIR_PROCESSORS",       Equip::AIR_PROCESSORS },
		{ "SLAVES",               Equip::SLAVES },
		{ "HAND_WEAPONS",         Equip::HAND_WEAPONS },
		{ "BATTLE_WEAPONS",       Equip::BATTLE_WEAPONS },
		{ "NERVE_GAS",            Equip::NERVE_GAS },
		{ "NARCOTICS",            Equip::NARCOTICS },
		{ "MILITARY_FUEL",        Equip::MILITARY_FUEL },
		{ "RUBBISH",              Equip::RUBBISH },
		{ "RADIOACTIVES",         Equip::RADIOACTIVES },

		{ "MISSILE_UNGUIDED",      Equip::MISSILE_UNGUIDED },
		{ "MISSILE_GUIDED",        Equip::MISSILE_GUIDED },
		{ "MISSILE_SMART",         Equip::MISSILE_SMART },
		{ "MISSILE_NAVAL",         Equip::MISSILE_NAVAL },
		{ "ATMOSPHERIC_SHIELDING", Equip::ATMOSPHERIC_SHIELDING },
		{ "ECM_BASIC",             Equip::ECM_BASIC },
		{ "SCANNER",               Equip::SCANNER },
		{ "ECM_ADVANCED",          Equip::ECM_ADVANCED },
		{ "SHIELD_GENERATOR",      Equip::SHIELD_GENERATOR },
		{ "LASER_COOLING_BOOSTER", Equip::LASER_COOLING_BOOSTER },
		{ "CARGO_LIFE_SUPPORT",    Equip::CARGO_LIFE_SUPPORT },
		{ "AUTOPILOT",             Equip::AUTOPILOT },
		{ "RADAR_MAPPER",          Equip::RADAR_MAPPER },
		{ "FUEL_SCOOP",            Equip::FUEL_SCOOP },
		{ "HYPERCLOUD_ANALYZER",   Equip::HYPERCLOUD_ANALYZER },
		{ "HULL_AUTOREPAIR",       Equip::HULL_AUTOREPAIR },
		{ "SHIELD_ENERGY_BOOSTER", Equip::SHIELD_ENERGY_BOOSTER },

		{ "DRIVE_CLASS1", Equip::DRIVE_CLASS1 },
		{ "DRIVE_CLASS2", Equip::DRIVE_CLASS2 },
		{ "DRIVE_CLASS3", Equip::DRIVE_CLASS3 },
		{ "DRIVE_CLASS4", Equip::DRIVE_CLASS4 },
		{ "DRIVE_CLASS5", Equip::DRIVE_CLASS5 },
		{ "DRIVE_CLASS6", Equip::DRIVE_CLASS6 },
		{ "DRIVE_CLASS7", Equip::DRIVE_CLASS7 },
		{ "DRIVE_CLASS8", Equip::DRIVE_CLASS8 },
		{ "DRIVE_CLASS9", Equip::DRIVE_CLASS9 },
		{ "DRIVE_MIL1",   Equip::DRIVE_MIL1 },
		{ "DRIVE_MIL2",   Equip::DRIVE_MIL2 },
		{ "DRIVE_MIL3",   Equip::DRIVE_MIL3 },
		{ "DRIVE_MIL4",   Equip::DRIVE_MIL4 },

		{ "PULSECANNON_1MW",       Equip::PULSECANNON_1MW },
		{ "PULSECANNON_DUAL_1MW",  Equip::PULSECANNON_DUAL_1MW },
		{ "PULSECANNON_2MW",       Equip::PULSECANNON_2MW },
		{ "PULSECANNON_RAPID_2MW", Equip::PULSECANNON_RAPID_2MW },
		{ "PULSECANNON_4MW",       Equip::PULSECANNON_4MW },
		{ "PULSECANNON_10MW",      Equip::PULSECANNON_10MW },
		{ "PULSECANNON_20MW",      Equip::PULSECANNON_20MW },
		{ "MININGCANNON_17MW",     Equip::MININGCANNON_17MW },
		{ "SMALL_PLASMA_ACCEL",    Equip::SMALL_PLASMA_ACCEL },
		{ "LARGE_PLASMA_ACCEL",    Equip::LARGE_PLASMA_ACCEL },
		{ 0, 0 }
	};
	_create_constant_table(l, "EquipType", equip_type_constants);


	static const pi_lua_constant_t ship_type_tag_constants[] = {
		{ "NONE",        ShipType::TAG_NONE },
		{ "SHIP",        ShipType::TAG_SHIP },
		{ "STATIC_SHIP", ShipType::TAG_STATIC_SHIP },
		{ "MISSILE",     ShipType::TAG_MISSILE },
		{ 0, 0 }
	};
	_create_constant_table(l, "ShipTypeTag", ship_type_tag_constants);

	static const pi_lua_constant_t ship_type_thruster_constants[] = {
		{ "REVERSE", ShipType::THRUSTER_REVERSE },
		{ "FORWARD", ShipType::THRUSTER_FORWARD },
		{ "UP",      ShipType::THRUSTER_UP },
		{ "DOWN",    ShipType::THRUSTER_DOWN },
		{ "LEFT",    ShipType::THRUSTER_LEFT },
		{ "RIGHT",   ShipType::THRUSTER_RIGHT },
		{ 0, 0 }
	};
	_create_constant_table(l, "ShipTypeThruster", ship_type_thruster_constants);


	static const pi_lua_constant_t ship_jump_status_constants[] = {
		{ "OK",                Ship::HYPERJUMP_OK },
		{ "CURRENT_SYSTEM",    Ship::HYPERJUMP_CURRENT_SYSTEM },
		{ "NO_DRIVE",          Ship::HYPERJUMP_NO_DRIVE },
		{ "OUT_OF_RANGE",      Ship::HYPERJUMP_OUT_OF_RANGE },
		{ "INSUFFICIENT_FUEL", Ship::HYPERJUMP_INSUFFICIENT_FUEL },
		{ 0, 0 }
	};
	_create_constant_table(l, "ShipJumpStatus", ship_jump_status_constants);


	static const pi_lua_constant_t ship_alert_status_constants[] = {
		{ "NONE",        Ship::ALERT_NONE },
		{ "SHIP_NEARBY", Ship::ALERT_SHIP_NEARBY },
		{ "SHIP_FIRING", Ship::ALERT_SHIP_FIRING },
		{ 0, 0 }
	};
	_create_constant_table(l, "ShipAlertStatus", ship_alert_status_constants);

	LUA_DEBUG_END(l, 0);
}
