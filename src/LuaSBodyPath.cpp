#include "LuaObject.h"
#include "LuaUtils.h"
#include "StarSystem.h"

template <> const char *LuaObject<LuaUncopyable<SBodyPath> >::s_type = "SBodyPath";
template <> const char *LuaObject<LuaUncopyable<SBodyPath> >::s_inherit = NULL;

template <> const luaL_reg LuaObject<LuaUncopyable<SBodyPath> >::s_methods[] = {
	{ 0, 0 }
};

template <> const luaL_reg LuaObject<LuaUncopyable<SBodyPath> >::s_meta[] = {
	{ 0, 0 }
};
