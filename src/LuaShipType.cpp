#include "LuaObject.h"
#include "LuaShipType.h"
#include "LuaUtils.h"
#include "ShipType.h"
#include "EquipType.h"

int l_shiptype_get_name(lua_State *l)
{
	const ShipType *st = LuaShipType::GetFromLua(1);
	lua_pushstring(l, st->name.c_str());
	return 1;
}

int l_shiptype_get_linear_thrust(lua_State *l)
{
	const ShipType *st = LuaShipType::GetFromLua(1);
	ShipType::Thruster t = static_cast<ShipType::Thruster>(luaL_checkinteger(l, 2));
	if (t < 0 || t >= ShipType::THRUSTER_MAX)
		luaL_error(l, "Unknown thruster type %d", t);
	lua_pushnumber(l, st->linThrust[t]);
	return 1;
}

int l_shiptype_get_angular_thrust(lua_State *l)
{
	const ShipType *st = LuaShipType::GetFromLua(1);
	lua_pushnumber(l, st->angThrust);
	return 1;
}

int l_shiptype_get_equip_slot_capacity(lua_State *l)
{
	const ShipType *st = LuaShipType::GetFromLua(1);
	Equip::Slot t = static_cast<Equip::Slot>(luaL_checkinteger(l, 2));
	if (t < 0 || t >= Equip::SLOT_MAX)
		luaL_error(l, "Unknown equipment type %d", t);
	lua_pushnumber(l, st->equipSlotCapacity[t]);
	return 1;
}

int l_shiptype_get_capacity(lua_State *l)
{
	const ShipType *st = LuaShipType::GetFromLua(1);
	lua_pushinteger(l, st->capacity);
	return 1;
}

int l_shiptype_get_hull_mass(lua_State *l)
{
	const ShipType *st = LuaShipType::GetFromLua(1);
	lua_pushinteger(l, st->hullMass);
	return 1;
}

int l_shiptype_get_base_price(lua_State *l)
{
	const ShipType *st = LuaShipType::GetFromLua(1);
	lua_pushnumber(l, (double)(st->baseprice) * 0.01);
	return 1;
}

int l_shiptype_get_default_hyperdrive(lua_State *l)
{
	const ShipType *st = LuaShipType::GetFromLua(1);
	lua_pushinteger(l, st->hyperdrive);
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
		tag = static_cast<ShipType::Tag>(luaL_checkinteger(l, 1));
	
	if (tag < 0 || tag >= ShipType::TAG_MAX)
		luaL_error(l, "Unknown ship tag %d", tag);

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
		{ "GetName",              l_shiptype_get_name                },
		{ "GetLinearThrust",      l_shiptype_get_linear_thrust       },
		{ "GetAngularThrust",     l_shiptype_get_angular_thrust      },
		{ "GetEquipSlotCapacity", l_shiptype_get_equip_slot_capacity },
		{ "GetCapacity",          l_shiptype_get_capacity            },
		{ "GetHullMass",          l_shiptype_get_hull_mass           },
		{ "GetBasePrice",         l_shiptype_get_base_price          },
		{ "GetDefaultHyperdrive", l_shiptype_get_default_hyperdrive  },

		{ "GetShipType",  l_shiptype_get_ship_type  },
		{ "GetShipTypes", l_shiptype_get_ship_types },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, NULL, l_methods, NULL, NULL);
}
