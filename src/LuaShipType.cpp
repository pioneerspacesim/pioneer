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
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, NULL, l_methods, NULL);
}
