#include "LuaObject.h"
#include "LuaSystemPath.h"
#include "LuaUtils.h"
#include "LuaStarSystem.h"
#include "LuaSBody.h"
#include "galaxy/SystemPath.h"
#include "galaxy/StarSystem.h"
#include "galaxy/Sector.h"

/*
 * Class: SystemPath
 *
 * Describes the location of a system within the galaxy and optionally, a body
 * within that system.
 *
 * A <SystemPath> consists of five components
 *  - the X coordinate of the sector
 *  - the Y coordinate of the sector
 *  - the Z coordinate of the sector
 *  - the system number within that sector
 *  - optionally, the index of a body within that system
 *
 * <SystemPath> objects are typically used to describe the location of a
 * system, space station or other body when specifying hyperspace or other
 * destinations.
 *
 * <SystemPath> objects will compare equal if and only if all five of their
 * components are the same. If you want to see if two paths correspond to the
 * same system without reference to their body indexes, use <IsSameSystem>.
 */

/*
 * Function: New
 *
 * Creates a new <SystemPath> object
 *
 * > path = SystemPath.New(sectorX, sectorY, sectorZ, systemIndex, bodyIndex)
 *
 * Parameters:
 *
 *   sectorX - galactic sector X coordinate
 *
 *   sectorY - galactic sector Y coordinate
 *
 *   sectorZ - galactic sector Z coordinate
 *
 *   systemIndex - optional. the numeric index of the system within the sector
 *
 *   bodyIndex - optional. the numeric index of a specific body within the
 *               system. Defaults to 0, which typically corresponds to the
 *               primary star.
 *
 * Availability:
 *
 *   alpha 10, alpha 13 (updated)
 *
 * Status:
 *
 *   stable
 */
static int l_sbodypath_new(lua_State *l)
{
	Sint32 sector_x = luaL_checkinteger(l, 1);
	Sint32 sector_y = luaL_checkinteger(l, 2);
	Sint32 sector_z = luaL_checkinteger(l, 3);

	SystemPath path(sector_x, sector_y, sector_z);

	if (lua_gettop(l) > 3) {
		path.systemIndex = luaL_checkinteger(l, 4);

		// if this is a system path, then check that the system exists
		Sector s(sector_x, sector_y, sector_z);
		if (size_t(path.systemIndex) >= s.m_systems.size())
			luaL_error(l, "System %d in sector <%d,%d,%d> does not exist", path.systemIndex, sector_x, sector_y, sector_z);

		if (lua_gettop(l) > 4) {
			path.bodyIndex = luaL_checkinteger(l, 5);

			// and if it's a body path, check that the body exists
			RefCountedPtr<StarSystem> sys = StarSystem::GetCached(path);
			if (size_t(path.bodyIndex) >= sys->m_bodies.size()) {
				luaL_error(l, "Body %d in system <%d,%d,%d : %d ('%s')> does not exist",
					path.bodyIndex, sector_x, sector_y, sector_z, path.systemIndex, sys->GetName().c_str());
			}
		}
	}
	LuaSystemPath::PushToLua(&path);
	return 1;
}

/*
 * Method: IsSameSystem
 *
 * Determine if two <SystemPath> objects point to objects in the same system. 
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
 * Method: IsSameSector
 *
 * Determine if two <SystemPath> objects point to objects in the same sector.
 *
 * > is_same = path:IsSameSector(otherpath)
 *
 * Parameters:
 *
 *   otherpath - the <SystemPath> to compare with this path
 *
 * Return:
 *
 *   is_same - true if the path's point to the same sector, false otherwise
 *
 * Availability:
 *
 *   alpha 17
 *
 * Status:
 *
 *   stable
 */
static int l_sbodypath_is_same_sector(lua_State *l)
{
	SystemPath *a = LuaSystemPath::GetFromLua(1);
	SystemPath *b = LuaSystemPath::GetFromLua(2);

	lua_pushboolean(l, a->IsSameSector(b));
	return 1;
}

/*
 * Method: SystemOnly
 *
 * Derive a SystemPath that points to the whole system.
 *
 * > system_path = path:SystemOnly()
 *
 * Return:
 *
 *   system_path - the SystemPath that represents just the system
 *
 * Availability:
 *
 *   alpha 17
 *
 * Status:
 *
 *   stable
 */
static int l_sbodypath_system_only(lua_State *l)
{
	SystemPath *path = LuaSystemPath::GetFromLua(1);
	LuaSystemPath::PushToLuaGC(new SystemPath(path->SystemOnly()));
	return 1;
}

/*
 * Method: SectorOnly
 *
 * Derive a SystemPath that points to the whole sector.
 *
 * > sector_path = path:SectorOnly()
 *
 * Return:
 *
 *   sector_path - the SystemPath that represents just the sector
 *
 * Availability:
 *
 *   alpha 17
 *
 * Status:
 *
 *   stable
 */
