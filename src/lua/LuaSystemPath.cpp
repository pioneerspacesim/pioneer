// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Game.h"
#include "LuaObject.h"
#include "LuaUtils.h"
#include "Json.h"
#include "Pi.h"
#include "galaxy/Galaxy.h"
#include "galaxy/GalaxyCache.h"
#include "galaxy/Sector.h"
#include "galaxy/StarSystem.h"
#include "galaxy/SystemPath.h"

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

template <>
void LuaObject<SystemPath>::PushToLua(const SystemPath &o)
{
	lua_State *l = Lua::manager->GetLuaState();

	// get the system path object cache
	if (!luaL_getsubtable(l, LUA_REGISTRYINDEX, "SystemPaths")) {
		lua_createtable(l, 0, 1);
		lua_pushliteral(l, "v");
		lua_setfield(l, -2, "__mode");
		lua_setmetatable(l, -2);
	}

	// stack: [SystemPaths]

	// push the system path as a blob to use as a key to look up the actual SystemPath object
	char key_blob[SystemPath::SizeAsBlob];
	o.SerializeToBlob(key_blob);

	lua_pushlstring(l, key_blob, sizeof(key_blob)); // [SystemPaths key]
	lua_pushvalue(l, -1); // [SystemPaths key key]
	lua_rawget(l, -3); // [SystemPaths key value/nil]
	if (lua_isnil(l, -1)) {
		// [SystemPaths key nil]
		lua_pop(l, 1);

		// push a new Lua SystemPath object
		Register(new (LuaObjectBase::Allocate(sizeof(LuaCopyObject<SystemPath>))) LuaCopyObject<SystemPath>(o));

		// store it in the SystemPaths cache, but keep a copy on the stack
		lua_pushvalue(l, -1); // [SystemPaths  key  value  value]
		lua_insert(l, -4); // [value SystemPaths key value]
		lua_rawset(l, -3); // [value SystemPaths]
		lua_pop(l, 1); // [value]
	} else {
		// [SystemPaths key value]
		lua_insert(l, -3); // [value SystemPaths key]
		lua_pop(l, 2); // [value]
	}
}

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

		if (Pi::game) {
			// if this is a system path, then check that the system exists
			RefCountedPtr<const Sector> s = Pi::game->GetGalaxy()->GetSector(path);
			if (size_t(path.systemIndex) >= s->m_systems.size())
				luaL_error(l, "System %d in sector <%d,%d,%d> does not exist", path.systemIndex, sector_x, sector_y, sector_z);
		}
		if (lua_gettop(l) > 4) {
			path.bodyIndex = luaL_checkinteger(l, 5);

			if (Pi::game) {
				// and if it's a body path, check that the body exists
				RefCountedPtr<StarSystem> sys = Pi::game->GetGalaxy()->GetStarSystem(path);
				if (path.bodyIndex >= sys->GetNumBodies()) {
					luaL_error(l, "Body %d in system <%d,%d,%d : %d ('%s')> does not exist",
						path.bodyIndex, sector_x, sector_y, sector_z, path.systemIndex, sys->GetName().c_str());
				}
			}
		}
	}
	LuaObject<SystemPath>::PushToLua(path);
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
	SystemPath *a = LuaObject<SystemPath>::CheckFromLua(1);
	SystemPath *b = LuaObject<SystemPath>::CheckFromLua(2);

	if (!a->HasValidSystem())
		return luaL_error(l, "SystemPath:IsSameSystem() self argument does not refer to a system");
	if (!b->HasValidSystem())
		return luaL_error(l, "SystemPath:IsSameSystem() argument #1 does not refer to a system");

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
	SystemPath *a = LuaObject<SystemPath>::CheckFromLua(1);
	SystemPath *b = LuaObject<SystemPath>::CheckFromLua(2);

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
	SystemPath *path = LuaObject<SystemPath>::CheckFromLua(1);
	if (!path->HasValidSystem())
		return luaL_error(l, "SystemPath:SystemOnly() self argument does not refer to a system");
	if (path->IsSystemPath()) {
		return 1;
	}
	const SystemPath sysOnly(path->SystemOnly());
	LuaObject<SystemPath>::PushToLua(sysOnly);
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
	SystemPath *path = LuaObject<SystemPath>::CheckFromLua(1);
	if (path->IsSectorPath()) {
		return 1;
	}
	LuaObject<SystemPath>::PushToLua(path->SectorOnly());
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

	const SystemPath *loc1 = LuaObject<SystemPath>::CheckFromLua(1);

	const SystemPath *loc2 = LuaObject<SystemPath>::GetFromLua(2);
	if (!loc2) {
		StarSystem *s2 = LuaObject<StarSystem>::CheckFromLua(2);
		loc2 = &(s2->GetPath());
		assert(loc2->HasValidSystem());
	}

	if (!loc1->HasValidSystem())
		return luaL_error(l, "SystemPath:DistanceTo() self argument does not refer to a system");
	if (!loc2->HasValidSystem())
		return luaL_error(l, "SystemPath:DistanceTo() argument #1 does not refer to a system");

	RefCountedPtr<const Sector> sec1 = Pi::game->GetGalaxy()->GetSector(*loc1);
	RefCountedPtr<const Sector> sec2 = Pi::game->GetGalaxy()->GetSector(*loc2);

	double dist = Sector::DistanceBetween(sec1, loc1->systemIndex, sec2, loc2->systemIndex);

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
	SystemPath *path = LuaObject<SystemPath>::CheckFromLua(1);

	if (path->IsSectorPath())
		return luaL_error(l, "SystemPath:GetStarSystem() self argument does not refer to a system");

	RefCountedPtr<StarSystem> s = Pi::game->GetGalaxy()->GetStarSystem(path);
	// LuaObject<StarSystem> shares ownership of the StarSystem,
	// because LuaAcquirer<LuaObject<StarSystem>> uses IncRefCount and DecRefCount
	LuaObject<StarSystem>::PushToLua(s.Get());
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
	SystemPath *path = LuaObject<SystemPath>::CheckFromLua(1);

	if (path->IsSectorPath()) {
		luaL_error(l, "Path <%d,%d,%d> does not name a system or body", path->sectorX, path->sectorY, path->sectorZ);
		return 0;
	}

	RefCountedPtr<StarSystem> sys = Pi::game->GetGalaxy()->GetStarSystem(path);
	if (path->IsSystemPath()) {
		luaL_error(l, "Path <%d,%d,%d : %d ('%s')> does not name a body", path->sectorX, path->sectorY, path->sectorZ, path->systemIndex, sys->GetName().c_str());
		return 0;
	}

	// Lua should never be able to get an invalid SystemPath
	// (note: this may change if it becomes possible to remove systems during the game)
	assert(path->bodyIndex < sys->GetNumBodies());

	SystemBody *sbody = sys->GetBodyByPath(path);
	LuaObject<SystemBody>::PushToLua(sbody);
	return 1;
}

