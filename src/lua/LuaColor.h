#ifndef LUACOLOR_H
#define LUACOLOR_H

#include "Color.h"

struct lua_State;

namespace LuaColor {
	extern const char LibName[];
	extern const char TypeName[];

	void Register(lua_State *L);
	Color4ub *PushNewToLua(lua_State *L);
	inline void PushToLua(lua_State *L, const Color4ub &v) { *PushNewToLua(L) = v; }
	const Color4ub *GetFromLua(lua_State *L, int idx);
	Color4ub *CheckFromLua(lua_State *L, int idx);

} // namespace LuaColor

inline void pi_lua_generic_push(lua_State *l, const Color4ub &value) { LuaColor::PushToLua(l, value); }

inline void pi_lua_generic_pull(lua_State *l, int index, Color4ub &out)
{
	out = *LuaColor::CheckFromLua(l, index);
}

inline bool pi_lua_strict_pull(lua_State *l, int index, Color4ub &out)
{
	const Color4ub *tmp = LuaColor::GetFromLua(l, index);
	if (tmp) {
		out = *tmp;
		return true;
	}
	return false;
}

#endif // LUACOLOR_H
