#include "LuaObject.h"
#include "LuaUtils.h"

template <> const char *LuaSubObject<Player>::s_type = "Player";
template <> const char *LuaSubObject<Player>::s_inherit = "Ship";

template <> const luaL_reg LuaSubObject<Player>::s_methods[] = {
	{ 0, 0 }
};

template <> const luaL_reg LuaSubObject<Player>::s_meta[] = {
	{ 0, 0 }
};
