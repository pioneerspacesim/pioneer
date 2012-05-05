#ifndef _LUAVECTOR_H
#define _LUAVECTOR_H

#include "vector3.h"

struct lua_State;

namespace LuaVector {
	extern const char LibName[];
	extern const char TypeName[];

	void Register(lua_State *L);
	void PushToLua(lua_State *L, const vector3d &v);
	const vector3d *GetFromLua(lua_State *L, int idx);
	const vector3d *CheckFromLua(lua_State *L, int idx);
}

#endif
