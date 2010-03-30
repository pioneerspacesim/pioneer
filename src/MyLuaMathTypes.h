#ifndef _MYLUAMATHTYPES_H
#define _MYLUAMATHTYPES_H

extern "C" {
#include "lua/lua.h"
#include "lua/lualib.h"
#include "lua/lauxlib.h"
}

#include "vector3.h"
#include "matrix4x4.h"

#define VEC "Vec"
#define MAT4X4 "Mat4x4"

namespace MyLuaVec {
	int Vec_new(lua_State *L);
	int Vec_newNormalized(lua_State *L);
	vector3f *checkVec (lua_State *L, int index);
	vector3f *pushVec(lua_State *L);
	int Vec_register (lua_State *L);
}; /* namespace MyLuaVec */

namespace MyLuaMatrix {
	matrix4x4f *checkMat4x4 (lua_State *L, int index);
	matrix4x4f *pushMat4x4(lua_State *L);
	int Mat4x4_register (lua_State *L);
}; /* namespace MyLuaMatrix */

// i'll just throw them in here...
static inline const char *luaPi_gettable_checkstring(lua_State *L, int table, int idx)
{
	lua_pushinteger(L, idx);
	lua_gettable(L, (table<0 ? table-1 : table));
	const char *r = luaL_checkstring(L, -1);
	lua_pop(L, 1);
	return r;
}

static inline double luaPi_gettable_checknumber(lua_State *L, int table, int idx)
{
	lua_pushinteger(L, idx);
	lua_gettable(L, (table<0 ? table-1 : table));
	double num = luaL_checknumber(L, -1);
	lua_pop(L, 1);
	return num;
}

static inline int luaPi_gettable_checkinteger(lua_State *L, int table, int idx)
{
	lua_pushinteger(L, idx);
	lua_gettable(L, (table<0 ? table-1 : table));
	int num = luaL_checkinteger(L, -1);
	lua_pop(L, 1);
	return num;
}
#endif /* _MYLUAMATHTYPES_H */
