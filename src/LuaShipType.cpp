#include "LuaObject.h"
#include "LuaShipType.h"
#include "LuaUtils.h"
#include "LuaConstants.h"
#include "ShipType.h"
#include "EquipType.h"

/*
 * Class: ShipType
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
int l_shiptype_attr_name(lua_State *l)
{
	const ShipType *st = LuaShipType::GetFromLua(1);
	lua_pushstring(l, st->name.c_str());
	return 1;
}

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
int l_shiptype_attr_angular_thrust(lua_State *l)
{
	const ShipType *st = LuaShipType::GetFromLua(1);
	lua_pushnumber(l, st->angThrust);
	return 1;
}

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
int l_shiptype_attr_capacity(lua_State *l)
{
	const ShipType *st = LuaShipType::GetFromLua(1);
	lua_pushinteger(l, st->capacity);
	return 1;
}

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
int l_shiptype_attr_hull_mass(lua_State *l)
{
	const ShipType *st = LuaShipType::GetFromLua(1);
	lua_pushinteger(l, st->hullMass);
	return 1;
}

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
int l_shiptype_attr_base_price(lua_State *l)
{
	const ShipType *st = LuaShipType::GetFromLua(1);
	lua_pushnumber(l, double(st->baseprice) * 0.01);
	return 1;
}

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
int l_shiptype_attr_default_hyperdrive(lua_State *l)
{
	const ShipType *st = LuaShipType::GetFromLua(1);
	lua_pushstring(l, LuaConstants::GetConstantString(l, "EquipType", st->hyperdrive));
	return 1;
}

/*
 * Method: GetLinearThrust
 *
 * Gets the linear thrust of a given thruster.
 *
 * > thrust = shiptype:GetLinearThrust(thruster)
 *
 * Parameters:
 *
 *   thruster - a <Constants.ShipTypeThruster> string for the wanted thruster
 *
 * Returns:
 *
 *   thrust - maximum thrust, in newtons
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   experimental
 */
int l_shiptype_get_linear_thrust(lua_State *l)
{
	const ShipType *st = LuaShipType::GetFromLua(1);
	ShipType::Thruster t = static_cast<ShipType::Thruster>(LuaConstants::GetConstant(l, "ShipTypeThruster", luaL_checkstring(l, 2)));
	lua_pushnumber(l, st->linThrust[t]);
	return 1;
}

/*
 * Method: GetEquipSlotCapacity
 *
 * Get the maximum number of a particular type of equipment this ship can
 * hold. This is the number of items that can be held, not the mass.
 * <Ship.AddEquip> will take care of ensuring the hull capacity is not
 * exceeded.
 *
 * > capacity = shiptype:GetEquipSlotCapacity(slot)
 *
 * Parameters:
 *
 *   slot - a <Constants.EquipSlot> string for the wanted equipment type
 *
 * Returns:
 *
 *   capacity - the maximum capacity of the equipment slot
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   experimental
 */
int l_shiptype_get_equip_slot_capacity(lua_State *l)
{
	const ShipType *st = LuaShipType::GetFromLua(1);
	Equip::Slot slot = static_cast<Equip::Slot>(LuaConstants::GetConstant(l, "EquipSlot", luaL_checkstring(l, 2)));
	lua_pushnumber(l, st->equipSlotCapacity[slot]);
	return 1;
}

/*
 * Function: GetShipType
 *
 * Get a description object for the given ship id
 *
 * > shiptype = ShipType.GetShipType(id)
 *
 * Parameters:
 *
 *   id - the id of the ship to get the description object for
 *
 * Example:
 *
 * > local shiptype = ShipType.GetShipType("eagle_lrf")
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   stable
 */
static int l_shiptype_get_ship_type(lua_State *l)
{
	const char *type = luaL_checkstring(l, 1);

	std::map<ShipType::Type,ShipType>::iterator i = ShipType::types.find(type);
	if (i == ShipType::types.end())
		luaL_error(l, "Invalid ship name '%s'", type);

	LuaShipType::PushToLua(&((*i).second));
	return 1;
}

