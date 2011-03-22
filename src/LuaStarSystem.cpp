#include "LuaObject.h"
#include "LuaUtils.h"
#include "StarSystem.h"

static int l_starsystem_get_name(lua_State *l)
{
	StarSystem *s = LuaStarSystem::PullFromLua();
	lua_pushstring(l, s->GetName().c_str());
	return 1;
} 

template <> const char *LuaSubObject<StarSystem>::s_type = "StarSystem";

template <> const luaL_reg LuaSubObject<StarSystem>::s_methods[] = {
	{ "get_name", l_starsystem_get_name },
	{ 0, 0 }
};

template <> const luaL_reg LuaSubObject<StarSystem>::s_meta[] = {
	{ 0, 0 }
};
