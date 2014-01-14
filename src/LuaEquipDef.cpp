// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Lua.h"
#include "LuaEquipDef.h"
#include "LuaUtils.h"
#include "EnumStrings.h"
#include "EquipType.h"

/*
 * Class: EquipDef
 *
 * Class for a description of a type of equipment or cargo.
 */

void LuaEquipDef::Register()
{
	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);

	lua_newtable(l);

	for (int i = Equip::NONE; i < Equip::TYPE_MAX; i++)
	{
		const EquipType *et = &(Equip::types[i]);
		const char *id = EnumStrings::GetString("EquipType", i);

		lua_newtable(l);

		pi_lua_settable(l, "id",          id);
		pi_lua_settable(l, "name",        et->name);
		pi_lua_settable(l, "slot",        EnumStrings::GetString("EquipSlot", et->slot));
		pi_lua_settable(l, "basePrice",   double(et->basePrice)*0.01);
		pi_lua_settable(l, "mass",        et->mass);
		pi_lua_settable(l, "purchasable", et->purchasable);

		pi_lua_readonly_table_proxy(l, -1);
		lua_setfield(l, -3, id);
		lua_pop(l, 1);
	}

	lua_getfield(l, LUA_REGISTRYINDEX, "CoreImports");
	pi_lua_readonly_table_proxy(l, -2);
	lua_setfield(l, -2, "EquipDef");
	lua_pop(l, 2);

	LUA_DEBUG_END(l, 0);
}
