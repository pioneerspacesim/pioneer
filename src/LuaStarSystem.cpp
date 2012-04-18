#include "LuaObject.h"
#include "LuaStar.h"
#include "LuaPlanet.h"
#include "LuaSpaceStation.h"
#include "LuaStarSystem.h"
#include "LuaSystemPath.h"
#include "LuaUtils.h"
#include "galaxy/StarSystem.h"
#include "EquipType.h"
#include "Pi.h"
#include "Space.h"
#include "Star.h"
#include "Planet.h"
#include "SpaceStation.h"
#include "galaxy/Sector.h"

/*
 * Class: StarSystem
 *
 * Representation of a star system.
 *
 * <StarSystem> holds properties that are used to generate a system when a
 * player enters it, as well as various non-physical attributes relating to
 * the system, eg political and economical data. It typically contains a
 * number of <SystemBody> objects. It does not have a direct relationship to
 * the physics <Body> objects in the system and as such, its possible to
 * access <StarSystem> objects for systems the player is not currently in.
 *
 * The <StarSystem> for the system the player is currently in is always
 * available via <Game.system>.
 */

/*
 * Method: GetStationPaths
 *
 * Get the <SystemPaths> to stations in this system
 *
 * > paths = system:GetStationPaths()
 *
 * Return:
 *
 *   paths - an array of <SystemPath> objects, one for each space station
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   experimental
 */
static int l_starsystem_get_station_paths(lua_State *l)
{
	LUA_DEBUG_START(l);

	StarSystem *s = LuaStarSystem::GetFromLua(1);

	lua_newtable(l);
	pi_lua_table_ro(l);

	for (std::vector<SystemBody*>::const_iterator i = s->m_spaceStations.begin(); i != s->m_spaceStations.end(); i++)
	{
		lua_pushinteger(l, lua_objlen(l, -1)+1);
		LuaSystemPath::PushToLua(&(*i)->path);
		lua_rawset(l, -3);
	}

	LUA_DEBUG_END(l, 1);

	return 1;
}

/*
 * Method: GetBodyPaths
 *
 * Get the <SystemPaths> to bodies (planets, stations, starports) in this system
 *
 * > paths = system:GetBodyPaths()
 *
 * Return:
 *
 *   paths - an array of <SystemPath> objects, one for each <SystemBody>
 *
 * Availability:
 *
 *   alpha 13
 *
 * Status:
 *
 *   experimental
 */
static int l_starsystem_get_body_paths(lua_State *l)
{
	LUA_DEBUG_START(l);

	StarSystem *s = LuaStarSystem::GetFromLua(1);

	lua_newtable(l);
	pi_lua_table_ro(l);

	for (std::vector<SystemBody*>::const_iterator i = s->m_bodies.begin(); i != s->m_bodies.end(); i++)
	{
		lua_pushinteger(l, lua_objlen(l, -1)+1);
		LuaSystemPath::PushToLua(&(*i)->path);
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
 * > alterations = system:GetCommodityBasePriceAlterations()
 *
 * Return:
 *
 *   alterations - a table. The keys are <Constants.EquipType> strings for
 *                 each cargo. The values are numbers that indicate the
 *                 percentage change to each cargo base price. Loosely,
 *                 positive values make the commodity more expensive,
 *                 indicating it is in demand, while negative values make the
 *                 commodity cheaper, indicating a surplus.
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   experimental
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
 * > is_legal = system:IsCommodityLegal(cargo)
 *
 * Parameters:
 *
 *   cargo - a <Constants.EquipType> string for the wanted commodity
 *
 * Return:
 *
 *   is_legal - true if the commodity is legal, otherwise false
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   experimental
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
 * > systems = system:GetNearbySystems(range, filter)
 *
 * Parameters:
 *
 *   range - distance from this system to search, in light years
 *
 *   filter - an optional function. If specified the function will be called
 *            once for each candidate system with the <StarSystem> object
 *            passed as the only parameter. If the filter function returns
 *            true then the system will be included in the array returned by
 *            <GetNearbySystems>, otherwise it will be omitted. If no filter
 *            function is specified then all systems in range are returned.
 *
 * Return:
 *
 *  systems - an array of systems in range that matched the filter
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   experimental
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

	SystemPath here = s->GetPath();

	int here_x = here.sectorX;
	int here_y = here.sectorY;
	int here_z = here.sectorZ;
	Uint32 here_idx = here.systemIndex;
	Sector here_sec(here_x, here_y, here_z);

	int diff_sec = int(ceil(dist_ly/Sector::SIZE));

	for (int x = here_x-diff_sec; x <= here_x+diff_sec; x++) {
		for (int y = here_y-diff_sec; y <= here_y+diff_sec; y++) {
			for (int z = here_z-diff_sec; z <= here_z+diff_sec; z++) {
				Sector sec(x, y, z);

				for (unsigned int idx = 0; idx < sec.m_systems.size(); idx++) {
					if (x == here_x && y == here_y && z == here_z && idx == here_idx)
						continue;

					if (Sector::DistanceBetween(&here_sec, here_idx, &sec, idx) > dist_ly)
						continue;

					RefCountedPtr<StarSystem> sys = StarSystem::GetCached(SystemPath(x, y, z, idx));
					if (filter) {
						lua_pushvalue(l, 3);
						LuaStarSystem::PushToLua(sys.Get());
						lua_call(l, 1, 1);
						if (!lua_toboolean(l, -1)) {
							lua_pop(l, 1);
							continue;
						}
						lua_pop(l, 1);
					}

					lua_pushinteger(l, lua_objlen(l, -1)+1);
					LuaStarSystem::PushToLua(sys.Get());
					lua_rawset(l, -3);
				}
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
 * > dist = system:DistanceTo(system)
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
static int l_starsystem_distance_to(lua_State *l)
{
	LUA_DEBUG_START(l);

	StarSystem *s = LuaStarSystem::GetFromLua(1);
	const SystemPath *loc1 = &(s->GetPath());

	const SystemPath *loc2 = LuaSystemPath::CheckFromLua(2);
	if (!loc2) {
		StarSystem *s2 = LuaStarSystem::GetFromLua(2);
		loc2 = &(s2->GetPath());
	}

	Sector sec1(loc1->sectorX, loc1->sectorY, loc1->sectorZ);
	Sector sec2(loc2->sectorX, loc2->sectorY, loc2->sectorZ);
	
	double dist = Sector::DistanceBetween(&sec1, loc1->systemIndex, &sec2, loc2->systemIndex);

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
 *   alpha 10
 *
 * Status:
 *
 *   stable
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
 *   alpha 10
 *
 * Status:
 *
 *   stable
 */
static int l_starsystem_attr_path(lua_State *l)
{
	StarSystem *s = LuaStarSystem::GetFromLua(1);
	SystemPath path = s->GetPath();
	LuaSystemPath::PushToLua(&path);
	return 1;
}

/*
 * Attribute: lawlessness
 *
 * The lawlessness value for the system
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   experimental
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
 * The population of this system, in billions of people
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   experimental
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
		{ "GetBodyPaths", l_starsystem_get_body_paths },

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
