#ifndef _LUAFIXED_H
#define _LUAFIXED_H

#include "fixed.h"

struct lua_State;

namespace LuaFixed {
	extern const char LibName[];
	extern const char TypeName[];

	void Register(lua_State *L);
	void PushToLua(lua_State *L, const fixed &v);
	const fixed *GetFromLua(lua_State *L, int idx);
	const fixed *CheckFromLua(lua_State *L, int idx);
}

#endif
