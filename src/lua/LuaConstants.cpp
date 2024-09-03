// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaConstants.h"
#include "LuaUtils.h"

#include "EnumStrings.h"
#include "enum_table.h"

#include <cassert>

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

int LuaConstants::GetConstantFromArg(lua_State *l, const char *ns, int idx)
{
	if (lua_type(l, idx) != LUA_TSTRING) {
		// heuristic assumption that positive (absolute) stack indexes refer to function args
		if (idx > 0) {
			const char *emsg = lua_pushfstring(l, "argument #%d is invalid (expected a constant in namespace '%s')", idx, ns);
			return luaL_argerror(l, idx, emsg);
		} else {
			return luaL_error(l, "value (stack #%d) is invalid (expected a constant in namespace '%s')", idx, ns);
		}
	}

	return GetConstant(l, ns, lua_tostring(l, idx));
}

bool LuaConstants::CheckConstantFromArg(lua_State *l, const char *ns, int idx, int *out)
{
	if (lua_type(l, idx) != LUA_TSTRING) {
		// heuristic assumption that positive (absolute) stack indexes refer to function args
		if (idx > 0) {
			const char *emsg = lua_pushfstring(l, "argument #%d is invalid (expected a constant in namespace '%s')", idx, ns);
			return luaL_argerror(l, idx, emsg);
		} else {
			return luaL_error(l, "value (stack #%d) is invalid (expected a constant in namespace '%s')", idx, ns);
		}
	}

	return CheckConstant(l, ns, lua_tostring(l, idx), out);
}

int LuaConstants::GetConstant(lua_State *l, const char *ns, const char *name)
{
	int value = EnumStrings::GetValue(ns, name);
	if (value < 0)
		luaL_error(l, "couldn't find constant with name '%s' in namespace '%s\n", name, ns);
	return value;
}

bool LuaConstants::CheckConstant(lua_State *l, const char *ns, const char *name, int *out)
{
	*out = EnumStrings::GetValue(ns, name);
	return *out >= 0;
}

