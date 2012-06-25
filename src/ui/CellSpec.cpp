#include "CellSpec.h"
#include "LuaObject.h"

namespace UI {

CellSpec CellSpec::FromLuaTable(lua_State *l, int idx) {
	int table = idx > 0 ? idx : lua_gettop(l) - idx;
	assert(lua_istable(l, table));

	float cellPercent[8];
	for (size_t i = 0; i < 8 && i < lua_rawlen(l, table); i++) {
		lua_rawgeti(l, table, i+1);
		cellPercent[i] = luaL_checknumber(l, -1);
		lua_pop(l, 1);
	}

	return CellSpec(cellPercent, lua_rawlen(l, table));
}

}
