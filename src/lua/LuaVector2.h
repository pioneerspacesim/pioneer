// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LUAVECTOR2_H
#define _LUAVECTOR2_H

#include "vector2.h"

struct lua_State;

namespace LuaVector2 {
	extern const char LibName[];
	extern const char TypeName[];

	void Register(lua_State *L);
	vector2d *PushNewToLua(lua_State *L);
	inline void PushToLua(lua_State *L, const vector2d &v) { *PushNewToLua(L) = v; }
	const vector2d *GetFromLua(lua_State *L, int idx);
	vector2d *CheckFromLua(lua_State *L, int idx);

	inline void PushToLuaF(lua_State *L, const vector2f &v) { PushToLua(L, vector2d(v)); }
	inline vector2f CheckFromLuaF(lua_State *L, int idx) { return vector2f(*CheckFromLua(L, idx)); }
} // namespace LuaVector2

inline void pi_lua_generic_push(lua_State *l, const vector2d &value) { LuaVector2::PushToLua(l, value); }

inline void pi_lua_generic_pull(lua_State *l, int index, vector2d &out)
{
	out = *LuaVector2::CheckFromLua(l, index);
}

inline bool pi_lua_strict_pull(lua_State *l, int index, vector2d &out)
{
	const vector2d *tmp = LuaVector2::GetFromLua(l, index);
	if (tmp) {
		out = *tmp;
		return true;
	}
	return false;
}

#endif
