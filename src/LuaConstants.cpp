#include "LuaConstants.h"
#include "LuaUtils.h"

#include "enum_table.h"

/*
 * Namespace: Constants
 *
 * Various constants exported from the game engine to the Lua environment.
 *
 * The Pioneer Lua environment makes use of "stringy" constants. That is, a
 * constant value is always represented by a string, never by a number.
 *
 * Methods that take a constant will usually not require that the constant's
 * namespace be supplied. The correct namespace will be inferred based on the
 * purpose of the called method.
 *
 * If you ever need a list of all the constants in a namespace, you can look
 * inside the global <Constants> table and pull out a table of constants for
 * the wanted namespace. For example, to get a list of valid body types, you
 * might try the following:
 *
 * > for (i,constant) in pairs(Constants.BodyType) do
 * >     print(constant)
 * > end
 */

int LuaConstants::GetConstant(lua_State *l, const char *ns, const char *name)
{
	lua_getfield(l, LUA_REGISTRYINDEX, "PiConstants");
	assert(lua_istable(l, -1));

	lua_pushstring(l, ns);
	lua_rawget(l, -2);
	assert(lua_istable(l, -1));

	lua_pushstring(l, name);
	lua_rawget(l, -2);
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

	lua_pushstring(l, ns);
	lua_rawget(l, -2);
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

static void _create_constant_table(lua_State *l, const char *ns, const EnumItem *c, bool consecutive)
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

	int value = 0;
	for (; c->name; c++) {
		if (! consecutive)
			value = c->value;
		pi_lua_settable(l, c->name, value);
		pi_lua_settable(l, value, c->name);
		++value;

		lua_pushinteger(l, lua_objlen(l, -3)+1);
		lua_pushstring(l, c->name);
		lua_rawset(l, -5);
	}

	lua_pop(l, 4);

	LUA_DEBUG_END(l, 0);
}

static void _create_constant_table_nonconsecutive(lua_State *l, const char *ns, const EnumItem *c)
{
	_create_constant_table(l, ns, c, false);
}

#if 0 // no longer used, since enum tables are auto-generated and always have their values filled in
static void _create_constant_table_consecutive(lua_State *l, const char *ns, const EnumItem *c)
{
	_create_constant_table(l, ns, c, true);
}
#endif

