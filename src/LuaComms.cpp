// Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaComms.h"
#include "LuaObject.h"
#include "LuaUtils.h"
#include "Pi.h"
#include "ShipCpanel.h"

/*
 * Interface: Comms
 *
 * Player communication functions.
 */

void LuaComms::Register()
{
	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);

	// static const luaL_Reg l_methods[] = {
	// 	{ 0, 0 }
	// };

	lua_getfield(l, LUA_REGISTRYINDEX, "CoreImports");
	LuaObjectBase::CreateObject(0, 0, 0);
	lua_setfield(l, -2, "Comms");
	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);
}
