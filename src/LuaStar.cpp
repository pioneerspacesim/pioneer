#include "LuaObject.h"
#include "LuaUtils.h"

template <> const char *LuaObject<Star>::s_type = "Star";
template <> const char *LuaObject<Star>::s_inherit = "Body";

template <> const luaL_reg LuaObject<Star>::s_methods[] = {
	{ 0, 0 }
};

template <> const luaL_reg LuaObject<Star>::s_meta[] = {
	{ 0, 0 }
};
