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

	// create Ship table, attach methods to it, leave it on the stack
	luaL_openlib(l, "Ship", ship_methods, 0);

	// create the metatable, leave it on the stack
	luaL_newmetatable(l, "Ship");
	// attach metamethods to it
	luaL_openlib(l, 0, ship_meta, 0);

	// remove them both from the stack
	lua_pop(l, 2);
}
