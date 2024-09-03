// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaObject.h"
#include "LuaTable.h"
#include "galaxy/GalaxyGenerator.h"
#include "galaxy/Galaxy.h"

/*
 * Class: Galaxy
 *
 * Using this class, you can get information about the stars without even
 * starting the game.
 */

template <>
const char *LuaObject<Galaxy>::s_type = "Galaxy";

/*
 * Method: GetSector
 *
 * > systems = galaxy:GetSector(x, y, z)
 *
 * Parameters:
 *
 *   x, y, z - sector coordinates,
 *
 * Return:
 *
 *   systems - array of objects of type 'StarSystem'
 *
 */
static int l_galaxy_get_sector(lua_State *l)
{
	auto galaxy = LuaObject<Galaxy>::CheckFromLua(1);
	int sx = LuaPull<int>(l, 2);
	int sy = LuaPull<int>(l, 3);
	int sz = LuaPull<int>(l, 4);
	RefCountedPtr<const Sector> sector = galaxy->GetSector(SystemPath(sx, sy, sz));
	LuaTable result(l, sector->m_systems.size(), 0);
	for(unsigned i = 0; i < sector->m_systems.size(); ++i) {
		result.Set(i + 1, galaxy->GetStarSystem(sector->m_systems[i].GetPath()));
	}
	return 1;
}

/*
 * Method: GetStarSystem
 *
 * Get a star system for the system that path points to, return nil if system
 * index is out of range for this sector
 *
 * > system = galaxy:GetStarSystem(path)
 *
 * Parameters:
 *
 *   path - the <SystemPath>
 *
 * Return:
 *
 *   system? - the <StarSystem>
 *
 */
static int l_galaxy_get_star_system(lua_State *l)
{
	auto galaxy = LuaObject<Galaxy>::CheckFromLua(1);
	auto path = LuaObject<SystemPath>::CheckFromLua(2);

	if (path->IsSectorPath()) {
		return luaL_error(l, "Galaxy:GetStarSystem() path argument does not refer to a system");
	}

	RefCountedPtr<const Sector> sector = galaxy->GetSector(SystemPath(path->sectorX, path->sectorY, path->sectorZ));
	if (path->systemIndex >= sector->m_systems.size()) {
		lua_pushnil(l);
		return 1;
	}

	RefCountedPtr<StarSystem> s = galaxy->GetStarSystem(path);
	LuaObject<StarSystem>::PushToLua(s.Get());
	return 1;
}

template <>
void LuaObject<Galaxy>::RegisterClass()
{
	static const luaL_Reg l_methods[] = {
		{ "GetSector", l_galaxy_get_sector },
		{ "GetStarSystem", l_galaxy_get_star_system },
		{ 0, 0 }
	};

	static const luaL_Reg l_attrs[] = {
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, 0, l_methods, l_attrs, 0);
}
