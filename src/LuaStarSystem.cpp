#include "LuaObject.h"
#include "LuaStar.h"
#include "LuaPlanet.h"
#include "LuaSpaceStation.h"
#include "LuaStarSystem.h"
#include "LuaSBodyPath.h"
#include "LuaUtils.h"
#include "StarSystem.h"
#include "EquipType.h"
#include "Pi.h"
#include "Space.h"
#include "Star.h"
#include "Planet.h"
#include "SpaceStation.h"
#include "Sector.h"

/*
 * Class: StarSystem
 *
 * Representation of a star system.
 *
 * <StarSystem> holds properties that are used to generate a system when a
 * player enters it, as well as various non-physical attributes relating to
 * the system, eg political and economical data. It typically contains a
 * number of <SystemBody> objects. It does not have a direct relationship to
 * the physics <Body> objects in the system and as such, its possible to acces
 * <StarSystem> objects for systems the player is not currently in.
 *
 * The <StarSystem> for the system the player is currently in is always
 * available via <Game.system>.
 */

/*
 * Method: GetStationPaths
 *
 * Get the <SystemPaths> to stations in this system
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  experimental
 */
static int l_starsystem_get_station_paths(lua_State *l)
{
	LUA_DEBUG_START(l);

	StarSystem *s = LuaStarSystem::GetFromLua(1);
	SysLoc loc = s->GetLocation();

	lua_newtable(l);
	pi_lua_table_ro(l);

	for (std::vector<SBody*>::const_iterator i = s->m_spaceStations.begin(); i != s->m_spaceStations.end(); i++)
	{
		SBodyPath *path = new SBodyPath(loc.GetSectorX(), loc.GetSectorY(), loc.GetSystemNum());
		path->sbodyId = (*i)->id;

		lua_pushinteger(l, lua_objlen(l, -1)+1);
		LuaSBodyPath::PushToLuaGC(path);
		lua_rawset(l, -3);
	}

	LUA_DEBUG_END(l, 1);

	return 1;
}

/*
 * Method: GetCommodityBasePriceAlterations
 *
 * Get the price alterations for cargo items bought and sold in this system
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  experimental
 */
static int l_starsystem_get_commodity_base_price_alterations(lua_State *l)
{
	LUA_DEBUG_START(l);

	StarSystem *s = LuaStarSystem::GetFromLua(1);

	lua_newtable(l);
    pi_lua_table_ro(l);

	for (int e = Equip::FIRST_COMMODITY; e <= Equip::LAST_COMMODITY; e++) {
		lua_pushstring(l, LuaConstants::GetConstantString(l, "EquipType", e));
		lua_pushnumber(l, s->GetCommodityBasePriceModPercent(e));
		lua_rawset(l, -3);
	}
	
	LUA_DEBUG_END(l, 1);
	
	return 1;
}

/*
 * Method: IsCommodityLegal
 *
 * Determine if a given cargo item is legal for trade in this system
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  experimental
 */
static int l_starsystem_is_commodity_legal(lua_State *l)
{
	StarSystem *s = LuaStarSystem::GetFromLua(1);
	Equip::Type e = static_cast<Equip::Type>(LuaConstants::GetConstant(l, "EquipType", luaL_checkstring(l, 2)));
	lua_pushboolean(l, Polit::IsCommodityLegal(s, e));
	return 1;
}

/*
 * Method: GetNearbySystems
 *
 * Get a list of nearby <StarSystems> that match some criteria
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  experimental
 */
