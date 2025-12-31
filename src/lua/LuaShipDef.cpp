// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaShipDef.h"
#include "EnumStrings.h"
#include "JsonUtils.h"
#include "Lua.h"
#include "LuaJson.h"
#include "LuaUtils.h"
#include "ShipType.h"

/*
 * Class: ShipDef
 *
 * Class for a description of a type of ship.
 */

/*
 * Attribute: name
 *
 * The name of the ship type
 */

/*
 * Attribute: i18n
 *
 * The capitalized ship name that pairs with a suffix to address a ship in
 * it's correct singular form in the translated names in /data/lang/ships.
 *
 * SHIP			- ship, base form.
 * SHIP_DEF		- the ship, definitive form.
 * SHIP_INDEF	- a ship, indefinite form.
 *
 * Example:
 *
 * > local ship = ShipDef[ad.shipid].i18n_key            -- 'NATRIX'
 * >
 * > local ship_def = ls[ship .. "_DEF"],                -- 'NATRIX_DEF' - 'the Natrix'
 * > print("We're counting on " .. ship_def .. " to give us some resistance!")
 * >
 * > local ship_indef = ls[ship .. "_INDEF"],            -- 'NATRIX_INDEF' - 'a Natrix'
 * > local body = ad.location:GetSystemBody()
 * > print("There are rumours of an abandoned " .. ship_undef ..
 * > ", drifting in a close orbit around " .. body .. ".")
 */

/*
 * Attribute: angularThrust
 *
 * The amount of angular thrust this ship can achieve. This is the value
 * responsible for the rate that the ship can turn at.
 */

/*
 * Attribute: capacity
 *
 * The maximum space available for cargo and equipment, in tonnes
 */

/*
 * Attribute: hullMass
 *
 * The total mass of the ship's hull, independent of any equipment or cargo
 * inside it, in tonnes. This is the value used when calculating hyperjump
 * ranges and hull damage.
 */

/*
 * Attribute: basePrice
 *
 * The base price of the ship. This typically receives some adjustment before
 * being used as a buy or sell price (eg based on supply or demand)
 */

/*
 * Attribute: minCrew
 *
 * Minimum number of crew required to launch.
 */

/*
 * Attribute: maxCrew
 *
 * Maximum number of crew the ship can carry.
 */

/*
 * Attribute: hyperdriveClass
 *
 * An integer representing the power of the hyperdrive usually installed on
 * those ships. If zero, it means the ship usually isn't equipped with one,
 * although this does not necessarily mean one cannot be installed.
 */

/*
 * Attribute: linearThrust
 *
 * Table keyed on <Constants.ShipTypeThruster>, containing linear thrust of
 * that thruster in newtons
 */

/*
 * Attribute: linAccelerationCap
 *
 * Table keyed on <Constants.ShipTypeThruster>, containing acceleration cap of
 * that thruster direction in m/s/s
 */

/*
 * Attribute: effectiveExhaustVelocity
 *
 * Ship thruster efficiency as the effective exhaust velocity in m/s.
 * See http://en.wikipedia.org/wiki/Specific_impulse for an explanation of this value.
 */

/*
 * Attribute: thrusterFuelUse
 *
 * Ship thruster efficiency as a percentage-of-tank-used per second of thrust.
 */

/*
 * Attribute: shipClass
 *
 * Class of the ship (e.g. "medium_courier").
 */

/*
 * Attribute: manufacturer
 *
 * Manufacturer of the ship (e.g. "kaluri").
 */

/*
 * Attribute: modelName
 *
 * Name for the model of this ship. Important for looking up the actual ship model (Engine.GetModel(ShipDef.modelName)).
 */

/*
 * Attribute: shieldModelName
 *
 * Name for the shield model of this ship. Can be useful for debug purposes to determine the shield model used by the ship.
 */

void LuaShipDef::Register()
{
	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);

	lua_newtable(l);

	for (auto iter : ShipType::types) {
		const ShipType &st = iter.second;
		lua_newtable(l);

		pi_lua_settable(l, "id", iter.first.c_str());
		pi_lua_settable(l, "path", st.definitionPath.c_str());
		pi_lua_settable(l, "name", st.name.c_str());
		pi_lua_settable(l, "i18n_key", st.i18n_key.c_str());
		pi_lua_settable(l, "shipClass", st.shipClass.c_str());
		pi_lua_settable(l, "manufacturer", st.manufacturer.c_str());
		pi_lua_settable(l, "modelName", st.modelName.c_str());
		pi_lua_settable(l, "shieldModelName", st.shieldName.c_str());
		pi_lua_settable(l, "cockpitName", st.cockpitName.c_str());
		pi_lua_settable(l, "tag", EnumStrings::GetString("ShipTypeTag", st.tag));
		pi_lua_settable(l, "angularThrust", st.angThrust);
		pi_lua_settable(l, "equipCapacity", st.capacity);
		pi_lua_settable(l, "cargo", st.cargo);
		pi_lua_settable(l, "hullMass", st.hullMass);
		pi_lua_settable(l, "fuelTankMass", st.fuelTankMass);
		pi_lua_settable(l, "basePrice", st.baseprice);
		pi_lua_settable(l, "minCrew", st.minCrew);
		pi_lua_settable(l, "maxCrew", st.maxCrew);
		pi_lua_settable(l, "hyperdriveClass", st.hyperdriveClass);
		pi_lua_settable(l, "effectiveExhaustVelocity", st.effectiveExhaustVelocity);
		pi_lua_settable(l, "thrusterFuelUse", st.GetFuelUseRate());
		pi_lua_settable(l, "frontCrossSec", st.frontCrossSection);
		pi_lua_settable(l, "sideCrossSec", st.sideCrossSection);
		pi_lua_settable(l, "topCrossSec", st.topCrossSection);
		pi_lua_settable(l, "atmosphericPressureLimit", st.atmosphericPressureLimit);

		lua_newtable(l);
		for (int t = Thruster::THRUSTER_REVERSE; t < Thruster::THRUSTER_MAX; t++)
			pi_lua_settable(l, EnumStrings::GetString("ShipTypeThruster", t), st.linThrust[t]);
		pi_lua_readonly_table_proxy(l, -1);
		lua_setfield(l, -3, "linearThrust");
		lua_pop(l, 1);

		lua_newtable(l);
		for (int t = Thruster::THRUSTER_REVERSE; t < Thruster::THRUSTER_MAX; t++)
			pi_lua_settable(l, EnumStrings::GetString("ShipTypeThruster", t), st.linAccelerationCap[t]);
		pi_lua_readonly_table_proxy(l, -1);
		lua_setfield(l, -3, "linAccelerationCap");
		lua_pop(l, 1);

		// Set up roles table
		lua_newtable(l);
		for (auto it = st.roles.cbegin(); it != st.roles.cend(); ++it) {
			pi_lua_settable(l, it->first.c_str(), it->second);
		}
		pi_lua_readonly_table_proxy(l, -1);
		lua_setfield(l, -3, "roles");
		lua_pop(l, 1);

		Json data = JsonUtils::LoadJsonDataFile(st.definitionPath);
		LuaJson::PushToLua(l, data);
		lua_setfield(l, -2, "raw");

		pi_lua_readonly_table_proxy(l, -1);
		lua_setfield(l, -3, iter.first.c_str());
		lua_pop(l, 1);
	}

	lua_getfield(l, LUA_REGISTRYINDEX, "CoreImports");
	pi_lua_readonly_table_proxy(l, -2);
	lua_setfield(l, -2, "ShipDef");
	lua_pop(l, 2);

	LUA_DEBUG_END(l, 0);
}
