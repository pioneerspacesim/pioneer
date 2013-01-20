// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "LuaEvent.h"
#include "LuaManager.h"
#include "LuaObject.h"
#include "LuaUtils.h"

namespace LuaEvent {

static void _get_method_onto_stack(lua_State *l, const char *method) {
	LUA_DEBUG_START(l);

	int top = lua_gettop(l);

	lua_rawgeti(l, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
	lua_pushstring(l, "Event");
	lua_rawget(l, -2);
	assert(lua_istable(l, -1));

	lua_pushstring(l, method);
	lua_rawget(l, -2);
	assert(lua_isfunction(l, -1));

	lua_insert(l, top+1);
	lua_settop(l, top+1);

	LUA_DEBUG_END(l, 1);
}

void Clear()
{
	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);
	_get_method_onto_stack(l, "_Clear");
	pi_lua_protected_call(l, 0, 0);
	LUA_DEBUG_END(l, 0);
}

void Emit()
{
	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);
	_get_method_onto_stack(l, "_Emit");
	pi_lua_protected_call(l, 0, 0);
	LUA_DEBUG_END(l, 0);
}

void Queue(const char *event, const ArgsBase &args)
{
	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);
	_get_method_onto_stack(l, "Queue");

	int top = lua_gettop(l);
    lua_pushstring(l, event);
	args.PrepareStack();
	pi_lua_protected_call(l, lua_gettop(l) - top, 0);

	LUA_DEBUG_END(l, 0);
}

}
