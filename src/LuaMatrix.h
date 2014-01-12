// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LUAMATRIX_H
#define _LUAMATRIX_H

#include "LuaUtils.h"
#include "matrix4x4.h"

namespace LuaMatrix {
	extern const char LibName[];
	extern const char TypeName[];

	void Register(lua_State *L);
	matrix4x4f *PushNewToLua(lua_State *L);
	inline void PushToLua(lua_State *L, const matrix4x4f &m) { matrix4x4f *v = PushNewToLua(L); *v = m; }
	const matrix4x4f *GetFromLua(lua_State *L, int index);
	const matrix4x4f *CheckFromLua(lua_State *L, int index);
}

#endif
