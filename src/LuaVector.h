#ifndef _LUAVECTOR_H
#define _LUAVECTOR_H

#include "vector3.h"
#include "matrix4x4.h"
#include "fixed.h"

struct lua_State;

namespace LuaVector {
	void Register(lua_State *L);
	void PushToLua(lua_State *L, const vector3d &v);
	const vector3d *GetFromLua(lua_State *L, int idx);
	const vector3d *CheckFromLua(lua_State *L, int idx);
}

#endif
