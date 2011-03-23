#include "LuaObject.h"
#include "LuaUtils.h"
#include "StarSystem.h"

template <> const char *LuaSubObject<LockedSBodyPath>::s_type = "SBodyPath";

template <> const luaL_reg LuaSubObject<LockedSBodyPath>::s_methods[] = {
	{ 0, 0 }
};

template <> const luaL_reg LuaSubObject<LockedSBodyPath>::s_meta[] = {
	{ 0, 0 }
};
