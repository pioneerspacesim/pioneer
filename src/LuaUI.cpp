#include "LuaUI.h"
#include "LuaObject.h"
#include "LuaUtils.h"
#include "Pi.h"
#include "ShipCpanel.h"

static int l_ui_message(lua_State *l)
{
	std::string from = luaL_checkstring(l, 1);
	std::string msg = luaL_checkstring(l, 2);
	Pi::cpan->MsgLog()->Message(from, msg);
	return 0;
}

static int l_ui_important_message(lua_State *l)
{
	std::string from = luaL_checkstring(l, 1);
	std::string msg = luaL_checkstring(l, 2);
	Pi::cpan->MsgLog()->ImportantMessage(from, msg);
	return 0;
}

void LuaUI::Register()
{
	lua_State *l = Pi::luaManager.GetLuaState();

	LUA_DEBUG_START(l);

	static const luaL_reg methods[] = {
		{ "Message",          l_ui_message           },
		{ "ImportantMessage", l_ui_important_message },
		{ 0, 0 }
	};

	luaL_register(l, "UI", methods);
	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);
}
