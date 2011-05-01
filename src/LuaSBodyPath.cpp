#include "LuaObject.h"
#include "LuaSBodyPath.h"
#include "LuaUtils.h"
#include "LuaStarSystem.h"
#include "LuaSBody.h"
#include "StarSystem.h"
#include "Sector.h"

/*
 * Class: SystemPath
 *
 * Describes the location of a system within the galaxy and optionally, a body
 * within that system.
 *
 * A <SystemPath> consists of four components
 *  - the X coordinate of the sector
 *  - the Y coordinate of the sector
 *  - the system number within that sector
 *  - optionally, the index of a body within that system
 *
 * <SystemPath> objects are typically used to describe the location of a
 * system, space station or other body when specifying hyperspace or other
 * destinations.
 *
 * <SystemPath> objects will compare equal if and only if all four of their
 * components are the same. If you want to see if two paths correspond to the
 * same system without reference to their body indexes, use <IsSameSystem>.
 */

/*
 * Function: New
 *
 * Creates a new <SystemPath> object
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  stable
 */
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

/*
 * Function: GetSectorX
 *
 * Get the X component of the path
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  stable
 */
static int l_sbodypath_get_sector_x(lua_State *l)
{
	SBodyPath *path = LuaSBodyPath::GetFromLua(1);
	lua_pushinteger(l, path->sectorX);
	return 1;
}

/*
 * Function: GetSectorY
 *
 * Get the Y component of the path
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  stable
 */
static int l_sbodypath_get_sector_y(lua_State *l)
{
	SBodyPath *path = LuaSBodyPath::GetFromLua(1);
	lua_pushinteger(l, path->sectorY);
	return 1;
}

/*
 * Function: GetSystemIndex
 *
 * Get the system index component of the path
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  stable
 */
static int l_sbodypath_get_system_index(lua_State *l)
{
	SBodyPath *path = LuaSBodyPath::GetFromLua(1);
	lua_pushinteger(l, path->systemNum);
	return 1;
}

/*
 * Function: GetBodyId
 *
 * Get the body id component of the path
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  stable
 */
static int l_sbodypath_get_body_id(lua_State *l)
{
	SBodyPath *path = LuaSBodyPath::GetFromLua(1);
	lua_pushinteger(l, path->sbodyId);
	return 1;
}

/*
 * Function: IsSameSystem
 *
 * Determine if two <SystemPath> objects point to the same system, ignoring
 * the body index.
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  stable
 */
static int l_sbodypath_is_same_system(lua_State *l)
{
	SBodyPath *a = LuaSBodyPath::GetFromLua(1);
	SBodyPath *b = LuaSBodyPath::GetFromLua(2);

	lua_pushboolean(l, a->sectorX == b->sectorX && a->sectorY == b->sectorY && a->systemNum == b->systemNum);
	return 1;
}

/*
 * Method: DistanceTo
 *
 * Calculate the distance between this and another system
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  stable
 */
static int l_sbodypath_distance_to(lua_State *l)
{
	LUA_DEBUG_START(l);

	const SysLoc *loc1 = LuaSBodyPath::GetFromLua(1);

	const SysLoc *loc2 = LuaSBodyPath::CheckFromLua(2);
	if (!loc2) {
		StarSystem *s2 = LuaStarSystem::GetFromLua(2);
		loc2 = &(s2->GetLocation());
	}

	Sector sec1(loc1->GetSectorX(), loc1->GetSectorY());
	Sector sec2(loc2->GetSectorX(), loc2->GetSectorY());
	
	double dist = Sector::DistanceBetween(&sec1, loc1->GetSystemNum(), &sec2, loc2->GetSystemNum());

	lua_pushnumber(l, dist);

	LUA_DEBUG_END(l, 1);
	return 1;
}

/*
 * Method: GetStarSystem
 *
 * Get a <StarSystem> object for the system that this path points to
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  stable
 */
static int l_sbodypath_get_star_system(lua_State *l)
{
	SBodyPath *path = LuaSBodyPath::GetFromLua(1);
	StarSystem *s = StarSystem::GetCached(path);
	LuaStarSystem::PushToLua(s);
	s->Release();
	return 1;
}

/*
 * Method: GetSystemBody
 *
 * Get a <SystemBody> object for the body that this path points to
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  stable
 */
static int l_sbodypath_get_system_body(lua_State *l)
{
	SBodyPath *path = LuaSBodyPath::GetFromLua(1);
	StarSystem *s = StarSystem::GetCached(path);
	SBody *sbody = s->GetBodyByPath(path);
	s->Release();
	LuaSBody::PushToLua(sbody);
	return 1;
}

static int l_sbodypath_meta_eq(lua_State *l)
{
	SBodyPath *a = LuaSBodyPath::GetFromLua(1);
	SBodyPath *b = LuaSBodyPath::GetFromLua(2);

	lua_pushboolean(l, *a == *b);
	return 1;
}

template <> const char *LuaObject<LuaUncopyable<SBodyPath> >::s_type = "SystemPath";

template <> void LuaObject<LuaUncopyable<SBodyPath> >::RegisterClass()
{
	static const luaL_reg l_methods[] = {
		{ "New", l_sbodypath_new },

		{ "GetSectorX",     l_sbodypath_get_sector_x     },
		{ "GetSectorY",     l_sbodypath_get_sector_y     },
		{ "GetSystemIndex", l_sbodypath_get_system_index },
		{ "GetBodyId",      l_sbodypath_get_body_id      },

		{ "IsSameSystem", l_sbodypath_is_same_system },

		{ "DistanceTo", l_sbodypath_distance_to },

		{ "GetStarSystem", l_sbodypath_get_star_system },
		{ "GetSystemBody", l_sbodypath_get_system_body },

		{ 0, 0 }
	};

	static const luaL_reg l_meta[] = {
		{ "__eq", l_sbodypath_meta_eq },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, NULL, l_methods, l_meta);
}