static int l_sbodypath_is_body_path(lua_State *l)
{
	SystemPath *path = LuaObject<SystemPath>::CheckFromLua(1);
	LuaPush(l, path->IsBodyPath());
	return 1;
}

static int l_sbodypath_is_sector_path(lua_State *l)
{
	SystemPath *path = LuaObject<SystemPath>::CheckFromLua(1);
	LuaPush(l, path->IsSectorPath());
	return 1;
}

static int l_sbodypath_is_system_path(lua_State *l)
{
	SystemPath *path = LuaObject<SystemPath>::CheckFromLua(1);
	LuaPush(l, path->IsSystemPath());
	return 1;
}

/*
* Method: ParseString
*
* Parse a string and try to make a SystemPath from it
*
* > sector_path = SystemPath.Parse()
*
* Return:
*
*   sector_path - the SystemPath that represents just the sector
*
* Availability:
*
*   2018-06-16
*
* Status:
*
*   experimental
*/
static int l_sbodypath_parse_string(lua_State *l)
{
	std::string path = LuaPull<std::string>(l, 1);
	try {
		SystemPath syspath = SystemPath::Parse(path.c_str());
		LuaObject<SystemPath>::PushToLua(syspath.SectorOnly());
		return 1;
	} catch (const SystemPath::ParseFailure &) {
		return 0;
	}
	return 0;
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
	SystemPath *path = LuaObject<SystemPath>::CheckFromLua(1);
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
	SystemPath *path = LuaObject<SystemPath>::CheckFromLua(1);
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
	SystemPath *path = LuaObject<SystemPath>::CheckFromLua(1);
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
	SystemPath *path = LuaObject<SystemPath>::CheckFromLua(1);
	if (path->HasValidSystem())
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
	SystemPath *path = LuaObject<SystemPath>::CheckFromLua(1);
	if (path->HasValidBody())
		lua_pushinteger(l, path->bodyIndex);
	else
		lua_pushnil(l);
	return 1;
}

