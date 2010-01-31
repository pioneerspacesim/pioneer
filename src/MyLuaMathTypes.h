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

#endif /* _MYLUAMATHTYPES_H */