static void _create_constant_table(lua_State *l, const char *ns, const EnumItem *c)
{
	LUA_DEBUG_START(l);

	lua_getglobal(l, "Constants");
	if (lua_isnil(l, -1)) {
		lua_pop(l, 1);
		lua_newtable(l);

		// publish a read-only proxy wrapper
		lua_rawgeti(l, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
		lua_pushstring(l, "Constants");
		pi_lua_readonly_table_proxy(l, -3);
		lua_rawset(l, -3);
		lua_pop(l, 1);
	} else {
		pi_lua_readonly_table_original(l, -1);
		lua_remove(l, -2);
	}
	assert(lua_istable(l, -1));

	lua_newtable(l); // 'Constants' table, enum table
	int enum_table_idx = lua_gettop(l);

	// publish a read-only proxy to the enum table
	lua_pushstring(l, ns);
	pi_lua_readonly_table_proxy(l, enum_table_idx);
	lua_rawset(l, -4);

	int index = 1;
	for (; c->name; c++) {
		lua_pushinteger(l, index);
		lua_pushstring(l, c->name);
		lua_rawset(l, enum_table_idx);
		++index;
	}

	lua_pop(l, 2);

	LUA_DEBUG_END(l, 0);
}

void LuaConstants::Register(lua_State *l)
{
	LUA_DEBUG_START(l);

	for (const EnumTable *table = ENUM_TABLES_PUBLIC; table->name; table++)
		_create_constant_table(l, table->name, table->first);

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

	/*
	 * Constants: PhysicsObjectType
	 *
	 * General physical objects
	 *
	 *   BODY - .
	 *	  MODELBODY - .
	 *	  SHIP - .
	 *	  PLAYER - .
	 *	  SPACESTATION - .
	 *	  PLANET - .
	 *	  STAR - .
	 *	  CARGOBODY - .
	 *	  MISSILE - .
	 *
	 * Availability:
	 *
	 *   2014 April
	 *
	 * Status:
	 *
	 *   experimental
	 */

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

	/*
	 * Constants: CommodityType
	 *
	 * Cargo/commodity types.
	 *
	 * HYDROGEN - hydrogen
	 * LIQUID_OXYGEN - liquid oxygen
	 * METAL_ORE - metal ore
	 * CARBON_ORE - carbon ore
	 * METAL_ALLOYS - metal alloys
	 * PLASTICS - plastics
	 * FRUIT_AND_VEG - fruit and vegetables
	 * ANIMAL_MEAT - animal meat
	 * LIVE_ANIMALS - live animals
	 * LIQUOR - liquor
	 * GRAIN - grain
	 * TEXTILES - textiles
	 * FERTILIZER - fertilizer
	 * WATER - water
	 * MEDICINES - medicines
	 * CONSUMER_GOODS - consumer goods
	 * COMPUTERS - computers
	 * ROBOTS - robots
	 * PRECIOUS_METALS - precious metals
	 * INDUSTRIAL_MACHINERY - industrial machinery
	 * FARM_MACHINERY - farm machinery
	 * MINING_MACHINERY - mining machinery (CARGO
	 * AIR_PROCESSORS - air processors
	 * SLAVES - slaves
	 * HAND_WEAPONS - hand weapons
	 * BATTLE_WEAPONS - battle weapons
	 * NERVE_GAS - nerve gas
	 * NARCOTICS - narcotics
	 * MILITARY_FUEL - military fuel
	 * RUBBISH - rubbish
	 * RADIOACTIVES - radioactives
	 *
	 * Availability:
	 *
	 *   alpha 10
	 *
	 * Status:
	 *
	 *   experimental
	 */

	/*
	 * Constants: DualLaserOrientation
	 *
	 * The orientation of dual laser mountings.
	 *
	 * HORIZONTAL - Lasers are mounted left and right
	 * VERTICAL   - Lasers are mounted top and bottom
	 *
	 * Availability:
	 *
	 *   alpha 27
	 *
	 * Status:
	 *
	 *   experimental
	 */

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
	 * MISSILE - missiles.
	 *
	 * Availability:
	 *
	 *   alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 */

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

	/*
	 * Constants: ShipJumpStatus
	 *
	 * Reasons that that a hyperspace jump might succeed or fail. Returned by
	 * <Ship.HyperspaceTo>, <Ship.CheckHyperspaceTo> and <Ship.GetHyperspaceDetails>.
	 *
	 * OK - jump successful
	 * CURRENT_SYSTEM - ship is already in the target system
	 * NO_DRIVE - ship has no drive
	 * DRIVE_ACTIVE - ship is already in hyperspace
	 * OUT_OF_RANGE - target system is out of range
	 * INSUFFICIENT_FUEL - target system is in range but the ship doesn't have
	 *                     enough fuel
	 * SAFETY_LOCKOUT - drive locked out for safety reasons
	 *                  (currently this happens if landed, docked or docking)
	 *
	 * Availability:
	 *
	 *   alpha 10
	 *
	 * Status:
	 *
	 *   stable
	 */

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

	/*
	 * Constants: PropulsionFuelStatus
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

	/*
	 * Constants: ShipFlightState
	 *
	 * Ship flight state
	 *
	 * FLYING     - open flight (includes autopilot)
	 * DOCKING    - in docking animation
	 * UNDOCKING  - in docking animation
	 * DOCKED     - docked with station
	 * LANDED     - rough landed (not docked)
	 * JUMPING    - just initiating hyperjump (as of February 2014)
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

	/*
	 * Constants: ShipAIError
	 *
	 * AI command error/result code passed to Event.onAICompleted
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

	/*
	 * Constants: FileSystemRoot
	 *
	 * Specifier of filesystem base.
	 *
	 * USER - directory containing Pioneer's config, savefiles, mods and other
	 * user-specific data
	 * DATA - directory containing Pioneer's data files
	 *
	 * Availability:
	 *
	 *   alpha 25
	 *
	 * Status:
	 *
	 *   experimental
	 */

	// XXX document UI tables

	LUA_DEBUG_END(l, 0);
}
