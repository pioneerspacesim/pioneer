#include "LuaObject.h"
#include "LuaUtils.h"

template <> const char *LuaSubObject<Star>::s_type = "Star";
template <> const char *LuaSubObject<Star>::s_inherit = "Body";

template <> const luaL_reg LuaSubObject<Star>::s_methods[] = {
	{ 0, 0 }
};

template <> const luaL_reg LuaSubObject<Star>::s_meta[] = {
	{ 0, 0 }
};
