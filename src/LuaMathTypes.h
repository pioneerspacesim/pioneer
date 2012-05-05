#ifndef _LUAMATHTYPES_H
#define _LUAMATHTYPES_H

/*
 * Old Lua classes for fixed, vector and matrix types. Do not use these unless
 * you absolutely have to (and if you're not sure if you have to, then you
 * don't). Instead use the OOLua classes from PiLuaClasses.h
 */

#include "LuaUtils.h"
#include "matrix4x4.h"

#define MYLUA_MATRIX "Matrix"

namespace MyLuaMatrix {
	matrix4x4f *checkMatrix (lua_State *L, int index);
	matrix4x4f *pushMatrix(lua_State *L);
	int Matrix_register (lua_State *L);
} /* namespace MyLuaMatrix */

#endif /* _MYLUAMATHTYPES_H */
