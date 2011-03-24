#include "LuaObject.h"
#include "LuaUtils.h"

template <> const char *LuaSubObject<Planet>::s_type = "Planet";
template <> const char *LuaSubObject<Planet>::s_inherit = "Body";

template <> const luaL_reg LuaSubObject<Planet>::s_methods[] = {
	{ 0, 0 }
};

template <> const luaL_reg LuaSubObject<Planet>::s_meta[] = {
	{ 0, 0 }
};
