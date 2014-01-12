// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Lua.h"
#include "LuaShipDef.h"
#include "LuaUtils.h"
#include "EnumStrings.h"
#include "ShipType.h"
#include "EquipType.h"

/*
 * Class: ShipDef
 *
 * Class for a description of a type of ship.
 */

/*
 * Attribute: name
 *
 * The name of the ship type
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
 * Attribute: angularThrust
 *
 * The amount of angular thrust this ship can achieve. This is the value
 * responsible for the rate that the ship can turn at.
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
 * Attribute: capacity
 *
 * The maximum space available for cargo and equipment, in tonnes
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
 * Attribute: hullMass
 *
 * The total mass of the ship's hull, independent of any equipment or cargo
 * inside it, in tonnes. This is the value used when calculating hyperjump
 * ranges and hull damage.
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
 * Attribute: basePrice
 *
 * The base price of the ship. This typically receives some adjustment before
 * being used as a buy or sell price (eg based on supply or demand)
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
 * Attribute: minCrew
 *
 * Minimum number of crew required to launch.
 *
 * Availability:
 *
 *   alpha 30
 *
 * Status:
 *
 *   experimental
 */

/*
 * Attribute: maxCrew
 *
 * Maximum number of crew the ship can carry.
 *
 * Availability:
 *
 *   alpha 30
 *
 * Status:
 *
 *   experimental
 */

/*
 * Attribute: defaultHyperdrive
 *
 * The default hyperdrive this ship receives. This is a <Constants.EquipType>
 * string corresponding to the appropriate drive. Not that this value is only
 * used when the player purchases a ship. Scripts using <Space.SpawnShip> etc
 * must manually add a hyperdrive to the ship; it does not get one by default.
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
 * Attribute: linearThrust
 *
 * Table keyed on <Constants.ShipTypeThruster>, containing linear thrust of
 * that thruster in newtons
 *
 * Availability:
 *
 *   alpha 32
 *
 * Status:
 *
 *   experimental
 */

/*
 * Attribute: effectiveExhaustVelocity
 *
 * Ship thruster efficiency as the effective exhaust velocity in m/s.
 * See http://en.wikipedia.org/wiki/Specific_impulse for an explanation of this value.
 *
 * Availability:
 *
 *   November 2013
 *
 * Status:
 *
 *   experimental
 */

/*
 * Attribute: thrusterFuelUse
 *
 * Ship thruster efficiency as a percentage-of-tank-used per second of thrust.
 *
 * Availability:
 *
 *   November 2013
 *
 * Status:
 *
 *   experimental
 */

/*
 * Attribute: equipSlotCapacity
 *
 * Table keyed on <Constants.EquipSlot>, containing maximum number of items
 * that can be held in that slot (ignoring mass)
 *
 * Availability:
 *
 *   alpha 32
 *
 * Status:
 *
 *   experimental
 */

void LuaShipDef::Register()
{
	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);

	lua_newtable(l);

	for (std::map<ShipType::Id,ShipType>::const_iterator i = ShipType::types.begin(); i != ShipType::types.end(); ++i)
	{
		const ShipType &st = (*i).second;
		lua_newtable(l);

		pi_lua_settable(l, "id",                (*i).first.c_str());
		pi_lua_settable(l, "name",              st.name.c_str());
		pi_lua_settable(l, "shipClass",         st.shipClass.c_str());
		pi_lua_settable(l, "manufacturer",      st.manufacturer.c_str());
		pi_lua_settable(l, "modelName",         st.modelName.c_str());
		pi_lua_settable(l, "cockpitName",		st.cockpitName.c_str());
		pi_lua_settable(l, "tag",               EnumStrings::GetString("ShipTypeTag", st.tag));
		pi_lua_settable(l, "angularThrust",     st.angThrust);
		pi_lua_settable(l, "capacity",          st.capacity);
		pi_lua_settable(l, "hullMass",          st.hullMass);
		pi_lua_settable(l, "fuelTankMass",      st.fuelTankMass);
		pi_lua_settable(l, "basePrice",         double(st.baseprice)*0.01);
		pi_lua_settable(l, "minCrew",           st.minCrew);
		pi_lua_settable(l, "maxCrew",           st.maxCrew);
		pi_lua_settable(l, "defaultHyperdrive", EnumStrings::GetString("EquipType", st.hyperdrive));
		pi_lua_settable(l, "effectiveExhaustVelocity", st.effectiveExhaustVelocity);
		pi_lua_settable(l, "thrusterFuelUse",   st.GetFuelUseRate());

		lua_newtable(l);
		for (int t = ShipType::THRUSTER_REVERSE; t < ShipType::THRUSTER_MAX; t++)
			pi_lua_settable(l, EnumStrings::GetString("ShipTypeThruster", t), st.linThrust[t]);
		pi_lua_readonly_table_proxy(l, -1);
		lua_setfield(l, -3, "linearThrust");
		lua_pop(l, 1);

		lua_newtable(l);
		for (int slot = Equip::SLOT_CARGO; slot < Equip::SLOT_MAX; slot++)
			pi_lua_settable(l, EnumStrings::GetString("EquipSlot", slot), st.equipSlotCapacity[slot]);
		pi_lua_readonly_table_proxy(l, -1);
		lua_setfield(l, -3, "equipSlotCapacity");
		lua_pop(l, 1);

		pi_lua_readonly_table_proxy(l, -1);
		lua_setfield(l, -3, (*i).first.c_str());
		lua_pop(l, 1);
	}

	lua_getfield(l, LUA_REGISTRYINDEX, "CoreImports");
	pi_lua_readonly_table_proxy(l, -2);
	lua_setfield(l, -2, "ShipDef");
	lua_pop(l, 2);

	LUA_DEBUG_END(l, 0);
}
