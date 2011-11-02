#include "LuaObject.h"
#include "LuaEquipType.h"
#include "LuaUtils.h"
#include "LuaConstants.h"
#include "EquipType.h"

/*
 * Class: EquipType
 *
 * Class for a description of a type of equipment or cargo.
 */

/*
 * Attribute: name
 *
 * The name of the equipment or cargo
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   experimental
 */
static int l_equiptype_attr_name(lua_State *l)
{
	const EquipType *et = LuaEquipType::GetFromLua(1);
	lua_pushstring(l, et->name);
	return 1;
}

/*
 * Attribute: slot
 *
 * The slot that the equipment or cargo is attached to
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   experimental
 */
static int l_equiptype_attr_slot(lua_State *l)
{
	const EquipType *et = LuaEquipType::GetFromLua(1);
	lua_pushstring(l, LuaConstants::GetConstantString(l, "EquipSlot", et->slot));
	return 1;
}

/*
 * Attribute: basePrice
 *
 * The base price of the equipment or cargo. This typically receives some
 * adjustment before being used as a buy or sell price (eg based on supply or
 * demand)
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   experimental
 */
static int l_equiptype_attr_base_price(lua_State *l)
{
	const EquipType *et = LuaEquipType::GetFromLua(1);
	lua_pushnumber(l, double(et->basePrice)*0.01);
	return 1;
}

/*
 * Attribute: mass
 *
 * Mass of the equipment or cargo in tonnes. This corresponds directly to
 * amount of weight/space this will use when added to a ship.
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   experimental
 */
static int l_equiptype_attr_mass(lua_State *l)
{
	const EquipType *et = LuaEquipType::GetFromLua(1);
	lua_pushinteger(l, et->mass);
	return 1;
}

/*
 * Function: GetEquipType
 *
 * Get a description object for the given equipment type constant
 *
 * > equiptype = EquipType.GetEquipType(type)
 *
 * Parameters:
 *
 *   type - the <Constants.EquipType> string for the the wanted description
 *
 * Example:
 *
 * > local hydrogen_description = EquipType.GetEquipType("HYDROGEN")
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   experimental
 */
static int l_equiptype_get_equip_type(lua_State *l)
{
	Equip::Type e = static_cast<Equip::Type>(LuaConstants::GetConstant(l, "EquipType", luaL_checkstring(l, 1)));
	LuaEquipType::PushToLua(const_cast<EquipType*>(&(Equip::types[e])));
	return 1;
}

/*
 * Function: GetEquipTypes
 *
 * Finds equipment types that match a criteria
 *
 * > equiptypes = EquipType.GetEquipTypes(slot, filter)
 *
 * Parameters:
 *
 *   slot   - a <Constant.EquipSlot> string. Only equipment that may be used
 *            in this slot will be returned.
 *
 *   filter - an optional function. If specified the function will be called
 *            once for each equipment or cargo type with two parameters:
 *            the <Constants.EquipType> string for the equipment type and
 *            the constant description object. If the filter function returns
 *            true this equipment or cargo type will be included in the array
 *            returned by <GetEquipTypes>, otherwise it will be omitted. If no
 *            filter function is specified then all available equipment types
 *            are returned.
 *
 * Returns:
 *
 *   equiptypes - an array containing zero or more <Constants.EquipType>
 *                strings for which the the relevant descripton object matched
 *                the filter
 *
 * Example:
 *
 * > local lasers = EquipType.GetEquipTypes('LASER', function (e,et)
 * >     return et.slot == "LASER"
 * > end)
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   experimental
 */
static int l_equiptype_get_equip_types(lua_State *l)
{
	LUA_DEBUG_START(l);

	Equip::Slot slot = static_cast<Equip::Slot>(LuaConstants::GetConstant(l, "EquipSlot", luaL_checkstring(l, 1)));
	
	bool filter = false;
	if (lua_gettop(l) >= 2) {
		if (!lua_isfunction(l, 2))
			luaL_typerror(l, 2, lua_typename(l, LUA_TFUNCTION));
		filter = true;
	}

	lua_newtable(l);
	pi_lua_table_ro(l);
	
	for (int i = Equip::NONE; i < Equip::TYPE_MAX; i++) {
		EquipType *et = const_cast<EquipType*>(&(Equip::types[i]));
		if (Equip::types[i].slot == slot) {
			const char *name = LuaConstants::GetConstantString(l, "EquipType", i);

			if (filter) {
				lua_pushvalue(l, 2);
				lua_pushstring(l, name);
				LuaEquipType::PushToLua(et);
				if (int ret = lua_pcall(l, 2, 1, 0)) {
					const char *errmsg;
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

			lua_pushinteger(l, lua_objlen(l, -1)+1);
			lua_pushstring(l, name);
			lua_rawset(l, -3);
		}
	}

	LUA_DEBUG_END(l, 1);

	return 1;
}

template <> const char *LuaObject<LuaUncopyable<EquipType> >::s_type = "EquipType";

template <> void LuaObject<LuaUncopyable<EquipType> >::RegisterClass()
{
	static const luaL_reg l_methods[] = {
        { "GetEquipType",  l_equiptype_get_equip_type  },
		{ "GetEquipTypes", l_equiptype_get_equip_types },

		{ 0, 0 }
	};

	static const luaL_reg l_attrs[] = {
		{ "name",      l_equiptype_attr_name       },
		{ "slot",      l_equiptype_attr_slot       },
		{ "basePrice", l_equiptype_attr_base_price },
		{ "mass",      l_equiptype_attr_mass       },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, NULL, l_methods, l_attrs, NULL);
}
