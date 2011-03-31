#include "LuaObject.h"
#include "LuaUtils.h"
#include "StarSystem.h"

static int l_sbodypath_get_name(lua_State *l)
{
	SBodyPath *path = LuaSBodyPath::GetFromLua(1);
	StarSystem *s = StarSystem::GetCached(*path);
	LuaString::PushToLua(s->GetName().c_str());
	return 1;
}

template <> const char *LuaObject<LuaUncopyable<SBodyPath> >::s_type = "SBodyPath";
template <> const char *LuaObject<LuaUncopyable<SBodyPath> >::s_inherit = NULL;

template <> const luaL_reg LuaObject<LuaUncopyable<SBodyPath> >::s_methods[] = {
	{ "GetName", l_sbodypath_get_name },
	{ 0, 0 }
};

template <> const luaL_reg LuaObject<LuaUncopyable<SBodyPath> >::s_meta[] = {
	{ 0, 0 }
};
