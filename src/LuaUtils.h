#ifndef _LUAUTILS_H
#define _LUAUTILS_H

#include <string>
#include "lua/lua.hpp"
#include "utils.h"

inline void pi_lua_settable(lua_State *l, const char *key, int value)
{
	lua_pushstring(l, key);
	lua_pushinteger(l, value);
	lua_rawset(l, -3);
}

inline void pi_lua_settable(lua_State *l, int key, int value)
{
	lua_pushinteger(l, key);
	lua_pushinteger(l, value);
	lua_rawset(l, -3);
}

inline void pi_lua_settable(lua_State *l, const char *key, double value)
{
	lua_pushstring(l, key);
	lua_pushnumber(l, value);
	lua_rawset(l, -3);
}

inline void pi_lua_settable(lua_State *l, int key, double value)
{
	lua_pushinteger(l, key);
	lua_pushnumber(l, value);
	lua_rawset(l, -3);
}

inline void pi_lua_settable(lua_State *l, int key, const char *value)
{
	lua_pushinteger(l, key);
	lua_pushstring(l, value);
	lua_rawset(l, -3);
}

inline void pi_lua_settable(lua_State *l, const char *key, const char *value)
{
	lua_pushstring(l, key);
	lua_pushstring(l, value);
	lua_rawset(l, -3);
}

void pi_lua_open_standard_base(lua_State *l);

// pushes a read-only proxy table that points at the table at <index>
void pi_lua_readonly_table_proxy(lua_State *l, int index);
// pushes the underlying (read-write) table pointed to by the proxy at <index>
void pi_lua_readonly_table_original(lua_State *l, int index);

int  pi_lua_panic(lua_State *l) __attribute((noreturn));
void pi_lua_protected_call(lua_State* state, int nargs, int nresults);
void pi_lua_dofile(lua_State *l, const std::string &path);
void pi_lua_dofile_recursive(lua_State *l, const std::string &basepath);
int  pi_load_lua(lua_State *l);

void pi_lua_warn(lua_State *l, const char *format, ...) __attribute((format(printf,2,3)));

#ifdef DEBUG
#include <stdlib.h> // for abort()
# define LUA_DEBUG_START(luaptr) const int __luaStartStackDepth = lua_gettop(luaptr)
# define LUA_DEBUG_END(luaptr, expectedStackDiff) \
	do { \
		const int __luaEndStackDepth = lua_gettop(luaptr); \
		if ( __luaEndStackDepth-expectedStackDiff != __luaStartStackDepth) { \
			fprintf(stderr, "%s:%d: lua stack difference is %d, expected %d\n", \
				__FILE__, __LINE__, __luaEndStackDepth-__luaStartStackDepth, expectedStackDiff); \
			abort(); \
		} \
	} while (0)
# define LUA_DEBUG_CHECK(luaptr, expectedStackDiff) LUA_DEBUG_END(luaptr, expectedStackDiff)
#else
# define LUA_DEBUG_START(luaptr)
# define LUA_DEBUG_END(luaptr, expectedStackDiff)
# define LUA_DEBUG_CHECK(luaptr, expectedStackDiff)
#endif

#endif
