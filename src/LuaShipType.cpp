#include "LuaObject.h"
#include "LuaShipType.h"
#include "LuaUtils.h"
#include "LuaConstants.h"
#include "ShipType.h"
#include "EquipType.h"

int l_shiptype_attr_name(lua_State *l)
{
	const ShipType *st = LuaShipType::GetFromLua(1);
	lua_pushstring(l, st->name.c_str());
	return 1;
}

int l_shiptype_attr_angular_thrust(lua_State *l)
{
	const ShipType *st = LuaShipType::GetFromLua(1);
	lua_pushnumber(l, st->angThrust);
	return 1;
}

int l_shiptype_attr_capacity(lua_State *l)
{
	const ShipType *st = LuaShipType::GetFromLua(1);
	lua_pushinteger(l, st->capacity);
	return 1;
}

int l_shiptype_attr_hull_mass(lua_State *l)
{
	const ShipType *st = LuaShipType::GetFromLua(1);
	lua_pushinteger(l, st->hullMass);
	return 1;
}

int l_shiptype_attr_base_price(lua_State *l)
{
	const ShipType *st = LuaShipType::GetFromLua(1);
	lua_pushnumber(l, (double)(st->baseprice) * 0.01);
	return 1;
}

int l_shiptype_attr_default_hyperdrive(lua_State *l)
{
	const ShipType *st = LuaShipType::GetFromLua(1);
	lua_pushstring(l, LuaConstants::GetConstantString(l, "EquipType", st->hyperdrive));
	return 1;
}

int l_shiptype_get_linear_thrust(lua_State *l)
{
	const ShipType *st = LuaShipType::GetFromLua(1);
	ShipType::Thruster t = static_cast<ShipType::Thruster>(LuaConstants::GetConstant(l, "ShipTypeThruster", luaL_checkstring(l, 2)));
	lua_pushnumber(l, st->linThrust[t]);
	return 1;
}

int l_shiptype_get_equip_slot_capacity(lua_State *l)
{
	const ShipType *st = LuaShipType::GetFromLua(1);
	Equip::Slot slot = static_cast<Equip::Slot>(LuaConstants::GetConstant(l, "EquipSlot", luaL_checkstring(l, 2)));
	lua_pushnumber(l, st->equipSlotCapacity[slot]);
	return 1;
}

static int l_shiptype_get_ship_type(lua_State *l)
{
	const char *type = luaL_checkstring(l, 1);

	std::map<ShipType::Type,ShipType>::iterator i = ShipType::types.find(type);
	if (i == ShipType::types.end())
		luaL_error(l, "Invalid ship name '%s'", type);
	
	LuaShipType::PushToLua(&((*i).second));
	return 1;
}

static int l_shiptype_get_ship_types(lua_State *l)
{
	LUA_DEBUG_START(l);

	ShipType::Tag tag = ShipType::TAG_NONE;

	if (lua_gettop(l) >= 1)
		tag = static_cast<ShipType::Tag>(LuaConstants::GetConstant(l, "ShipTypeTag", luaL_checkstring(l, 1)));

	bool filter = false;
	if (lua_gettop(l) >= 2) {
		if (!lua_isfunction(l, 2))
			luaL_typerror(l, 2, lua_typename(l, LUA_TFUNCTION));
		filter = true;
	}
	
	lua_newtable(l);
	pi_lua_table_ro(l);
	
	for (std::map<ShipType::Type,ShipType>::iterator i = ShipType::types.begin(); i != ShipType::types.end(); i++)
	{
		ShipType *st = &((*i).second);
		if (tag == ShipType::TAG_NONE || tag == st->tag) {
			if (filter) {
				lua_pushvalue(l, 2);
				LuaShipType::PushToLua(st);
				lua_call(l, 1, 1);
				if (!lua_toboolean(l, -1)) {
					lua_pop(l, 1);
					continue;
				}
				lua_pop(l, 1);
			}

			lua_pushinteger(l, lua_objlen(l, -1)+1);
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
	static const luaL_reg l_methods[] = {
		{ "GetLinearThrust",      l_shiptype_get_linear_thrust       },
		{ "GetEquipSlotCapacity", l_shiptype_get_equip_slot_capacity },

		{ "GetShipType",  l_shiptype_get_ship_type  },
		{ "GetShipTypes", l_shiptype_get_ship_types },
		{ 0, 0 }
	};

	static const luaL_reg l_attrs[] = {
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
