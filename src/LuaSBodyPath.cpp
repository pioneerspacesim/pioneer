#include "LuaObject.h"
#include "LuaSBodyPath.h"
#include "LuaUtils.h"
#include "StarSystem.h"
#include "Sector.h"

static int l_sbodypath_new(lua_State *l)
{
	int sector_x = luaL_checkinteger(l, 1);
	int sector_y = luaL_checkinteger(l, 2);
	int system_idx = luaL_checkinteger(l, 3);

	int sbody_id = 0;
	if (!lua_isnone(l, 4))
		sbody_id = luaL_checkinteger(l, 4);
	
	Sector s(sector_x, sector_y);
	if ((size_t)system_idx >= s.m_systems.size())
		luaL_error(l, "System %d in sector [%d,%d] does not exist", system_idx, sector_x, sector_y);

	// XXX explode if sbody_id doesn't exist in the target system?
	
	SBodyPath *path = new SBodyPath(sector_x, sector_y, system_idx);
	path->sbodyId = sbody_id;

	LuaSBodyPath::PushToLuaGC(path);

	return 1;
}

static int l_sbodypath_get_sector_x(lua_State *l)
{
	SBodyPath *path = LuaSBodyPath::GetFromLua(1);
	lua_pushinteger(l, path->sectorX);
	return 1;
}

static int l_sbodypath_get_sector_y(lua_State *l)
{
	SBodyPath *path = LuaSBodyPath::GetFromLua(1);
	lua_pushinteger(l, path->sectorY);
	return 1;
}

static int l_sbodypath_get_system_index(lua_State *l)
{
	SBodyPath *path = LuaSBodyPath::GetFromLua(1);
	lua_pushinteger(l, path->systemNum);
	return 1;
}

static int l_sbodypath_get_system_name(lua_State *l)
{
	SBodyPath *path = LuaSBodyPath::GetFromLua(1);
	StarSystem *s = StarSystem::GetCached(path);
	lua_pushstring(l, s->GetName().c_str());
	return 1;
}

static int l_sbodypath_get_body_name(lua_State *l)
{
	SBodyPath *path = LuaSBodyPath::GetFromLua(1);
	StarSystem *s = StarSystem::GetCached(path);
	SBody *sbody = s->GetBodyByPath(path);
	lua_pushstring(l, sbody->name.c_str());
	return 1;
}

static int l_sbodypath_is_same_system(lua_State *l)
{
	SBodyPath *a = LuaSBodyPath::GetFromLua(1);
	SBodyPath *b = LuaSBodyPath::GetFromLua(2);

	lua_pushboolean(l, a->sectorX == b->sectorX && a->sectorY == b->sectorY && a->systemNum == b->systemNum);
	return 1;
}

static int l_sbodypath_meta_eq(lua_State *l)
{
	SBodyPath *a = LuaSBodyPath::GetFromLua(1);
	SBodyPath *b = LuaSBodyPath::GetFromLua(2);

	lua_pushboolean(l, *a == *b);
	return 1;
}

template <> const char *LuaObject<LuaUncopyable<SBodyPath> >::s_type = "BodyPath";

template <> void LuaObject<LuaUncopyable<SBodyPath> >::RegisterClass()
{
	static const luaL_reg l_methods[] = {
		{ "New",            l_sbodypath_new              },
		{ "GetSectorX",     l_sbodypath_get_sector_x     },
		{ "GetSectorY",     l_sbodypath_get_sector_y     },
		{ "GetSystemIndex", l_sbodypath_get_system_index },
		{ "GetSystemName",  l_sbodypath_get_system_name  },
		{ "GetBodyName",    l_sbodypath_get_body_name    },
		{ "IsSameSystem",   l_sbodypath_is_same_system   },
		{ 0, 0 }
	};

	static const luaL_reg l_meta[] = {
		{ "__eq", l_sbodypath_meta_eq },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, NULL, l_methods, l_meta);
}
