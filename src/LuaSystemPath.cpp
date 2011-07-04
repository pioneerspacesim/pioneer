#include "LuaObject.h"
#include "LuaSystemPath.h"
#include "LuaUtils.h"
#include "LuaStarSystem.h"
#include "LuaSBody.h"
#include "SystemPath.h"
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
 * > path = SystemPath.New(sectorX, sectorY, systemIndex, bodyIndex)
 *
 * Parameters:
 *
 *   sectorX - galactic sector X coordinate
 *
 *   sectorY - galactic sector Y coordinate
 *
 *   systemIndex - the numeric index of the system within the sector
 *
 *   bodyIndex - optional, the numeric index of a specific body within the
 *               system. Defaults to 0, which typically corresponds to the
 *               primary star.
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   stable
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
	if (size_t(system_idx) >= s.m_systems.size())
		luaL_error(l, "System %d in sector [%d,%d] does not exist", system_idx, sector_x, sector_y);

	// XXX explode if sbody_id doesn't exist in the target system?
	
	SystemPath *path = new SystemPath(sector_x, sector_y, system_idx, sbody_id);

	LuaSystemPath::PushToLuaGC(path);

	return 1;
}

/*
 * Method: IsSameSystem
 *
 * Determine if two <SystemPath> objects point to the same system, ignoring
 * the body index.
 *
 * > is_same = path:IsSameSystem(otherpath)
 *
 * Parameters:
 *
 *   otherpath - the <SystemPath> to compare with this path
 *
 * Return:
 *
 *   is_same - true if the path's point to the same system, false otherwise
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   stable
 */
static int l_sbodypath_is_same_system(lua_State *l)
{
	SystemPath *a = LuaSystemPath::GetFromLua(1);
	SystemPath *b = LuaSystemPath::GetFromLua(2);

	lua_pushboolean(l, a->IsSameSystem(b));
	return 1;
}

/*
 * Method: DistanceTo
 *
 * Calculate the distance between this and another system
 *
 * > dist = path:DistanceTo(system)
 *
 * Parameters:
 *
 *   system - a <SystemPath> or <StarSystem> to calculate the distance to
 *
 * Return:
 *
 *   dist - the distance, in light years
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   stable
 */
static int l_sbodypath_distance_to(lua_State *l)
{
	LUA_DEBUG_START(l);

	const SystemPath *loc1 = LuaSystemPath::GetFromLua(1);

	const SystemPath *loc2 = LuaSystemPath::CheckFromLua(2);
	if (!loc2) {
		StarSystem *s2 = LuaStarSystem::GetFromLua(2);
		loc2 = &(s2->GetPath());
	}

	Sector sec1(loc1->sectorX, loc1->sectorY);
	Sector sec2(loc2->sectorX, loc2->sectorY);
	
	double dist = Sector::DistanceBetween(&sec1, loc1->systemIndex, &sec2, loc2->systemIndex);

	lua_pushnumber(l, dist);

	LUA_DEBUG_END(l, 1);
	return 1;
}

/*
 * Method: GetStarSystem
 *
 * Get a <StarSystem> object for the system that this path points to
 *
 * > system = path:GetStarSystem()
 *
 * Return:
 *
 *   system - the <StarSystem>
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   stable
 */
static int l_sbodypath_get_star_system(lua_State *l)
{
	SystemPath *path = LuaSystemPath::GetFromLua(1);
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
 * > body = path:GetSystemBody()
 *
 * Return:
 *
 *   body - the <SystemBody>
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   stable
 */
static int l_sbodypath_get_system_body(lua_State *l)
{
	SystemPath *path = LuaSystemPath::GetFromLua(1);
	StarSystem *s = StarSystem::GetCached(path);
	SBody *sbody = s->GetBodyByPath(path);
	s->Release();
	LuaSBody::PushToLua(sbody);
	return 1;
}


/*
 * Attribute: sectorX
 *
 * The X component of the path
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   stable
 */
static int l_sbodypath_attr_sector_x(lua_State *l)
{
	SystemPath *path = LuaSystemPath::GetFromLua(1);
	lua_pushinteger(l, path->sectorX);
	return 1;
}

/*
 * Attribute: sectorY
 *
 * The Y component of the path
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   stable
 */

static int l_sbodypath_attr_sector_y(lua_State *l)
{
	SystemPath *path = LuaSystemPath::GetFromLua(1);
	lua_pushinteger(l, path->sectorY);
	return 1;
}

/*
 * Attribute: systemIndex
 *
 * The system index component of the path
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   stable
 */
static int l_sbodypath_attr_system_index(lua_State *l)
{
	SystemPath *path = LuaSystemPath::GetFromLua(1);
	lua_pushinteger(l, path->systemIndex);
	return 1;
}

/*
 * Attribute: bodyIndex
 *
 * The body index component of the path
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   stable
 */
static int l_sbodypath_attr_body_index(lua_State *l)
{
	SystemPath *path = LuaSystemPath::GetFromLua(1);
	lua_pushinteger(l, path->bodyIndex);
	return 1;
}

static int l_sbodypath_meta_eq(lua_State *l)
{
	SystemPath *a = LuaSystemPath::GetFromLua(1);
	SystemPath *b = LuaSystemPath::GetFromLua(2);

	lua_pushboolean(l, *a == *b);
	return 1;
}

template <> const char *LuaObject<LuaUncopyable<SystemPath> >::s_type = "SystemPath";

template <> void LuaObject<LuaUncopyable<SystemPath> >::RegisterClass()
{
	static const luaL_reg l_methods[] = {
		{ "New", l_sbodypath_new },

		{ "IsSameSystem", l_sbodypath_is_same_system },

		{ "DistanceTo", l_sbodypath_distance_to },

		{ "GetStarSystem", l_sbodypath_get_star_system },
		{ "GetSystemBody", l_sbodypath_get_system_body },

		{ 0, 0 }
	};

	static const luaL_reg l_attrs[] = {
		{ "sectorX",     l_sbodypath_attr_sector_x     },
		{ "sectorY",     l_sbodypath_attr_sector_y     },
		{ "systemIndex", l_sbodypath_attr_system_index },
		{ "bodyIndex",   l_sbodypath_attr_body_index   },
		{ 0, 0 }
	};

	static const luaL_reg l_meta[] = {
		{ "__eq",  l_sbodypath_meta_eq },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, NULL, l_methods, l_attrs, l_meta);
}