static int l_sbodypath_sector_only(lua_State *l)
{
	SystemPath *path = LuaSystemPath::GetFromLua(1);
	LuaSystemPath::PushToLuaGC(new SystemPath(path->SectorOnly()));
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

	Sector sec1(loc1->sectorX, loc1->sectorY, loc1->sectorZ);
	Sector sec2(loc2->sectorX, loc2->sectorY, loc1->sectorZ);
	
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
	RefCountedPtr<StarSystem> s = StarSystem::GetCached(path);
	// LuaStarSystem shares ownership of the StarSystem,
	// because LuaAcquirer<LuaStarSystem> uses IncRefCount and DecRefCount
	LuaStarSystem::PushToLua(s.Get());
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

	if (path->IsSectorPath()) {
		luaL_error(l, "Path <%d,%d,%d> does not name a system or body", path->sectorX, path->sectorY, path->sectorZ);
		return 0;
	}

	RefCountedPtr<StarSystem> sys = StarSystem::GetCached(path);
	if (path->IsSystemPath()) {
		luaL_error(l, "Path <%d,%d,%d : %d ('%s')> does not name a body", path->sectorX, path->sectorY, path->sectorZ, path->systemIndex, sys->GetName().c_str());
		return 0;
	}

	// Lua should never be able to get an invalid SystemPath
	// (note: this may change if it becomes possible to remove systems during the game)
	assert(size_t(path->bodyIndex) < sys->m_bodies.size());

	SBody *sbody = sys->GetBodyByPath(path);
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
 * Attribute: sectorZ
 *
 * The Z component of the path
 *
 * Availability:
 *
 *   alpha 13
 *
 * Status:
 *
 *   stable
 */

static int l_sbodypath_attr_sector_z(lua_State *l)
{
	SystemPath *path = LuaSystemPath::GetFromLua(1);
	lua_pushinteger(l, path->sectorZ);
	return 1;
}

/*
 * Attribute: systemIndex
 *
 * The system index component of the path, or nil if the SystemPath does
 * not point to a system.
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
	if (!path->IsSectorPath())
		lua_pushinteger(l, path->systemIndex);
	else
		lua_pushnil(l);
	return 1;
}

/*
 * Attribute: bodyIndex
 *
 * The body index component of the path, or nil if the SystemPath does
 * not point to a body.
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
	if (path->IsBodyPath())
		lua_pushinteger(l, path->bodyIndex);
	else
		lua_pushnil(l);
	return 1;
}

static int l_sbodypath_meta_eq(lua_State *l)
{
	SystemPath *a = LuaSystemPath::GetFromLua(1);
	SystemPath *b = LuaSystemPath::GetFromLua(2);

	lua_pushboolean(l, *a == *b);
	return 1;
}

static int l_sbodypath_meta_tostring(lua_State *l)
{
	SystemPath *path = LuaSystemPath::GetFromLua(1);
	if (path->IsSectorPath()) {
		lua_pushfstring(l, "<%d,%d,%d>", path->sectorX, path->sectorY, path->sectorZ);
	} else if (path->IsSystemPath()) {
		lua_pushfstring(l, "<%d,%d,%d : %d>",
			path->sectorX, path->sectorY, path->sectorZ,
			path->systemIndex);
	} else {
		assert(path->IsBodyPath());
		lua_pushfstring(l, "<%d,%d,%d : %d, %d>",
			path->sectorX, path->sectorY, path->sectorZ,
			path->systemIndex, path->bodyIndex);
	}
	return 1;
}

template <> const char *LuaObject<LuaUncopyable<SystemPath> >::s_type = "SystemPath";

template <> void LuaObject<LuaUncopyable<SystemPath> >::RegisterClass()
{
	static const luaL_reg l_methods[] = {
		{ "New", l_sbodypath_new },

		{ "IsSameSystem", l_sbodypath_is_same_system },
		{ "IsSameSector", l_sbodypath_is_same_sector },

		{ "SystemOnly", l_sbodypath_system_only },
		{ "SectorOnly", l_sbodypath_sector_only },

		{ "DistanceTo", l_sbodypath_distance_to },

		{ "GetStarSystem", l_sbodypath_get_star_system },
		{ "GetSystemBody", l_sbodypath_get_system_body },

		{ 0, 0 }
	};

	static const luaL_reg l_attrs[] = {
		{ "sectorX",     l_sbodypath_attr_sector_x     },
		{ "sectorY",     l_sbodypath_attr_sector_y     },
		{ "sectorZ",     l_sbodypath_attr_sector_z     },
		{ "systemIndex", l_sbodypath_attr_system_index },
		{ "bodyIndex",   l_sbodypath_attr_body_index   },
		{ 0, 0 }
	};

	static const luaL_reg l_meta[] = {
		{ "__eq",  l_sbodypath_meta_eq },
		{ "__tostring", l_sbodypath_meta_tostring },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, NULL, l_methods, l_attrs, l_meta);
}
