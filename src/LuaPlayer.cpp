#include "LuaObject.h"
#include "LuaUtils.h"

template <> const char *LuaObject<Player>::s_type = "Player";
template <> const char *LuaObject<Player>::s_inherit = "Ship";

template <> const luaL_reg LuaObject<Player>::s_methods[] = {
	{ 0, 0 }
};

template <> const luaL_reg LuaObject<Player>::s_meta[] = {
	{ 0, 0 }
};
