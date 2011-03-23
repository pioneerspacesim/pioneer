#include "LuaObject.h"
#include "LuaUtils.h"
#include "StarSystem.h"

template <> const char *LuaSubObject<LuaUncopyable<SBodyPath> >::s_type = "SBodyPath";

template <> const luaL_reg LuaSubObject<LuaUncopyable<SBodyPath> >::s_methods[] = {
	{ 0, 0 }
};

template <> const luaL_reg LuaSubObject<LuaUncopyable<SBodyPath> >::s_meta[] = {
	{ 0, 0 }
};
