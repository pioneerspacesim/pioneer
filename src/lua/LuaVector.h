// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

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
	vector3d *CheckFromLua(lua_State *L, int idx);

	inline void PushToLuaF(lua_State *L, const vector3f &v) { PushToLua(L, vector3d(v)); }
	inline vector3f CheckFromLuaF(lua_State *L, int idx) { return vector3f(*CheckFromLua(L, idx)); }
} // namespace LuaVector

inline void pi_lua_generic_push(lua_State *l, const vector3d &value) { LuaVector::PushToLua(l, value); }

inline void pi_lua_generic_pull(lua_State *l, int index, vector3d &out)
{
	out = *LuaVector::CheckFromLua(l, index);
}

inline bool pi_lua_strict_pull(lua_State *l, int index, vector3d &out)
{
	const vector3d *tmp = LuaVector::GetFromLua(l, index);
	if (tmp) {
		out = *tmp;
		return true;
	}
	return false;
}

#endif
