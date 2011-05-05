#include "LuaObject.h"
#include "LuaEquipType.h"
#include "LuaUtils.h"
#include "LuaConstants.h"
#include "EquipType.h"

static int l_equiptype_attr_name(lua_State *l)
{
	const EquipType *et = LuaEquipType::GetFromLua(1);
	lua_pushstring(l, et->name);
	return 1;
}

static int l_equiptype_attr_slot(lua_State *l)
{
	const EquipType *et = LuaEquipType::GetFromLua(1);
	lua_pushstring(l, LuaConstants::GetConstantString(l, "EquipSlot", et->slot));
	return 1;
}

static int l_equiptype_attr_base_price(lua_State *l)
{
	const EquipType *et = LuaEquipType::GetFromLua(1);
	lua_pushnumber(l, (double)(et->basePrice)*0.01);
	return 1;
}

static int l_equiptype_attr_mass(lua_State *l)
{
	const EquipType *et = LuaEquipType::GetFromLua(1);
	lua_pushinteger(l, et->mass);
	return 1;
}

static int l_equiptype_get_equip_type(lua_State *l)
{
	Equip::Type e = static_cast<Equip::Type>(LuaConstants::GetConstant(l, "EquipType", luaL_checkstring(l, 1)));
	LuaEquipType::PushToLua(const_cast<EquipType*>(&(Equip::types[e])));
	return 1;
}

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
				lua_call(l, 2, 1);
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
