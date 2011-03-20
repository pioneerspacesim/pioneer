#include "LuaShip.h"

static int l_ship_get_label(lua_State *l)
{
	luaL_checktype(l, 1, LUA_TLIGHTUSERDATA);
	lid id = (lid)lua_touserdata(l, 1);
	Ship *s = static_cast<Ship*>(LuaObject::Lookup(id));
	lua_pushstring(l, s->GetLabel().c_str());
	return 1;
}

static const luaL_reg ship_methods[] = {
	{ "get_label", l_ship_get_label },
	{ 0, 0 }
};

static const luaL_reg ship_meta[] = {
	{ 0, 0 }
};

void LuaShip::RegisterClass(lua_State *l)
{
	// create Ship table, attach methods to it, leave it on the stack
	luaL_openlib(l, "Ship", ship_methods, 0);

	// create the metatable, leave it on the stack
	luaL_newmetatable(l, "Ship");
	// attach metamethods to it
	luaL_openlib(l, 0, ship_meta, 0);

	// remove them both from the stack
	lua_pop(l, 2);
}
