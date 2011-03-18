#include "LuaShip.h"

static const luaL_reg ship_methods[] = {
	{ 0, 0 }
};

static const luaL_reg ship_meta[] = {
	{ 0, 0 }
};

void LuaShip::RegisterClass()
{
	lua_State *l = LuaManager::Instance()->GetLuaState();

	luaL_openlib(l, "Ship", ship_methods, 0);
	luaL_newmetatable(l, "Ship");
	luaL_openlib(l, 0, ship_meta, 0);
	lua_pushliteral(l, "__index");
	lua_pushvalue(l, -3);
	lua_rawset(l, -3);
	lua_pushliteral(l, "__metatable");
	lua_pushvalue(l, -3);
	lua_rawset(l, -3);
	lua_pop(l, 2);
}
