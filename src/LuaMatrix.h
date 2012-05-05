#ifndef _LUAMATRIX_H
#define _LUAMATRIX_H

#include "LuaUtils.h"
#include "matrix4x4.h"

#define MYLUA_MATRIX "Matrix"

namespace MyLuaMatrix {
	matrix4x4f *checkMatrix (lua_State *L, int index);
	matrix4x4f *pushMatrix(lua_State *L);
	int Matrix_register (lua_State *L);
} /* namespace MyLuaMatrix */

#endif