static int l_starsystem_get_nearby_systems(lua_State *l)
{
	LUA_DEBUG_START(l);

	StarSystem *s = LuaStarSystem::GetFromLua(1);
	double dist_ly = luaL_checknumber(l, 2);

	bool filter = false;
	if (lua_gettop(l) >= 3) {
		if (!lua_isfunction(l, 3))
			luaL_typerror(l, 3, lua_typename(l, LUA_TFUNCTION));
		filter = true;
	}

	lua_newtable(l);
	pi_lua_table_ro(l);

	int here_x = s->SectorX();
	int here_y = s->SectorY();
	unsigned int here_idx = s->SystemIdx();
	Sector here_sec(here_x, here_y);

	int diff_sec = ceil(dist_ly/Sector::SIZE);

	for (int x = here_x-diff_sec; x <= here_x+diff_sec; x++) {
		for (int y = here_y-diff_sec; y <= here_y+diff_sec; y++) {
			Sector sec(x, y);

			for (unsigned int idx = 0; idx < sec.m_systems.size(); idx++) {
				if (x == here_x && y == here_y && idx == here_idx)
					continue;

				if (Sector::DistanceBetween(&here_sec, here_idx, &sec, idx) > dist_ly)
					continue;

				StarSystem *sys = StarSystem::GetCached(x, y, idx);
				if (filter) {
					lua_pushvalue(l, 3);
					LuaStarSystem::PushToLua(sys);
					lua_call(l, 1, 1);
					if (!lua_toboolean(l, -1)) {
						lua_pop(l, 1);
						sys->Release();
						continue;
					}
					lua_pop(l, 1);
				}

				lua_pushinteger(l, lua_objlen(l, -1)+1);
				LuaStarSystem::PushToLua(sys);
				lua_rawset(l, -3);

				sys->Release();
			}
		}
	}

	LUA_DEBUG_END(l, 1);
	
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
 *  experimental
 */
static int l_starsystem_distance_to(lua_State *l)
{
	LUA_DEBUG_START(l);

	StarSystem *s = LuaStarSystem::GetFromLua(1);
	const SysLoc *loc1 = &(s->GetLocation());

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
 * Attribute: name
 *
 * The name of the system. This is usually the same as the name of the primary
 * star.
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  stable
 */
static int l_starsystem_attr_name(lua_State *l)
{
	StarSystem *s = LuaStarSystem::GetFromLua(1);
	lua_pushstring(l, s->GetName().c_str());
	return 1;
} 

/*
 * Attribute: path
 *
 * The <SystemPath> to the system
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  stable
 */
static int l_starsystem_attr_path(lua_State *l)
{
	StarSystem *s = LuaStarSystem::GetFromLua(1);
	SysLoc loc = s->GetLocation();

	SBodyPath *path = new SBodyPath;
	path->sectorX = loc.sectorX;
	path->sectorY = loc.sectorY;
	path->systemNum = loc.systemNum;
	path->sbodyId = 0;

	LuaSBodyPath::PushToLuaGC(path);

	return 1;
}

/*
 * Attribute: lawlessness
 *
 * The lawlessness value for the system
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  experimental
 */
static int l_starsystem_attr_lawlessness(lua_State *l)
{
	StarSystem *s = LuaStarSystem::GetFromLua(1);
	lua_pushnumber(l, s->GetSysPolit().lawlessness.ToDouble());
	return 1;
}

/*
 * Attribute: population
 *
 * The population of this system
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  experimental
 */
static int l_starsystem_attr_population(lua_State *l)
{
	StarSystem *s = LuaStarSystem::GetFromLua(1);
	lua_pushnumber(l, s->m_totalPop.ToDouble());
	return 1;
}

template <> const char *LuaObject<StarSystem>::s_type = "StarSystem";

template <> void LuaObject<StarSystem>::RegisterClass()
{
	static const luaL_reg l_methods[] = {
		{ "GetStationPaths", l_starsystem_get_station_paths },

		{ "GetCommodityBasePriceAlterations", l_starsystem_get_commodity_base_price_alterations },
		{ "IsCommodityLegal",                 l_starsystem_is_commodity_legal                   },

		{ "GetNearbySystems", l_starsystem_get_nearby_systems },

		{ "DistanceTo", l_starsystem_distance_to },

		{ 0, 0 }
	};

	static const luaL_reg l_attrs[] = {
		{ "name", l_starsystem_attr_name },
		{ "path", l_starsystem_attr_path },

		{ "lawlessness", l_starsystem_attr_lawlessness },
		{ "population",  l_starsystem_attr_population  },

		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, NULL, l_methods, l_attrs, NULL);
}