static int l_sbodypath_meta_tostring(lua_State *l)
{
	SystemPath *path = LuaObject<SystemPath>::CheckFromLua(1);
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

static bool _systempath_to_json(lua_State *l, Json &out)
{
	auto *p = LuaObject<SystemPath>::GetFromLua(-1);
	if (!p) return false;

	out = Json::array();
	out[0] = Json(p->sectorX);
	out[1] = Json(p->sectorY);
	out[2] = Json(p->sectorZ);
	out[3] = Json(p->systemIndex);
	out[4] = Json(p->bodyIndex);
	return true;
}

static bool _systempath_from_json(lua_State *l, const Json &obj)
{
	if (!obj.is_array()) return false;
	if (obj.size() < 3 || obj.size() > 5) return false;
	for (size_t i = 0; i < obj.size(); ++i) {
		if (!obj[i].is_number_integer()) {
			return false;
		}
	}

	SystemPath p;
	p.sectorX = obj[0];
	p.sectorY = obj[1];
	p.sectorZ = obj[2];
	if (obj.size() >= 4) {
		p.systemIndex = obj[3];
	}
	if (obj.size() >= 5) {
		p.bodyIndex = obj[4];
	}

	LuaObject<SystemPath>::PushToLua(p);
	return true;
}

template <>
const char *LuaObject<SystemPath>::s_type = "SystemPath";

template <>
void LuaObject<SystemPath>::RegisterClass()
{
	static const luaL_Reg l_methods[] = {
		{ "New", l_sbodypath_new },

		{ "IsSameSystem", l_sbodypath_is_same_system },
		{ "IsSameSector", l_sbodypath_is_same_sector },

		{ "SystemOnly", l_sbodypath_system_only },
		{ "SectorOnly", l_sbodypath_sector_only },

		{ "DistanceTo", l_sbodypath_distance_to },

		{ "GetStarSystem", l_sbodypath_get_star_system },
		{ "GetSystemBody", l_sbodypath_get_system_body },
		{ "IsSystemPath", l_sbodypath_is_system_path },
		{ "IsSectorPath", l_sbodypath_is_sector_path },
		{ "IsBodyPath", l_sbodypath_is_body_path },
		{ "ParseString", l_sbodypath_parse_string },
		{ 0, 0 }
	};

	static const luaL_Reg l_attrs[] = {
		{ "sectorX", l_sbodypath_attr_sector_x },
		{ "sectorY", l_sbodypath_attr_sector_y },
		{ "sectorZ", l_sbodypath_attr_sector_z },
		{ "systemIndex", l_sbodypath_attr_system_index },
		{ "bodyIndex", l_sbodypath_attr_body_index },
		{ 0, 0 }
	};

	static const luaL_Reg l_meta[] = {
		{ "__tostring", l_sbodypath_meta_tostring },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, 0, l_methods, l_attrs, l_meta);
	LuaObjectBase::RegisterSerializer(s_type, SerializerPair(_systempath_to_json, _systempath_from_json));
}
