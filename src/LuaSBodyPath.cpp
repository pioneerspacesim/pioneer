#include "LuaObject.h"
#include "LuaUtils.h"
#include "StarSystem.h"

template <> const char *LuaSubObject<UncopyableSBodyPath>::s_type = "SBodyPath";

template <> const luaL_reg LuaSubObject<UncopyableSBodyPath>::s_methods[] = {
	{ 0, 0 }
};

template <> const luaL_reg LuaSubObject<UncopyableSBodyPath>::s_meta[] = {
	{ 0, 0 }
};
