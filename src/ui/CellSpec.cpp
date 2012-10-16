// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "CellSpec.h"
#include "LuaObject.h"

namespace UI {

CellSpec CellSpec::FromLuaTable(lua_State *l, int idx) {
	const int table = lua_absindex(l, idx);
	assert(lua_istable(l, table));

	float cellPercent[MAX_CELLS];
	for (size_t i = 0; i < 8 && i < lua_rawlen(l, table); i++) {
	for (size_t i = 0; i < MAX_CELLS && i < lua_rawlen(l, table); i++) {
		lua_rawgeti(l, table, i+1);
		cellPercent[i] = luaL_checknumber(l, -1);
		lua_pop(l, 1);
	}

	return CellSpec(cellPercent, lua_rawlen(l, table));
}

}
