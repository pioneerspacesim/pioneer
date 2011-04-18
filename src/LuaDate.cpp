#include "LuaDate.h"
#include "LuaObject.h"
#include "LuaUtils.h"

static int l_date_format(lua_State *l)
{
	double t = luaL_checknumber(l, 1);
	lua_pushstring(l, format_date(t).c_str());
	return 1;
}

void LuaDate::Register()
{
	lua_State *l = LuaManager::Instance()->GetLuaState();

	LUA_DEBUG_START(l);

	static const luaL_reg methods[] = {
		{ "Format", l_date_format },
		{ 0, 0 }
	};

	luaL_register(l, "Date", methods);

	LUA_DEBUG_END(l, 0);
}
