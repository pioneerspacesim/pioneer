#ifndef _LUAVECTOR_H
#define _LUAVECTOR_H

#include "vector3.h"

struct lua_State;

namespace LuaVector {
	extern const char LibName[];
	extern const char TypeName[];

	void Register(lua_State *L);
	vector3d *PushNewToLua(lua_State *L);
	inline void PushToLua(lua_State *L, const vector3d &v) { *PushNewToLua(L) = v; }
	const vector3d *GetFromLua(lua_State *L, int idx);
	const vector3d *CheckFromLua(lua_State *L, int idx);

	inline void PushToLuaF(lua_State *L, const vector3f &v) { PushToLua(L, vector3d(v)); }
	inline vector3f CheckFromLuaF(lua_State *L, int idx) { return vector3f(*CheckFromLua(L, idx)); }
}

#endif
