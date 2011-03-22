#ifndef _LUAUTILS_H
#define _LUAUTILS_H

#include "lua.h"

inline void pi_lua_settable(lua_State *l, const char *key, int value)
{
	lua_pushstring(l, key);
	lua_pushinteger(l, value);
	lua_settable(l, -3);
}

inline void pi_lua_settable(lua_State *l, int key, int value)
{
	lua_pushinteger(l, key);
	lua_pushinteger(l, value);
	lua_settable(l, -3);
}

inline void pi_lua_settable(lua_State *l, const char *key, double value)
{
	lua_pushstring(l, key);
	lua_pushnumber(l, value);
	lua_settable(l, -3);
}

inline void pi_lua_settable(lua_State *l, int key, double value)
{
	lua_pushinteger(l, key);
	lua_pushnumber(l, value);
	lua_settable(l, -3);
}

#endif
