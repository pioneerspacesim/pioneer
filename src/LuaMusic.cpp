#include "LuaMusic.h"
#include "LuaObject.h"
#include "LuaUtils.h"

void LuaMusic::Register()
{
	lua_State *l = Pi::luaManager.GetLuaState();

	LUA_DEBUG_START();

	static const luaL_reg methods[]= {
		{ },
		{ },
		{0, 0}
	};

	luaL_register(l, "Music", methods);
	lua_pop(1, l);

	LUA_DEBUG_END(l, 0);
}
