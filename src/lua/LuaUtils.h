// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LUAUTILS_H
#define _LUAUTILS_H

// to mask __attribute on MSVC
#include "core/macros.h"

#include <lua.hpp>
#include <string>

namespace FileSystem {
	class FileData;
}

inline void pi_lua_settable(lua_State *l, const char *key, bool value)
{
	lua_pushstring(l, key);
	lua_pushboolean(l, value);
	lua_rawset(l, -3);
}

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

bool pi_lua_import(lua_State *l, const std::string &importName, bool popImported = false);
void pi_lua_import_recursive(lua_State *L, const std::string &importName);

int pi_lua_panic(lua_State *l) __attribute((noreturn));
void pi_lua_protected_call(lua_State *state, int nargs, int nresults);
int pi_lua_loadfile(lua_State *l, const FileSystem::FileData &code);
void pi_lua_dofile(lua_State *l, const std::string &path, int nret = 0);
void pi_lua_dofile_recursive(lua_State *l, const std::string &basepath);

void pi_lua_warn(lua_State *l, const char *format, ...) __attribute((format(printf, 2, 3)));

bool pi_lua_split_table_path(lua_State *l, const std::string &path);

int secure_trampoline(lua_State *l);

std::string pi_lua_traceback(lua_State *l, int top);
std::string pi_lua_dumpstack(lua_State *l, int top);
void pi_lua_printvalue(lua_State *l, int idx);
void pi_lua_stacktrace(lua_State *l);

#ifdef DEBUG
#define LUA_DEBUG_START(luaptr) const int __luaStartStackDepth = lua_gettop(luaptr)
#define LUA_DEBUG_END(luaptr, expectedStackDiff)                                                   \
	do {                                                                                           \
		const int __luaEndStackDepth = lua_gettop(luaptr);                                         \
		if (__luaEndStackDepth - expectedStackDiff != __luaStartStackDepth) {                      \
			Error("%s:%d: lua stack difference is %d, expected %d",                                \
				__FILE__, __LINE__, __luaEndStackDepth - __luaStartStackDepth, expectedStackDiff); \
		}                                                                                          \
	} while (0)
#define LUA_DEBUG_CHECK(luaptr, expectedStackDiff) LUA_DEBUG_END(luaptr, expectedStackDiff)
#else
#define LUA_DEBUG_START(luaptr)
#define LUA_DEBUG_END(luaptr, expectedStackDiff)
#define LUA_DEBUG_CHECK(luaptr, expectedStackDiff)
#endif

#endif
