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

/*
 * Title: Constants
 *
 * Various constants exported from the game engine to the Lua environment.
 */

static inline void _get_named_table(lua_State *l, int index, const char *name)
{
	if (index != LUA_GLOBALSINDEX) index = lua_gettop(l) + index + 1;

	lua_getfield(l, index, name);
	if (lua_isnil(l, -1)) {
		lua_pop(l, 1);
		lua_newtable(l);
		lua_pushstring(l, name);
		lua_pushvalue(l, -2);
		lua_settable(l, index);
	}
	assert(lua_istable(l, -1));
}

void LuaConstants::Register(lua_State *l)
{
	LUA_DEBUG_START(l);

	_get_named_table(l, LUA_GLOBALSINDEX, "Body");
	
	/*
	 * Constants: Body.Type
     *
	 *   Body.Type.GRAVPOINT - .
	 *   Body.Type.BROWN_DWARF - .
	 *   Body.Type.STAR_M - .
	 *   Body.Type.STAR_K - .
	 *   Body.Type.STAR_G - .
	 *   Body.Type.STAR_F - .
	 *   Body.Type.STAR_A - .
	 *   Body.Type.STAR_B - .
	 *   Body.Type.STAR_O - .
	 *   Body.Type.STAR_M_GIANT - .
	 *   Body.Type.STAR_K_GIANT - .
	 *   Body.Type.STAR_G_GIANT - .
	 *   Body.Type.STAR_F_GIANT - .
	 *   Body.Type.STAR_A_GIANT - .
	 *   Body.Type.STAR_B_GIANT - .
	 *   Body.Type.STAR_O_GIANT - .
	 *   Body.Type.STAR_M_SUPER_GIANT - .
	 *   Body.Type.STAR_K_SUPER_GIANT - .
	 *   Body.Type.STAR_G_SUPER_GIANT - .
	 *   Body.Type.STAR_F_SUPER_GIANT - .
	 *   Body.Type.STAR_A_SUPER_GIANT - .
	 *   Body.Type.STAR_B_SUPER_GIANT - .
	 *   Body.Type.STAR_O_SUPER_GIANT - .
	 *   Body.Type.STAR_M_HYPER_GIANT - .
	 *   Body.Type.STAR_K_HYPER_GIANT - .
	 *   Body.Type.STAR_G_HYPER_GIANT - .
	 *   Body.Type.STAR_F_HYPER_GIANT - .
	 *   Body.Type.STAR_A_HYPER_GIANT - .
	 *   Body.Type.STAR_B_HYPER_GIANT - .
	 *   Body.Type.STAR_O_HYPER_GIANT - .
	 *   Body.Type.STAR_M_WF - .
	 *   Body.Type.STAR_B_WF - .
	 *   Body.Type.STAR_O_WF - .
	 *   Body.Type.STAR_S_BH - .
	 *   Body.Type.STAR_IM_BH - .
	 *   Body.Type.STAR_SM_BH - .
	 *   Body.Type.WHITE_DWARF - .
	 *   Body.Type.PLANET_GAS_GIANT - .
	 *   Body.Type.PLANET_ASTEROID - .
	 *   Body.Type.PLANET_TERRESTRIAL - .
	 *   Body.Type.STARPORT_ORBITAL - .
	 *   Body.Type.STARPORT_SURFACE - .
     */
	_get_named_table(l, -1, "Type");
	pi_lua_settable(l, "GRAVPOINT",          SBody::TYPE_GRAVPOINT);
	pi_lua_settable(l, "BROWN_DWARF",        SBody::TYPE_BROWN_DWARF);
	pi_lua_settable(l, "STAR_M",             SBody::TYPE_STAR_M);
	pi_lua_settable(l, "STAR_K",             SBody::TYPE_STAR_K);
	pi_lua_settable(l, "STAR_G",             SBody::TYPE_STAR_G);
	pi_lua_settable(l, "STAR_F",             SBody::TYPE_STAR_F);
	pi_lua_settable(l, "STAR_A",             SBody::TYPE_STAR_A);
	pi_lua_settable(l, "STAR_B",             SBody::TYPE_STAR_B);
	pi_lua_settable(l, "STAR_O",             SBody::TYPE_STAR_O);
	pi_lua_settable(l, "STAR_M_GIANT",       SBody::TYPE_STAR_M_GIANT);
	pi_lua_settable(l, "STAR_K_GIANT",       SBody::TYPE_STAR_K_GIANT);
	pi_lua_settable(l, "STAR_G_GIANT",       SBody::TYPE_STAR_G_GIANT);
	pi_lua_settable(l, "STAR_F_GIANT",       SBody::TYPE_STAR_F_GIANT);
	pi_lua_settable(l, "STAR_A_GIANT",       SBody::TYPE_STAR_A_GIANT);
	pi_lua_settable(l, "STAR_B_GIANT",       SBody::TYPE_STAR_B_GIANT);
	pi_lua_settable(l, "STAR_O_GIANT",       SBody::TYPE_STAR_O_GIANT);
	pi_lua_settable(l, "STAR_M_SUPER_GIANT", SBody::TYPE_STAR_M_SUPER_GIANT);
	pi_lua_settable(l, "STAR_K_SUPER_GIANT", SBody::TYPE_STAR_K_SUPER_GIANT);
	pi_lua_settable(l, "STAR_G_SUPER_GIANT", SBody::TYPE_STAR_G_SUPER_GIANT);
	pi_lua_settable(l, "STAR_F_SUPER_GIANT", SBody::TYPE_STAR_F_SUPER_GIANT);
	pi_lua_settable(l, "STAR_A_SUPER_GIANT", SBody::TYPE_STAR_A_SUPER_GIANT);
	pi_lua_settable(l, "STAR_B_SUPER_GIANT", SBody::TYPE_STAR_B_SUPER_GIANT);
	pi_lua_settable(l, "STAR_O_SUPER_GIANT", SBody::TYPE_STAR_O_SUPER_GIANT);
	pi_lua_settable(l, "STAR_M_HYPER_GIANT", SBody::TYPE_STAR_M_HYPER_GIANT);
	pi_lua_settable(l, "STAR_K_HYPER_GIANT", SBody::TYPE_STAR_K_HYPER_GIANT);
	pi_lua_settable(l, "STAR_G_HYPER_GIANT", SBody::TYPE_STAR_G_HYPER_GIANT);
	pi_lua_settable(l, "STAR_F_HYPER_GIANT", SBody::TYPE_STAR_F_HYPER_GIANT);
	pi_lua_settable(l, "STAR_A_HYPER_GIANT", SBody::TYPE_STAR_A_HYPER_GIANT);
	pi_lua_settable(l, "STAR_B_HYPER_GIANT", SBody::TYPE_STAR_B_HYPER_GIANT);
	pi_lua_settable(l, "STAR_O_HYPER_GIANT", SBody::TYPE_STAR_O_HYPER_GIANT);
	pi_lua_settable(l, "STAR_M_WF",          SBody::TYPE_STAR_M_WF);
	pi_lua_settable(l, "STAR_B_WF",          SBody::TYPE_STAR_B_WF);
	pi_lua_settable(l, "STAR_O_WF",          SBody::TYPE_STAR_O_WF);
	pi_lua_settable(l, "STAR_S_BH",          SBody::TYPE_STAR_S_BH);
	pi_lua_settable(l, "STAR_IM_BH",         SBody::TYPE_STAR_IM_BH);
	pi_lua_settable(l, "STAR_SM_BH",         SBody::TYPE_STAR_SM_BH);
	pi_lua_settable(l, "WHITE_DWARF",        SBody::TYPE_WHITE_DWARF);
	pi_lua_settable(l, "PLANET_GAS_GIANT",   SBody::TYPE_PLANET_GAS_GIANT);
	pi_lua_settable(l, "PLANET_ASTEROID",    SBody::TYPE_PLANET_ASTEROID);
	pi_lua_settable(l, "PLANET_TERRESTRIAL", SBody::TYPE_PLANET_TERRESTRIAL);
	pi_lua_settable(l, "STARPORT_ORBITAL",   SBody::TYPE_STARPORT_ORBITAL);
	pi_lua_settable(l, "STARPORT_SURFACE",   SBody::TYPE_STARPORT_SURFACE);
	lua_pop(l, 1);

    /*
     * Constants: Body.SuperType
     *
	 *   Body.SuperType.NONE - .
	 *   Body.SuperType.STAR - .
	 *   Body.SuperType.ROCKY_PLANET - .
	 *   Body.SuperType.GAS_GIANT - .
	 *   Body.SuperType.STARPORT - .
     */
	_get_named_table(l, -1, "SuperType");
	pi_lua_settable(l, "NONE",         SBody::SUPERTYPE_NONE);
	pi_lua_settable(l, "STAR",         SBody::SUPERTYPE_STAR);
	pi_lua_settable(l, "ROCKY_PLANET", SBody::SUPERTYPE_ROCKY_PLANET);
	pi_lua_settable(l, "GAS_GIANT",    SBody::SUPERTYPE_GAS_GIANT);
	pi_lua_settable(l, "STARPORT",     SBody::SUPERTYPE_STARPORT);
	lua_pop(l, 1);

	lua_pop(l, 1);


	_get_named_table(l, LUA_GLOBALSINDEX, "Polit");

    /*
     * Constants: Polit.Crime
     *
	 *   Polit.Crime.TRADING_ILLEGAL_GOODS - .
	 *   Polit.Crime.WEAPON_DISCHARGE - .
	 *   Polit.Crime.PIRACY - .
	 *   Polit.Crime.MURDER - .
	 */
	_get_named_table(l, -1, "Crime");
	pi_lua_settable(l, "TRADING_ILLEGAL_GOODS", Polit::CRIME_TRADING_ILLEGAL_GOODS);
	pi_lua_settable(l, "WEAPON_DISCHARGE",      Polit::CRIME_WEAPON_DISCHARGE);
	pi_lua_settable(l, "PIRACY",                Polit::CRIME_PIRACY);
	pi_lua_settable(l, "MURDER",                Polit::CRIME_MURDER);
	lua_pop(l, 1);

    /*
     * Constants: Polit.Bloc
     *
	 *   Polit.Bloc.NONE - .
	 *   Polit.Bloc.EARTHFED - .
	 *   Polit.Bloc.CIS - .
	 *   Polit.Bloc.EMPIRE - .
	 */
	_get_named_table(l, -1, "Bloc");
	pi_lua_settable(l, "NONE",     Polit::BLOC_NONE);
	pi_lua_settable(l, "EARTHFED", Polit::BLOC_EARTHFED);
	pi_lua_settable(l, "CIS",      Polit::BLOC_CIS);
	pi_lua_settable(l, "EMPIRE",   Polit::BLOC_EMPIRE);
	lua_pop(l, 1);

    /*
     * Constants: Polit.Econ
     *
	 *   Polit.Econ.NONE - .
	 *   Polit.Econ.VERY_CAPITALIST - .
	 *   Polit.Econ.CAPITALIST - .
	 *   Polit.Econ.MIXED - .
	 *   Polit.Econ.PLANNED - .
	 */
	_get_named_table(l, -1, "Econ");
	pi_lua_settable(l, "NONE",            Polit::ECON_NONE);
	pi_lua_settable(l, "VERY_CAPITALIST", Polit::ECON_VERY_CAPITALIST);
	pi_lua_settable(l, "CAPITALIST",      Polit::ECON_CAPITALIST);
	pi_lua_settable(l, "MIXED",           Polit::ECON_MIXED);
	pi_lua_settable(l, "PLANNED",         Polit::ECON_PLANNED);
	lua_pop(l, 1);

    /*
     * Constants: Polit.GovType
     *
	 *   Polit.GovType.INVALID - .
	 *   Polit.GovType.NONE - .
	 *   Polit.GovType.EARTHCOLONIAL - .
	 *   Polit.GovType.EARTHDEMOC - .
	 *   Polit.GovType.EMPIRERULE - .
	 *   Polit.GovType.CISLIBDEM - .
	 *   Polit.GovType.CISSOCDEM - .
	 *   Polit.GovType.LIBDEM - .
	 *   Polit.GovType.CORPORATE - .
	 *   Polit.GovType.SOCDEM - .
	 *   Polit.GovType.EARTHMILDICT - .
	 *   Polit.GovType.MILDICT1 - .
	 *   Polit.GovType.MILDICT2 - .
	 *   Polit.GovType.EMPIREMILDICT - .
	 *   Polit.GovType.COMMUNIST - .
	 *   Polit.GovType.PLUTOCRATIC - .
	 *   Polit.GovType.DISORDER - .
	 */
	_get_named_table(l, -1, "GovType");
	pi_lua_settable(l, "INVALID",       Polit::GOV_INVALID);
	pi_lua_settable(l, "NONE",          Polit::GOV_NONE);
	pi_lua_settable(l, "EARTHCOLONIAL", Polit::GOV_EARTHCOLONIAL);
	pi_lua_settable(l, "EARTHDEMOC",    Polit::GOV_EARTHDEMOC);
	pi_lua_settable(l, "EMPIRERULE",    Polit::GOV_EMPIRERULE);
	pi_lua_settable(l, "CISLIBDEM",     Polit::GOV_CISLIBDEM);
	pi_lua_settable(l, "CISSOCDEM",     Polit::GOV_CISSOCDEM);
	pi_lua_settable(l, "LIBDEM",        Polit::GOV_LIBDEM);
	pi_lua_settable(l, "CORPORATE",     Polit::GOV_CORPORATE);
	pi_lua_settable(l, "SOCDEM",        Polit::GOV_SOCDEM);
	pi_lua_settable(l, "EARTHMILDICT",  Polit::GOV_EARTHMILDICT);
	pi_lua_settable(l, "MILDICT1",      Polit::GOV_MILDICT1);
	pi_lua_settable(l, "MILDICT2",      Polit::GOV_MILDICT2);
	pi_lua_settable(l, "EMPIREMILDICT", Polit::GOV_EMPIREMILDICT);
	pi_lua_settable(l, "COMMUNIST",     Polit::GOV_COMMUNIST);
	pi_lua_settable(l, "PLUTOCRATIC",   Polit::GOV_PLUTOCRATIC);
	pi_lua_settable(l, "DISORDER",      Polit::GOV_DISORDER);
	lua_pop(l, 1);

	lua_pop(l, 1);


	_get_named_table(l, LUA_GLOBALSINDEX, "Equip");

    /*
     * Constants: Equip.Slot
     *
	 *   Equip.Slot.CARGO - .
	 *   Equip.Slot.ENGINE - .
	 *   Equip.Slot.LASER - .
	 *   Equip.Slot.MISSILE - .
	 *   Equip.Slot.ECM - .
	 *   Equip.Slot.SCANNER - .
	 *   Equip.Slot.RADARMAPPER - .
	 *   Equip.Slot.HYPERCLOUD - .
	 *   Equip.Slot.HULLAUTOREPAIR - .
	 *   Equip.Slot.ENERGYBOOSTER - .
	 *   Equip.Slot.ATMOSHIELD - .
	 *   Equip.Slot.FUELSCOOP - .
	 *   Equip.Slot.LASERCOOLER - .
	 *   Equip.Slot.CARGOLIFESUPPORT - .
	 *   Equip.Slot.AUTOPILOT - .
	 */
	_get_named_table(l, -1, "Slot");
	pi_lua_settable(l, "CARGO",            Equip::SLOT_CARGO);
	pi_lua_settable(l, "ENGINE",           Equip::SLOT_ENGINE);
	pi_lua_settable(l, "LASER",            Equip::SLOT_LASER);
	pi_lua_settable(l, "MISSILE",          Equip::SLOT_MISSILE);
	pi_lua_settable(l, "ECM",              Equip::SLOT_ECM);
	pi_lua_settable(l, "SCANNER",          Equip::SLOT_SCANNER);
	pi_lua_settable(l, "RADARMAPPER",      Equip::SLOT_RADARMAPPER);
	pi_lua_settable(l, "HYPERCLOUD",       Equip::SLOT_HYPERCLOUD);
	pi_lua_settable(l, "HULLAUTOREPAIR",   Equip::SLOT_HULLAUTOREPAIR);
	pi_lua_settable(l, "ENERGYBOOSTER",    Equip::SLOT_ENERGYBOOSTER);
	pi_lua_settable(l, "ATMOSHIELD",       Equip::SLOT_ATMOSHIELD);
	pi_lua_settable(l, "FUELSCOOP",        Equip::SLOT_FUELSCOOP);
	pi_lua_settable(l, "LASERCOOLER",      Equip::SLOT_LASERCOOLER);
	pi_lua_settable(l, "CARGOLIFESUPPORT", Equip::SLOT_CARGOLIFESUPPORT);
	pi_lua_settable(l, "AUTOPILOT",        Equip::SLOT_AUTOPILOT);
	lua_pop(l, 1);

    /*
     * Constants: Equip.Type
     *
	 *   Equip.Type.NONE - .
	 *   Equip.Type.HYDROGEN - .
	 *   Equip.Type.LIQUID_OXYGEN - .
	 *   Equip.Type.METAL_ORE - .
	 *   Equip.Type.CARBON_ORE - .
	 *   Equip.Type.METAL_ALLOYS - .
	 *   Equip.Type.PLASTICS - .
	 *   Equip.Type.FRUIT_AND_VEG - .
	 *   Equip.Type.ANIMAL_MEAT - .
	 *   Equip.Type.LIVE_ANIMALS - .
	 *   Equip.Type.LIQUOR - .
	 *   Equip.Type.GRAIN - .
	 *   Equip.Type.TEXTILES - .
	 *   Equip.Type.FERTILIZER - .
	 *   Equip.Type.WATER - .
	 *   Equip.Type.MEDICINES - .
	 *   Equip.Type.CONSUMER_GOODS - .
	 *   Equip.Type.COMPUTERS - .
	 *   Equip.Type.ROBOTS - .
	 *   Equip.Type.PRECIOUS_METALS - .
	 *   Equip.Type.INDUSTRIAL_MACHINERY - .
	 *   Equip.Type.FARM_MACHINERY - .
	 *   Equip.Type.MINING_MACHINERY - .
	 *   Equip.Type.AIR_PROCESSORS - .
	 *   Equip.Type.SLAVES - .
	 *   Equip.Type.HAND_WEAPONS - .
	 *   Equip.Type.BATTLE_WEAPONS - .
	 *   Equip.Type.NERVE_GAS - .
	 *   Equip.Type.NARCOTICS - .
	 *   Equip.Type.MILITARY_FUEL - .
	 *   Equip.Type.RUBBISH - .
	 *   Equip.Type.RADIOACTIVES - .
	 *   Equip.Type.MISSILE_UNGUIDED - .
	 *   Equip.Type.MISSILE_GUIDED - .
	 *   Equip.Type.MISSILE_SMART - .
	 *   Equip.Type.MISSILE_NAVAL - .
	 *   Equip.Type.ATMOSPHERIC_SHIELDING - .
	 *   Equip.Type.ECM_BASIC - .
	 *   Equip.Type.SCANNER - .
	 *   Equip.Type.ECM_ADVANCED - .
	 *   Equip.Type.SHIELD_GENERATOR - .
	 *   Equip.Type.LASER_COOLING_BOOSTER - .
	 *   Equip.Type.CARGO_LIFE_SUPPORT - .
	 *   Equip.Type.AUTOPILOT - .
	 *   Equip.Type.RADAR_MAPPER - .
	 *   Equip.Type.FUEL_SCOOP - .
	 *   Equip.Type.HYPERCLOUD_ANALYZER - .
	 *   Equip.Type.HULL_AUTOREPAIR - .
	 *   Equip.Type.SHIELD_ENERGY_BOOSTER - .
	 *   Equip.Type.DRIVE_CLASS1 - .
	 *   Equip.Type.DRIVE_CLASS2 - .
	 *   Equip.Type.DRIVE_CLASS3 - .
	 *   Equip.Type.DRIVE_CLASS4 - .
	 *   Equip.Type.DRIVE_CLASS5 - .
	 *   Equip.Type.DRIVE_CLASS6 - .
	 *   Equip.Type.DRIVE_CLASS7 - .
	 *   Equip.Type.DRIVE_CLASS8 - .
	 *   Equip.Type.DRIVE_CLASS9 - .
	 *   Equip.Type.DRIVE_MIL1 - .
	 *   Equip.Type.DRIVE_MIL2 - .
	 *   Equip.Type.DRIVE_MIL3 - .
	 *   Equip.Type.DRIVE_MIL4 - .
	 *   Equip.Type.PULSECANNON_1MW - .
	 *   Equip.Type.PULSECANNON_DUAL_1MW - .
	 *   Equip.Type.PULSECANNON_2MW - .
	 *   Equip.Type.PULSECANNON_RAPID_2MW - .
	 *   Equip.Type.PULSECANNON_4MW - .
	 *   Equip.Type.PULSECANNON_10MW - .
	 *   Equip.Type.PULSECANNON_20MW - .
	 *   Equip.Type.MININGCANNON_17MW - .
	 *   Equip.Type.SMALL_PLASMA_ACCEL - .
	 *   Equip.Type.LARGE_PLASMA_ACCEL - .
	 *   Equip.Type.FIRST_COMMODITY - .
	 *   Equip.Type.LAST_COMMODITY - .
	 *   Equip.Type.FIRST_SHIPEQUIP - .
	 *   Equip.Type.LAST_SHIPEQUIP - .
	 */
	_get_named_table(l, -1, "Type");
	pi_lua_settable(l, "NONE",                 Equip::NONE);
	pi_lua_settable(l, "HYDROGEN",             Equip::HYDROGEN);
	pi_lua_settable(l, "LIQUID_OXYGEN",        Equip::LIQUID_OXYGEN);
	pi_lua_settable(l, "METAL_ORE",            Equip::METAL_ORE);
	pi_lua_settable(l, "CARBON_ORE",           Equip::CARBON_ORE);
	pi_lua_settable(l, "METAL_ALLOYS",         Equip::METAL_ALLOYS);
	pi_lua_settable(l, "PLASTICS",             Equip::PLASTICS);
	pi_lua_settable(l, "FRUIT_AND_VEG",        Equip::FRUIT_AND_VEG);
	pi_lua_settable(l, "ANIMAL_MEAT",          Equip::ANIMAL_MEAT);
	pi_lua_settable(l, "LIVE_ANIMALS",         Equip::LIVE_ANIMALS);
	pi_lua_settable(l, "LIQUOR",               Equip::LIQUOR);
	pi_lua_settable(l, "GRAIN",                Equip::GRAIN);
	pi_lua_settable(l, "TEXTILES",             Equip::TEXTILES);
	pi_lua_settable(l, "FERTILIZER",           Equip::FERTILIZER);
	pi_lua_settable(l, "WATER",                Equip::WATER);
	pi_lua_settable(l, "MEDICINES",            Equip::MEDICINES);
	pi_lua_settable(l, "CONSUMER_GOODS",       Equip::CONSUMER_GOODS);
	pi_lua_settable(l, "COMPUTERS",            Equip::COMPUTERS);
	pi_lua_settable(l, "ROBOTS",               Equip::ROBOTS);
	pi_lua_settable(l, "PRECIOUS_METALS",      Equip::PRECIOUS_METALS);
	pi_lua_settable(l, "INDUSTRIAL_MACHINERY", Equip::INDUSTRIAL_MACHINERY);
	pi_lua_settable(l, "FARM_MACHINERY",       Equip::FARM_MACHINERY);
	pi_lua_settable(l, "MINING_MACHINERY",     Equip::MINING_MACHINERY);
	pi_lua_settable(l, "AIR_PROCESSORS",       Equip::AIR_PROCESSORS);
	pi_lua_settable(l, "SLAVES",               Equip::SLAVES);
	pi_lua_settable(l, "HAND_WEAPONS",         Equip::HAND_WEAPONS);
	pi_lua_settable(l, "BATTLE_WEAPONS",       Equip::BATTLE_WEAPONS);
	pi_lua_settable(l, "NERVE_GAS",            Equip::NERVE_GAS);
	pi_lua_settable(l, "NARCOTICS",            Equip::NARCOTICS);
	pi_lua_settable(l, "MILITARY_FUEL",        Equip::MILITARY_FUEL);
	pi_lua_settable(l, "RUBBISH",              Equip::RUBBISH);
	pi_lua_settable(l, "RADIOACTIVES",         Equip::RADIOACTIVES);

	pi_lua_settable(l, "MISSILE_UNGUIDED",      Equip::MISSILE_UNGUIDED);
	pi_lua_settable(l, "MISSILE_GUIDED",        Equip::MISSILE_GUIDED);
	pi_lua_settable(l, "MISSILE_SMART",         Equip::MISSILE_SMART);
	pi_lua_settable(l, "MISSILE_NAVAL",         Equip::MISSILE_NAVAL);
	pi_lua_settable(l, "ATMOSPHERIC_SHIELDING", Equip::ATMOSPHERIC_SHIELDING);
	pi_lua_settable(l, "ECM_BASIC",             Equip::ECM_BASIC);
	pi_lua_settable(l, "SCANNER",               Equip::SCANNER);
	pi_lua_settable(l, "ECM_ADVANCED",          Equip::ECM_ADVANCED);
	pi_lua_settable(l, "SHIELD_GENERATOR",      Equip::SHIELD_GENERATOR);
	pi_lua_settable(l, "LASER_COOLING_BOOSTER", Equip::LASER_COOLING_BOOSTER);
	pi_lua_settable(l, "CARGO_LIFE_SUPPORT",    Equip::CARGO_LIFE_SUPPORT);
	pi_lua_settable(l, "AUTOPILOT",             Equip::AUTOPILOT);
	pi_lua_settable(l, "RADAR_MAPPER",          Equip::RADAR_MAPPER);
	pi_lua_settable(l, "FUEL_SCOOP",            Equip::FUEL_SCOOP);
	pi_lua_settable(l, "HYPERCLOUD_ANALYZER",   Equip::HYPERCLOUD_ANALYZER);
	pi_lua_settable(l, "HULL_AUTOREPAIR",       Equip::HULL_AUTOREPAIR);
	pi_lua_settable(l, "SHIELD_ENERGY_BOOSTER", Equip::SHIELD_ENERGY_BOOSTER);

	pi_lua_settable(l, "DRIVE_CLASS1", Equip::DRIVE_CLASS1);
	pi_lua_settable(l, "DRIVE_CLASS2", Equip::DRIVE_CLASS2);
	pi_lua_settable(l, "DRIVE_CLASS3", Equip::DRIVE_CLASS3);
	pi_lua_settable(l, "DRIVE_CLASS4", Equip::DRIVE_CLASS4);
	pi_lua_settable(l, "DRIVE_CLASS5", Equip::DRIVE_CLASS5);
	pi_lua_settable(l, "DRIVE_CLASS6", Equip::DRIVE_CLASS6);
	pi_lua_settable(l, "DRIVE_CLASS7", Equip::DRIVE_CLASS7);
	pi_lua_settable(l, "DRIVE_CLASS8", Equip::DRIVE_CLASS8);
	pi_lua_settable(l, "DRIVE_CLASS9", Equip::DRIVE_CLASS9);
	pi_lua_settable(l, "DRIVE_MIL1",   Equip::DRIVE_MIL1);
	pi_lua_settable(l, "DRIVE_MIL2",   Equip::DRIVE_MIL2);
	pi_lua_settable(l, "DRIVE_MIL3",   Equip::DRIVE_MIL3);
	pi_lua_settable(l, "DRIVE_MIL4",   Equip::DRIVE_MIL4);

	pi_lua_settable(l, "PULSECANNON_1MW",       Equip::PULSECANNON_1MW);
	pi_lua_settable(l, "PULSECANNON_DUAL_1MW",  Equip::PULSECANNON_DUAL_1MW);
	pi_lua_settable(l, "PULSECANNON_2MW",       Equip::PULSECANNON_2MW);
	pi_lua_settable(l, "PULSECANNON_RAPID_2MW", Equip::PULSECANNON_RAPID_2MW);
	pi_lua_settable(l, "PULSECANNON_4MW",       Equip::PULSECANNON_4MW);
	pi_lua_settable(l, "PULSECANNON_10MW",      Equip::PULSECANNON_10MW);
	pi_lua_settable(l, "PULSECANNON_20MW",      Equip::PULSECANNON_20MW);
	pi_lua_settable(l, "MININGCANNON_17MW",     Equip::MININGCANNON_17MW);
	pi_lua_settable(l, "SMALL_PLASMA_ACCEL",    Equip::SMALL_PLASMA_ACCEL);
	pi_lua_settable(l, "LARGE_PLASMA_ACCEL",    Equip::LARGE_PLASMA_ACCEL);

	pi_lua_settable(l, "FIRST_COMMODITY", Equip::FIRST_COMMODITY);
	pi_lua_settable(l, "LAST_COMMODITY",  Equip::LAST_COMMODITY);
	pi_lua_settable(l, "FIRST_SHIPEQUIP", Equip::FIRST_SHIPEQUIP);
	pi_lua_settable(l, "LAST_SHIPEQUIP",  Equip::LAST_SHIPEQUIP);
	lua_pop(l, 1);

	lua_pop(l, 1);


	_get_named_table(l, LUA_GLOBALSINDEX, "ShipType");

    /*
     * Constants: ShipType.Tag
     *
	 *   ShipType.Tag.NONE - .
	 *   ShipType.Tag.SHIP - .
	 *   ShipType.Tag.STATIC_SHIP - .
	 *   ShipType.Tag.MISSILE - .
	 */
	_get_named_table(l, -1, "Tag");
	pi_lua_settable(l, "NONE",        ShipType::TAG_NONE);
	pi_lua_settable(l, "SHIP",        ShipType::TAG_SHIP);
	pi_lua_settable(l, "STATIC_SHIP", ShipType::TAG_STATIC_SHIP);
	pi_lua_settable(l, "MISSILE",     ShipType::TAG_MISSILE);
	lua_pop(l, 1);

    /*
     * Constants: ShipType.Thruster
     *
	 *   ShipType.Thruster.REVERSE - .
	 *   ShipType.Thruster.FORWARD - .
	 *   ShipType.Thruster.UP - .
	 *   ShipType.Thruster.DOWN - .
	 *   ShipType.Thruster.LEFT - .
	 *   ShipType.Thruster.RIGHT - .
	 */
	_get_named_table(l, -1, "Thruster");
	pi_lua_settable(l, "REVERSE", ShipType::THRUSTER_REVERSE);
	pi_lua_settable(l, "FORWARD", ShipType::THRUSTER_FORWARD);
	pi_lua_settable(l, "UP",      ShipType::THRUSTER_UP);
	pi_lua_settable(l, "DOWN",    ShipType::THRUSTER_DOWN);
	pi_lua_settable(l, "LEFT",    ShipType::THRUSTER_LEFT);
	pi_lua_settable(l, "RIGHT",   ShipType::THRUSTER_RIGHT);
	lua_pop(l, 1);

	lua_pop(l, 1);


	_get_named_table(l, LUA_GLOBALSINDEX, "Ship");

    /*
     * Constants: Ship.JumpStatus
     *
	 *   Ship.JumpStatus.OK - .
	 *   Ship.JumpStatus.CURRENT_SYSTEM - .
	 *   Ship.JumpStatus.NO_DRIVE - .
	 *   Ship.JumpStatus.OUT_OF_RANGE - .
	 *   Ship.JumpStatus.INSUFFICIENT_FUEL - .
	 */
	_get_named_table(l, -1, "JumpStatus");
	pi_lua_settable(l, "OK",                Ship::HYPERJUMP_OK);
	pi_lua_settable(l, "CURRENT_SYSTEM",    Ship::HYPERJUMP_CURRENT_SYSTEM);
	pi_lua_settable(l, "NO_DRIVE",          Ship::HYPERJUMP_NO_DRIVE);
	pi_lua_settable(l, "OUT_OF_RANGE",      Ship::HYPERJUMP_OUT_OF_RANGE);
	pi_lua_settable(l, "INSUFFICIENT_FUEL", Ship::HYPERJUMP_INSUFFICIENT_FUEL);
	lua_pop(l, 1);

	lua_pop(l, 1);


	LUA_DEBUG_END(l, 0);
}
