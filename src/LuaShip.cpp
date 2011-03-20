#include "LuaShip.h"

static int l_ship_get_label(lua_State *l)
{
	luaL_checktype(l, 1, LUA_TUSERDATA);
	lid *idp = (lid*)lua_touserdata(l, 1);
	Ship *s = static_cast<Ship*>(LuaObject::Lookup(*idp));
	lua_pushstring(l, s->GetLabel().c_str());
	return 1;
} 

static const char *s_name = "Ship";

static const luaL_reg s_methods[] = {
	{ "get_label", l_ship_get_label },
	{ 0, 0 }
};

static const luaL_reg s_meta[] = {
	{ 0, 0 }
};

void LuaShip::RegisterClass()
{
	CreateClass(s_name, s_methods, s_meta);
}

void LuaShip::PushToLua()
{
	LuaObject::PushToLua(s_name);
}
