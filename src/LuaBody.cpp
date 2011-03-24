#include "LuaObject.h"
#include "LuaUtils.h"

template <> const char *LuaSubObject<Body>::s_type = "Body";
template <> const char *LuaSubObject<Body>::s_inherit = NULL;

template <> const luaL_reg LuaSubObject<Body>::s_methods[] = {
	{ 0, 0 }
};

template <> const luaL_reg LuaSubObject<Body>::s_meta[] = {
	{ 0, 0 }
};
