#include "LuaObject.h"
#include "LuaUtils.h"
#include "StarSystem.h"

static int l_sbodypath_get_sector_x(lua_State *l)
{
	SBodyPath *path = LuaSBodyPath::GetFromLua(1);
	LuaInt::PushToLua(path->sectorX);
	return 1;
}

static int l_sbodypath_get_sector_y(lua_State *l)
{
	SBodyPath *path = LuaSBodyPath::GetFromLua(1);
	LuaInt::PushToLua(path->sectorY);
	return 1;
}

static int l_sbodypath_get_system_name(lua_State *l)
{
	SBodyPath *path = LuaSBodyPath::GetFromLua(1);
	StarSystem *s = StarSystem::GetCached(*path);
	LuaString::PushToLua(s->GetName().c_str());
	return 1;
}

static int l_sbodypath_get_body_name(lua_State *l)
{
	SBodyPath *path = LuaSBodyPath::GetFromLua(1);
	StarSystem *s = StarSystem::GetCached(*path);
	SBody *sbody = s->m_bodies[path->sbodyId];
	LuaString::PushToLua(sbody->name.c_str());
	return 1;
}

template <> const char *LuaObject<LuaUncopyable<SBodyPath> >::s_type = "SBodyPath";
template <> const char *LuaObject<LuaUncopyable<SBodyPath> >::s_inherit = NULL;

template <> const luaL_reg LuaObject<LuaUncopyable<SBodyPath> >::s_methods[] = {
	{ "GetSectorX",    l_sbodypath_get_sector_x    },
	{ "GetSectorY",    l_sbodypath_get_sector_y    },
	{ "GetSystemName", l_sbodypath_get_system_name },
	{ "GetBodyName",   l_sbodypath_get_body_name   },
	{ 0, 0 }
};

template <> const luaL_reg LuaObject<LuaUncopyable<SBodyPath> >::s_meta[] = {
	{ 0, 0 }
};