/*
 * Function: GetShipTypes
 *
 * Returns an array of ship description objects that match the specified
 * filter
 *
 * > shiptypes = ShipType.GetShipTypes(tag, filter)
 *
 * Parameters:
 *
 *   tag - a <Constants.ShipTypeTag> to select the ship group to search. Using
 *         "NONE" will check all ships.
 *
 *   filter - an optional function. If specified the function will be called
 *            once for each ship type with the description object as the only
 *            parameter. If the filter function returns true then the
 *            ship name will be included in the array returned by
 *            <GetShipTypes>, otherwise it will be omitted. If no filter
 *            function is specified then all ships are returned.
 *
 * Returns:
 *
 *   shiptypes - an array containing zero or more ship names for which the
 *               relevant description object matched the filter
 *
 * Example:
 *
 * > local shiptypes = ShipType.GetShipTypes('SHIP', function (t)
 * >     local mass = t.hullMass
 * >     return mass >= 50 and mass <= 150
 * > end)
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   stable
 */
static int l_shiptype_get_ship_types(lua_State *l)
{
	LUA_DEBUG_START(l);

	ShipType::Tag tag = ShipType::TAG_NONE;

	if (lua_gettop(l) >= 1)
		tag = static_cast<ShipType::Tag>(LuaConstants::GetConstant(l, "ShipTypeTag", luaL_checkstring(l, 1)));

	bool filter = false;
	if (lua_gettop(l) >= 2) {
		luaL_checktype(l, 2, LUA_TFUNCTION); // any type of function
		filter = true;
	}

	lua_newtable(l);

	for (std::map<ShipType::Type,ShipType>::iterator i = ShipType::types.begin(); i != ShipType::types.end(); ++i)
	{
		ShipType *st = &((*i).second);
		if (tag == ShipType::TAG_NONE || tag == st->tag) {
			if (filter) {
				lua_pushvalue(l, 2);
				LuaShipType::PushToLua(st);
				if (int ret = lua_pcall(l, 1, 1, 0)) {
					const char *errmsg( "Unknown error" );
					if (ret == LUA_ERRRUN)
						errmsg = lua_tostring(l, -1);
					else if (ret == LUA_ERRMEM)
						errmsg = "memory allocation failure";
					else if (ret == LUA_ERRERR)
						errmsg = "error in error handler function";
					luaL_error(l, "Error in filter function: %s", errmsg);
				}
				if (!lua_toboolean(l, -1)) {
					lua_pop(l, 1);
					continue;
				}
				lua_pop(l, 1);
			}

			lua_pushinteger(l, lua_rawlen(l, -1)+1);
			lua_pushstring(l, (*i).first.c_str());
			lua_rawset(l, -3);
		}
	}

	LUA_DEBUG_END(l, 1);

	return 1;
}

template <> const char *LuaObject<LuaUncopyable<ShipType> >::s_type = "ShipType";

template <> void LuaObject<LuaUncopyable<ShipType> >::RegisterClass()
{
	static const luaL_Reg l_methods[] = {
		{ "GetLinearThrust",      l_shiptype_get_linear_thrust       },
		{ "GetEquipSlotCapacity", l_shiptype_get_equip_slot_capacity },

		{ "GetShipType",  l_shiptype_get_ship_type  },
		{ "GetShipTypes", l_shiptype_get_ship_types },
		{ 0, 0 }
	};

	static const luaL_Reg l_attrs[] = {
		{ "name",              l_shiptype_attr_name               },
		{ "angularThrust",     l_shiptype_attr_angular_thrust     },
		{ "capacity",          l_shiptype_attr_capacity           },
		{ "hullMass",          l_shiptype_attr_hull_mass          },
		{ "basePrice",         l_shiptype_attr_base_price         },
		{ "defaultHyperdrive", l_shiptype_attr_default_hyperdrive },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, NULL, l_methods, l_attrs, NULL);
}
