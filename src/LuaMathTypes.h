#ifndef _LUAMATHTYPES_H
#define _LUAMATHTYPES_H

/*
 * Old Lua classes for fixed, vector and matrix types. Do not use these unless
 * you absolutely have to (and if you're not sure if you have to, then you
 * don't). Instead use the OOLua classes from PiLuaClasses.h
 */

#include "LuaUtils.h"
#include "vector3.h"
#include "matrix4x4.h"

#define MYLUA_VEC "Vec"
#define MYLUA_MATRIX "Matrix"

namespace MyLuaVec {
	int Vec_new(lua_State *L);
	int Vec_newNormalized(lua_State *L);
	vector3f *checkVec (lua_State *L, int index);
	vector3f *pushVec(lua_State *L);
	int Vec_register (lua_State *L);
} /* namespace MyLuaVec */

namespace MyLuaMatrix {
	matrix4x4f *checkMatrix (lua_State *L, int index);
	matrix4x4f *pushMatrix(lua_State *L);
	int Matrix_register (lua_State *L);
} /* namespace MyLuaMatrix */

#endif /* _MYLUAMATHTYPES_H */
