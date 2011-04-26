#include "LuaFormat.h"
#include "LuaObject.h"
#include "LuaUtils.h"
#include "utils.h"

static int l_format_date(lua_State *l)
{
	double t = luaL_checknumber(l, 1);
	lua_pushstring(l, format_date(t).c_str());
	return 1;
}

static int l_format_distance(lua_State *l)
{
	double t = luaL_checknumber(l, 1);
	lua_pushstring(l, format_distance(t).c_str());
	return 1;
}

static int l_format_money(lua_State *l)
{
	double t = luaL_checknumber(l, 1);
	lua_pushstring(l, format_money((Sint64)(t*100.0)).c_str());
	return 1;
}

void LuaFormat::Register()
{
	lua_State *l = LuaManager::Instance()->GetLuaState();

	LUA_DEBUG_START(l);

	static const luaL_reg methods[] = {
		{ "Date",     l_format_date     },
		{ "Distance", l_format_distance },
		{ "Money",    l_format_money    },
		{ 0, 0 }
	};

	luaL_register(l, "Format", methods);

	LUA_DEBUG_END(l, 0);
}