void LuaConstants::Register(lua_State *l)
{
	LUA_DEBUG_START(l);


	/*
	 * Constants: BodyType
	 *
	 * Describe different kinds of system bodies such as stars, planets and
	 * space stations.
	 *
	 * GRAVPOINT - a pseudo-type for a gravitational point that multiple
	 *             bodies may orbit
	 * BROWN_DWARF - brown dwarf sub-stellar object
	 * STAR_M - type 'M' red star
	 * STAR_K - type 'K' orange star
	 * STAR_G - type 'G' yellow star
	 * STAR_F - type 'F' white star
	 * STAR_A - type 'A' hot white star
	 * STAR_B - type 'B' blue star
	 * STAR_O - type 'O' hot blue star
	 * WHITE_DWARF - white dwarf stellar remnant
	 * STAR_M_GIANT - red giant star
	 * STAR_K_GIANT - orange giant star
	 * STAR_G_GIANT - yellow giant star
	 * STAR_F_GIANT - white giant star
	 * STAR_A_GIANT - hot white giant star
	 * STAR_B_GIANT - blue giant star
	 * STAR_O_GIANT - hot blue giant star
	 * STAR_M_SUPER_GIANT - red supergiant star
	 * STAR_K_SUPER_GIANT - orange supergiant star
	 * STAR_G_SUPER_GIANT - yellow supergiant star
	 * STAR_F_SUPER_GIANT - white supergiant star
	 * STAR_A_SUPER_GIANT - hot white supergiant star
	 * STAR_B_SUPER_GIANT - blue supergiant star
	 * STAR_O_SUPER_GIANT - hot blue supergiant star
	 * STAR_M_HYPER_GIANT - red hypergiant star
	 * STAR_K_HYPER_GIANT - orange hypergiant star
	 * STAR_G_HYPER_GIANT - yellow hypergiant star
	 * STAR_F_HYPER_GIANT - white hypergiant star
	 * STAR_A_HYPER_GIANT - hot white hypergiant star
	 * STAR_B_HYPER_GIANT - blue hypergiant star
	 * STAR_O_HYPER_GIANT - hot blue hypergiant star
	 * STAR_M_WF - Wolf-Rayet star (unstable)
	 * STAR_B_WF - Wolf-Rayet star (risk of collapse)
	 * STAR_O_WF - Wolf-Rayet star (imminent collapse)
	 * STAR_S_BH - stellar black hole
	 * STAR_IM_BH - intermediate-mass black hole
	 * STAR_SM_BH - supermassive black hole
	 * PLANET_GAS_GIANT - gas giant
	 * PLANET_ASTEROID - asteroid
	 * PLANET_TERRESTRIAL - terrestrial planet
	 * STARPORT_ORBITAL - orbital starport (space station)
	 * STARPORT_SURFACE - surface starport
	 *
	 * Availability:
	 *
	 *   alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 */
	_create_constant_table_nonconsecutive(l, "BodyType", ENUM_BodyType);


	/*
	 * Constants: BodySuperType
	 *
	 * Describe general categories of system bodies.
	 *
	 * NONE - uncategorised
	 * STAR - star
	 * ROCKY_PLANET - a solid planet (terrestrial or asteroid)
	 * GAS_GIANT - gas giant
	 * STARPORT - surface or orbital starport
	 *
	 * Availability:
	 *
	 *   alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 */
	_create_constant_table_nonconsecutive(l, "BodySuperType", ENUM_BodySuperType);


	/*
	 * Constants: PolitCrime
	 *
	 * Crimes
	 *
	 * TRADING_ILLEGAL_GOODS - .
	 * WEAPON_DISCHARGE - .
	 * PIRACY - .
	 * MURDER - .
	 *
	 * Availability:
	 *
	 *   alpha 10
	 *
	 * Status:
	 *
	 *   experimental
	 */
	_create_constant_table_nonconsecutive(l, "PolitCrime", ENUM_PolitCrime);


	/*
	 * Constants: PolitBloc
	 *
	 * Political alignment
	 *
	 * NONE - independent
	 * EARTHFED - Federation
	 * CIS - Confederation of Independent Systems
	 * EMPIRE - Empire
	 *
	 * Availability:
	 *
	 *   alpha 10
	 *
	 * Status:
	 *
	 *   experimental
	 */
	_create_constant_table_nonconsecutive(l, "PolitBloc", ENUM_PolitBloc);

	/*
	 * Constants: PolitEcon
	 *
	 * Economy type
	 *
	 * NONE - .
	 * VERY_CAPITALIST - .
	 * CAPITALIST - .
	 * MIXED - .
	 * PLANNED - .
	 *
	 * Availability:
	 *
	 *   alpha 10
	 *
	 * Status:
	 *
	 *   experimental
	 */
	_create_constant_table_nonconsecutive(l, "PolitEcon", ENUM_PolitEcon);

	/*
	 * Constants: PolitGovType
	 *
	 * Government type
	 *
	 * NONE - .
	 * EARTHCOLONIAL - .
	 * EARTHDEMOC - .
	 * EMPIRERULE - .
	 * CISLIBDEM - .
	 * CISSOCDEM - .
	 * LIBDEM - .
	 * CORPORATE - .
	 * SOCDEM - .
	 * EARTHMILDICT - .
	 * MILDICT1 - .
	 * MILDICT2 - .
	 * EMPIREMILDICT - .
	 * COMMUNIST - .
	 * PLUTOCRATIC - .
	 * DISORDER - .
	 *
	 * Availability:
	 *
	 *   alpha 10
	 *
	 * Status:
	 *
	 *   experimental
	 */
	_create_constant_table_nonconsecutive(l, "PolitGovType", ENUM_PolitGovType);


	/*
	 * Constants: EquipSlot
	 *
	 * Equipment slots. Every equipment or cargo type has a corresponding
	 * "slot" that it fits in to. Each slot has an independent capacity.
	 *
	 * CARGO - any cargo (commodity) item
	 * ENGINE - hyperdrives and military drives
	 * LASER - lasers and plasma accelerators
	 * MISSILE - missile
	 * ECM - ECM system
	 * SCANNER - scanner
	 * RADARMAPPER - radar mapper
	 * HYPERCLOUD - hyperspace cloud analyser
	 * HULLAUTOREPAIR - hull auto-repair system
	 * ENERGYBOOSTER - shield energy booster unit
	 * ATMOSHIELD - atmospheric shielding
	 * CABIN - cabin
	 * SHIELD - shield
	 * FUELSCOOP - fuel scoop
	 * CARGOSCOOP - cargo scoop
	 * LASERCOOLER - laser cooling booster
	 * CARGOLIFESUPPORT - cargo bay life support
	 * AUTOPILOT - autopilot
	 *
	 * Availability:
	 *
	 *   alpha 10
	 *
	 * Status:
	 *
	 *   experimental
	 */
	_create_constant_table_nonconsecutive(l, "EquipSlot", ENUM_EquipSlot);

	/*
	 * Constants: EquipType
	 *
	 * Equipment and cargo types. Because of the slot arrangement described
	 * under <EquipType> means that cargo is treated as a special type of
	 * equipment.
	 *
	 * NONE - no equipment. Usually used to indicate the absence of equipment
	 * HYDROGEN - hydrogen (CARGO)
	 * LIQUID_OXYGEN - liquid oxygen (CARGO)
	 * METAL_ORE - metal ore (CARGO)
	 * CARBON_ORE - carbon ore (CARGO)
	 * METAL_ALLOYS - metal alloys (CARGO)
	 * PLASTICS - plastics (CARGO)
	 * FRUIT_AND_VEG - fruit and vegetables (CARGO)
	 * ANIMAL_MEAT - animal meat (CARGO)
	 * LIVE_ANIMALS - live animals (CARGO)
	 * LIQUOR - liquor (CARGO)
	 * GRAIN - grain (CARGO)
	 * TEXTILES - textiles (CARGO)
	 * FERTILIZER - fertilizer (CARGO)
	 * WATER - water (CARGO)
	 * MEDICINES - medicines (CARGO)
	 * CONSUMER_GOODS - consumer goods (CARGO)
	 * COMPUTERS - computers (CARGO)
	 * ROBOTS - robots (CARGO)
	 * PRECIOUS_METALS - precious metals (CARGO)
	 * INDUSTRIAL_MACHINERY - industrial machinery (CARGO)
	 * FARM_MACHINERY - farm machinery (CARGO)
	 * MINING_MACHINERY - mining machinery (CARGO
	 * AIR_PROCESSORS - air processors (CARGO)
	 * SLAVES - slaves (CARGO)
	 * HAND_WEAPONS - hand weapons (CARGO)
	 * BATTLE_WEAPONS - battle weapons (CARGO)
	 * NERVE_GAS - nerve gas (CARGO)
	 * NARCOTICS - narcotics (CARGO)
	 * MILITARY_FUEL - military fuel (CARGO)
	 * RUBBISH - rubbish (CARGO)
	 * RADIOACTIVES - radioactives (CARGO)
	 * MISSILE_UNGUIDED - unguided rocket (MISSILE)
	 * MISSILE_GUIDED - guided missile (MISSILE)
	 * MISSILE_SMART - smart missile (MISSILE)
	 * MISSILE_NAVAL - naval missile (MISSILE)
	 * ATMOSPHERIC_SHIELDING - atmospheric shielding (ATMOSHIELD)
	 * ECM_BASIC - basic ECM system (ECM)
	 * SCANNER - scanner (SCANNER)
	 * ECM_ADVANCED - advanced ECM system (ECM)
	 * UNOCCUPIED_CABIN - unoccupied passenger cabin (CABIN)
	 * PASSENGER_CABIN - occupied passenger cabin (CABIN)
	 * SHIELD_GENERATOR - shield generator (SHIELD)
	 * LASER_COOLING_BOOSTER - laser cooling booster (LASERCOOLER)
	 * CARGO_LIFE_SUPPORT - cargo bay life support (CARGOLIFESUPPORT)
	 * AUTOPILOT - autopilot (AUTOPILOT)
	 * RADAR_MAPPER - radar mapper (RADARMAPPER)
	 * FUEL_SCOOP - fuel scoop (FUELSCOOP)
	 * CARGO_SCOOP - cargo scoop (CARGOSCOOP)
	 * HYPERCLOUD_ANALYZER - hyperspace cloud analyser (HYPERCLOUD)
	 * HULL_AUTOREPAIR - hull auto-repair system (HULLAUTOREPAIR)
	 * SHIELD_ENERGY_BOOSTER - shield energy booster unit (ENERGYBOOSTER)
	 * DRIVE_CLASS1 - class 1 hyperdrive (ENGINE)
	 * DRIVE_CLASS2 - class 2 hyperdrive (ENGINE)
	 * DRIVE_CLASS3 - class 3 hyperdrive (ENGINE)
	 * DRIVE_CLASS4 - class 4 hyperdrive (ENGINE)
	 * DRIVE_CLASS5 - class 5 hyperdrive (ENGINE)
	 * DRIVE_CLASS6 - class 6 hyperdrive (ENGINE)
	 * DRIVE_CLASS7 - class 7 hyperdrive (ENGINE)
	 * DRIVE_CLASS8 - class 8 hyperdrive (ENGINE)
	 * DRIVE_CLASS9 - class 9 hyperdrive (ENGINE)
	 * DRIVE_MIL1 - class 1 military drive (ENGINE)
	 * DRIVE_MIL2 - class 2 military drive (ENGINE)
	 * DRIVE_MIL3 - class 3 military drive (ENGINE)
	 * DRIVE_MIL4 - class 4 military drive (ENGINE)
	 * PULSECANNON_1MW - 1MW pulse cannon (LASER)
	 * PULSECANNON_DUAL_1MW - 1MW dual-fire pulse cannon (LASER)
	 * PULSECANNON_2MW - 2MW pulse cannon (LASER)
	 * PULSECANNON_RAPID_2MW - 2MW rapid-fire pulse cannon (LASER)
	 * PULSECANNON_4MW - 4MW pulse cannon (LASER)
	 * PULSECANNON_10MW - 10MW pulse cannon (LASER)
	 * PULSECANNON_20MW - 20MW pulse cannon (LASER)
	 * MININGCANNON_17MW - 17MW blast-mining cannon (LASER)
	 * SMALL_PLASMA_ACCEL - small plasma accelerator (LASER)
	 * LARGE_PLASMA_ACCEL - large plasma accelerator (LASER)
	 *
	 * Availability:
	 *
	 *   alpha 10
	 *
	 * Status:
	 *
	 *   experimental
	 */
	_create_constant_table_nonconsecutive(l, "EquipType", ENUM_EquipType);


	/*
	 * Constants: ShipTypeTag
	 *
	 * Ship tags mark whether a ship is suitable for a particular use. Used
	 * with <ShipType.GetShipTypes> to select which set of ships to search.
	 *
	 * NONE - no tags
	 * SHIP - standard ships. These are the ones available to the player and
	 *        used for regular game functions (trade, combat, etc)
	 * STATIC_SHIP - static ships. These are not available to the player and
	 *               are used for mission specific functions (large supply
	 *               ships, warships, etc)
	 * MISSILE - missiles. Correspond directly to the <EquipType> constants of
	 *           the same name.
	 *
	 * Availability:
	 *
	 *   alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 */
	_create_constant_table_nonconsecutive(l, "ShipTypeTag", ENUM_ShipTypeTag);

	/*
	 * Constants: ShipTypeThruster
	 *
	 * Thruster types. Used by <ShipType.GetLinearThrust>. The name
	 * corresponds to the direction the ship moves when the thruster is fired.
	 *
	 * REVERSE - front (fore) thruster
	 * FORWARD - main/rear (aft) thruster
	 * UP - bottom/underbelly (ventral) thruster
	 * DOWN - top/back (dorsal) thruster
	 * LEFT - right-side (starboard) thruster
	 * RIGHT - left-side (port) thruster
	 *
	 * Availability:
	 *
	 *   alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 */
	_create_constant_table_nonconsecutive(l, "ShipTypeThruster", ENUM_ShipTypeThruster);


	/*
	 * Constants: ShipJumpStatus
	 *
	 * Reasons that that a hyperspace jump might succeed or fail. Returned by
	 * <Ship.HyperspaceTo> and <Ship.CanHyperspaceTo>.
	 *
	 * OK - jump successful
	 * CURRENT_SYSTEM - ship is already in the target system
	 * NO_DRIVE - ship has no drive
	 * OUT_OF_RANGE - target system is out of range
	 * INSUFFICIENT_FUEL - target system is in range but the ship doesn't have
	 *                     enough fuel
	 *
	 * Availability:
	 *
	 *   alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 */
	_create_constant_table_nonconsecutive(l, "ShipJumpStatus", ENUM_ShipJumpStatus);

	/*
	 * Constants: ShipAlertStatus
	 *
	 * Current alert status. Based on proximity and actions of nearby ships.
	 *
	 * NONE - no alert. All is well (green)
	 * SHIP_NEARBY - ship within 100km (yellow)
	 * SHIP_FIRING - ship within 100km is firing lasers (though not
	 * necessarily at us) (red)
	 *
	 * Availability:
	 *
	 *   alpha 10
	 *
	 * Status:
	 *
	 *   experimental
	 */
	_create_constant_table_nonconsecutive(l, "ShipAlertStatus", ENUM_ShipAlertStatus);

	/*
	 * Constants: ShipFuelStatus
	 *
	 * Current fuel status.
	 *
	 * OK - more than 5% fuel remaining
	 * WARNING - less than 5% fuel remaining
	 * EMPTY - no fuel remaining
	 *
	 * Availability:
	 *
	 *   alpha 20
	 *
	 * Status:
	 *
	 *   experimental
	 */
	_create_constant_table_nonconsecutive(l, "ShipFuelStatus", ENUM_ShipFuelStatus);

	/*
	 * Constants: ShipFlightState
	 *
	 * Ship flight state (used by LMR)
	 *
	 * FLYING     - open flight (includes autopilot)
	 * DOCKING    - in docking animation
	 * DOCKED     - docked with station
	 * LANDED     - rough landed (not docked)
	 * HYPERSPACE - in hyperspace
	 *
	 * Availability:
	 *
	 *   alpha 16
	 *
	 * Status:
	 *
	 *   experimental
	 */
	_create_constant_table_nonconsecutive(l, "ShipFlightState", ENUM_ShipFlightState);

	/*
	 * Constants: ShipAIError
	 *
	 * AI command error/result code passed to EventQueue.onAICompleted
	 *
	 * NONE             - AI completed successfully
	 * GRAV_TOO_HIGH    - AI can not compensate for gravity
	 * REFUSED_PERM     - AI was refused docking permission
	 * ORBIT_IMPOSSIBLE - AI was asked to enter an impossible orbit (orbit is
	 *                    outside target's frame)
	 *
	 * Availability:
	 *
	 *   alpha 17
	 *
	 * Status:
	 *
	 *   experimental
	 */
	_create_constant_table_nonconsecutive(l, "ShipAIError", ENUM_ShipAIError);

	/*
	 * Constants: ShipAnimation
	 *
	 * Animation code used by LMR. Pass one of these constants to
	 * get_animation_stage() or get_animation_position() in a model script.
	 *
	 * WHEEL_STATE  - animation state unused; position gives position of undercarriage
	 *
	 * Availability:
	 *
	 *   alpha 16
	 *
	 * Status:
	 *
	 *   experimental
	 */
	_create_constant_table_nonconsecutive(l, "ShipAnimation", ENUM_ShipAnimation);

	/*
	 * Constants: SpaceStationAnimation
	 *
	 * Animation code used by LMR. Pass one of these constants to
	 * get_animation_stage() or get_animation_position() in a model script.
	 *
	 * WHEEL_STATE  - animation state unused; position gives position of undercarriage
	 *
	 * Availability:
	 *
	 *   alpha 16
	 *
	 * Status:
	 *
	 *   experimental
	 */
	_create_constant_table_nonconsecutive(l, "SpaceStationAnimation", ENUM_SpaceStationAnimation);

    /*
     * Constants: MissionStatus
     *
     * Status of a mission.
     *
     * ACTIVE - mission in progress
     * COMPLETED - mission completed
     * FAILED - mission failed
     *
	 * Availability:
	 *
	 *   alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 */
	_create_constant_table_nonconsecutive(l, "MissionStatus", ENUM_MissionStatus);


	LUA_DEBUG_END(l, 0);
}
