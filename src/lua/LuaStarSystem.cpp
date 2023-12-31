// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "EnumStrings.h"
#include "FileSystem.h"
#include "Game.h"
#include "GameSaveError.h"
#include "LuaConstants.h"
#include "LuaObject.h"
#include "LuaTable.h"
#include "LuaUtils.h"
#include "Pi.h"
#include "Planet.h"
#include "Space.h"
#include "SpaceStation.h"
#include "Star.h"
#include "galaxy/Economy.h"
#include "galaxy/Factions.h"
#include "galaxy/Galaxy.h"
#include "galaxy/GalaxyCache.h"
#include "galaxy/Sector.h"
#include "galaxy/StarSystem.h"

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
	PROFILE_SCOPED()
	LUA_DEBUG_START(l);

	StarSystem *s = LuaObject<StarSystem>::CheckFromLua(1);

	lua_newtable(l);

	for (const SystemBody *station : s->GetSpaceStations()) {
		lua_pushinteger(l, lua_rawlen(l, -1) + 1);
		LuaObject<SystemPath>::PushToLua(&station->GetPath());
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
	PROFILE_SCOPED()
	LUA_DEBUG_START(l);

	StarSystem *s = LuaObject<StarSystem>::CheckFromLua(1);

	lua_newtable(l);

	for (RefCountedPtr<const SystemBody> sb : s->GetBodies()) {
		lua_pushinteger(l, lua_rawlen(l, -1) + 1);
		LuaObject<SystemPath>::PushToLua(&sb->GetPath());
		lua_rawset(l, -3);
	}

	LUA_DEBUG_END(l, 1);

	return 1;
}

static int l_starsystem_get_body_by_path(lua_State *l)
{
	PROFILE_SCOPED()

	StarSystem *s = LuaObject<StarSystem>::CheckFromLua(1);
	const SystemPath *path = LuaObject<SystemPath>::CheckFromLua(2);
	LuaObject<SystemBody>::PushToLua(s->GetBodyByPath(*path));
	return 1;
}

/*
 * Method: GetCommodityBasePriceAlterations
 *
 * Get the price alterations for commodities bought and sold in this system
 *
 * > alteration = system:GetCommodityBasePriceAlterations(commodity)
 *
 * Parameters:
 *
 *   commodity	- The commodity name to look up. Should be a valid name returned
 *                by Economy.GetCommodities()
 * Return:
 *
 *   percentage	- percentage change to the cargo base price. Loosely,
 *                positive values make the commodity more expensive,
 *                indicating it is in demand, while negative values make the
 *                commodity cheaper, indicating a surplus.
 *
 * Availability:
 *
 *   June 2014
 *
 * Status:
 *
 *   experimental
 */
static int l_starsystem_get_commodity_base_price_alterations(lua_State *l)
{
	PROFILE_SCOPED()
	LUA_DEBUG_START(l);

	StarSystem *s = LuaObject<StarSystem>::CheckFromLua(1);
	std::string commodityName = luaL_checkstring(l, 2);

	GalacticEconomy::CommodityId commId = GalacticEconomy::GetCommodityByName(commodityName);
	if (commId != GalacticEconomy::InvalidCommodityId) {
		lua_pushnumber(l, s->GetCommodityBasePriceModPercent(commId));
	} else {
		lua_pushnumber(l, 0);
	}

	LUA_DEBUG_END(l, 1);
	return 1;
}

/*
 * Method: IsCommodityLegal
 *
 * Determine if a given commodity is legal for trade in this system
 *
 * > is_legal = system:IsCommodityLegal(commodity)
 *
 * Parameters:
 *
 *   commodity - the wanted commodity; should be a commodity name returned by Economy.GetCommodities()
 *               (for instance, "hydrogen")
 *
 * Return:
 *
 *   is_legal  - true if the commodity is legal, otherwise false
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
	PROFILE_SCOPED()
	StarSystem *s = LuaObject<StarSystem>::CheckFromLua(1);
	std::string commName = luaL_checkstring(l, 2);

	GalacticEconomy::CommodityId commId = GalacticEconomy::GetCommodityByName(commName);
	if (commId != GalacticEconomy::InvalidCommodityId)
		lua_pushboolean(l, s->IsCommodityLegal(commId));
	else
		lua_pushboolean(l, true);

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
	PROFILE_SCOPED()
	LUA_DEBUG_START(l);

	const StarSystem *s = LuaObject<StarSystem>::CheckFromLua(1);
	const double dist_ly = luaL_checknumber(l, 2);

	bool filter = false;
	if (lua_gettop(l) >= 3) {
		luaL_checktype(l, 3, LUA_TFUNCTION); // any type of function
		filter = true;
	}

	lua_newtable(l);

	const SystemPath &here = s->GetPath();

	const int here_x = here.sectorX;
	const int here_y = here.sectorY;
	const int here_z = here.sectorZ;
	const Uint32 here_idx = here.systemIndex;
	RefCountedPtr<const Sector> here_sec = s->m_galaxy->GetSector(here);

	const int diff_sec = int(ceil(dist_ly / Sector::SIZE));

	uint32_t numSystems = 0;
	for (int x = here_x - diff_sec; x <= here_x + diff_sec; x++) {
		for (int y = here_y - diff_sec; y <= here_y + diff_sec; y++) {
			for (int z = here_z - diff_sec; z <= here_z + diff_sec; z++) {
				RefCountedPtr<const Sector> sec = s->m_galaxy->GetSector(SystemPath(x, y, z));

				for (unsigned int idx = 0; idx < sec->m_systems.size(); idx++) {
					if (x == here_x && y == here_y && z == here_z && idx == here_idx)
						continue;

					if (Sector::DistanceBetween(here_sec, here_idx, sec, idx) > dist_ly)
						continue;

					RefCountedPtr<StarSystem> sys = s->m_galaxy->GetStarSystem(SystemPath(x, y, z, idx));
					if (filter) {
						lua_pushvalue(l, 3);
						LuaObject<StarSystem>::PushToLua(sys.Get());
						lua_call(l, 1, 1);
						if (!lua_toboolean(l, -1)) {
							lua_pop(l, 1);
							continue;
						}
						lua_pop(l, 1);
					}

					lua_pushinteger(l, ++numSystems);
					LuaObject<StarSystem>::PushToLua(sys.Get());
					lua_rawset(l, -3);
				}
			}
		}
	}

	LUA_DEBUG_END(l, 1);

	return 1;
}

static int l_starsystem_get_stars(lua_State *l)
{
	const StarSystem *s = LuaObject<StarSystem>::CheckFromLua(1);
	lua_newtable(l);
	int i = 1;
	for (SystemBody *star : s->GetStars()) {
		lua_pushnumber(l, i++);
		LuaObject<SystemBody>::PushToLua(star);
		lua_settable(l, -3);
	}
	return 1;
}

static int l_starsystem_get_jumpable(lua_State *l)
{
	const StarSystem *s = LuaObject<StarSystem>::CheckFromLua(1);
	lua_newtable(l);
	int i = 1;
	for (RefCountedPtr<SystemBody> sb : s->GetBodies())
		if (sb->IsJumpable()) {
			lua_pushnumber(l, i++);
			LuaObject<SystemBody>::PushToLua(sb.Get());
			lua_settable(l, -3);
		}
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
	PROFILE_SCOPED()
	LUA_DEBUG_START(l);

	StarSystem *s = LuaObject<StarSystem>::CheckFromLua(1);
	const SystemPath *loc1 = &(s->GetPath());

	const SystemPath *loc2 = LuaObject<SystemPath>::GetFromLua(2);
	if (!loc2) {
		StarSystem *s2 = LuaObject<StarSystem>::CheckFromLua(2);
		loc2 = &(s2->GetPath());
	}

	RefCountedPtr<const Sector> sec1 = s->m_galaxy->GetSector(*loc1);
	RefCountedPtr<const Sector> sec2 = s->m_galaxy->GetSector(*loc2);

	// this only works if the SystemPath is valid
	if (loc1->HasValidSystem() && loc2->HasValidSystem()) {
		double dist = Sector::DistanceBetween(sec1, loc1->systemIndex, sec2, loc2->systemIndex);
		lua_pushnumber(l, dist);
	} else {
		lua_pushnumber(l, FLT_MAX);
		return luaL_error(l, "Cannot compare non-systemPaths");
	}

	LUA_DEBUG_END(l, 1);
	return 1;
}

/*
 * Method: ExportToLua
 *
 * Export of generated system for personal interest, customisation, etc
 *
 * Availability:
 *
 *   alpha 33
 *
 * Status:
 *
 *   experimental
 */
static int l_starsystem_export_to_lua(lua_State *l)
{
	PROFILE_SCOPED()
	LUA_DEBUG_START(l);

	StarSystem *s = LuaObject<StarSystem>::CheckFromLua(1);

	static const std::string EXPORTED_SYSTEMS_DIR_NAME("exported_systems");
	if (!FileSystem::userFiles.MakeDirectory(EXPORTED_SYSTEMS_DIR_NAME)) {
		throw CouldNotOpenFileException();
	}

	// construct the filename with folder and extension
	try {
		const std::string filename(EXPORTED_SYSTEMS_DIR_NAME + "/" + FileSystem::SanitiseFileName(s->GetName()) + ".lua");
		const std::string finalPath = FileSystem::NormalisePath(
			FileSystem::JoinPathBelow(FileSystem::GetUserDir(), filename));
		s->ExportToLua(finalPath.c_str());
	} catch (std::invalid_argument &) {
		return luaL_error(l, "could not export system -- name forms an invalid path");
	}

	LUA_DEBUG_END(l, 0);
	return 0;
}

/*
 * Method: Explore
 *
 * Set the star system to be explored by the Player.
 *
 * > system:Explore(time)
 *
 * Parameters:
 *
 *   time - optional, the game time at which the system was explored.
 *          Defaults to current game time.
 *
 * Availability:
 *
 *   October 2014
 *
 * Status:
 *
 *   experimental
 */
static int l_starsystem_explore(lua_State *l)
{
	LUA_DEBUG_START(l);

	StarSystem *s = LuaObject<StarSystem>::CheckFromLua(1);
	double time;
	if (lua_isnumber(l, 2))
		time = luaL_checknumber(l, 2);
	else
		time = Pi::game->GetTime();

	s->ExploreSystem(time);

	LUA_DEBUG_END(l, 0);
	return 0;
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
	PROFILE_SCOPED()
	StarSystem *s = LuaObject<StarSystem>::CheckFromLua(1);
	lua_pushstring(l, s->GetName().c_str());
	return 1;
}

static int l_starsystem_attr_other_names(lua_State *l)
{
	PROFILE_SCOPED();
	StarSystem *s = LuaObject<StarSystem>::CheckFromLua(1);
	LuaTable names(l);
	int i = 1;
	for (std::string n : s->GetOtherNames()) {
		LuaPush(l, i++);
		LuaPush(l, n);
		lua_settable(l, -3);
	}
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
	PROFILE_SCOPED()
	StarSystem *s = LuaObject<StarSystem>::CheckFromLua(1);
	SystemPath path = s->GetPath();
	LuaObject<SystemPath>::PushToLua(&path);
	return 1;
}

/*
 * Attribute: lawlessness
 *
 * The lawlessness value for the system, 0 for peaceful, 1 for raging
 * hordes of pirates
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
	PROFILE_SCOPED()
	StarSystem *s = LuaObject<StarSystem>::CheckFromLua(1);
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
	PROFILE_SCOPED()
	StarSystem *s = LuaObject<StarSystem>::CheckFromLua(1);
	lua_pushnumber(l, s->GetTotalPop().ToDouble());
	return 1;
}

/*
 * Attribute: faction
 *
 * The <Faction> that controls this system
 *
 * Availability:
 *
 *   alpha 28
 *
 * Status:
 *
 *   experimental
 */
static int l_starsystem_attr_faction(lua_State *l)
{
	PROFILE_SCOPED()
	StarSystem *s = LuaObject<StarSystem>::CheckFromLua(1);
	if (s->GetFaction()->IsValid()) {
		LuaObject<Faction>::PushToLua(const_cast<Faction *>(s->GetFaction())); // XXX const-correctness violation
		return 1;
	} else {
		return 0;
	}
}

static int l_starsystem_attr_number_of_stars(lua_State *l)
{
	StarSystem *s = LuaObject<StarSystem>::CheckFromLua(1);
	LuaPush(l, s->GetNumStars());
	return 1;
}

static int l_starsystem_attr_number_of_stations(lua_State *l)
{
	StarSystem *s = LuaObject<StarSystem>::CheckFromLua(1);
	LuaPush(l, s->GetNumSpaceStations());
	return 1;
}

static int l_starsystem_attr_number_of_bodies(lua_State *l)
{
	StarSystem *s = LuaObject<StarSystem>::CheckFromLua(1);
	LuaPush(l, s->GetNumBodies());
	return 1;
}

static int l_starsystem_attr_root_system_body(lua_State *l)
{
	StarSystem *s = LuaObject<StarSystem>::CheckFromLua(1);
	SystemBody *body = s->GetBodyByPath(s->GetRootBody()->GetPath());
	LuaObject<SystemBody>::PushToLua(body);
	return 1;
}

static int l_starsystem_attr_short_description(lua_State *l)
{
	StarSystem *s = LuaObject<StarSystem>::CheckFromLua(1);
	LuaPush(l, s->GetShortDescription());
	return 1;
}

/*
* Attribute: govDescription
*
* The translated description of the system's government type.
*
* Availability:
*
*   November 2020
*
* Status:
*
*   experimental
*/
static int l_starsystem_attr_gov_description(lua_State *l)
{
	StarSystem *s = LuaObject<StarSystem>::CheckFromLua(1);
	LuaPush(l, s->GetSysPolit().GetGovernmentDesc());
	return 1;
}

/*
* Attribute: econDescription
*
* The translated description of the system's economy type.
*
* Availability:
*
*   November 2020
*
* Status:
*
*   experimental
*/
static int l_starsystem_attr_econ_description(lua_State *l)
{
	StarSystem *s = LuaObject<StarSystem>::CheckFromLua(1);
	LuaPush(l, s->GetSysPolit().GetEconomicDesc());
	return 1;
}

/*
* Attribute: govtype
*
* The government type used in the system
* (PolitGovType string constant, EARTHCOLONIAL, EARTHDEMOC, EMPIRERULE, etc.
*
* Availability:
*
*   december 2017
*
* Status:
*
*   experimental
*/
static int l_starsystem_attr_govtype(lua_State *l)
{
	PROFILE_SCOPED()
	StarSystem *s = LuaObject<StarSystem>::CheckFromLua(1);
	lua_pushstring(l, EnumStrings::GetString("PolitGovType", s->GetSysPolit().govType));
	return 1;
}

/*
 * Attribute: explored
 *
 *   If this system has been explored then returns true
 *
 * Availability:
 *
 *   alpha 30
 *
 * Status:
 *
 *   experimental
 */

static int l_starsystem_attr_explored(lua_State *l)
{
	PROFILE_SCOPED()
	StarSystem *s = LuaObject<StarSystem>::CheckFromLua(1);
	lua_pushboolean(l, !s->GetUnexplored());
	return 1;
}

template <>
const char *LuaObject<StarSystem>::s_type = "StarSystem";

template <>
void LuaObject<StarSystem>::RegisterClass()
{
	static const luaL_Reg l_methods[] = {
		{ "GetStationPaths", l_starsystem_get_station_paths },
		{ "GetBodyPaths", l_starsystem_get_body_paths },
		{ "GetBodyByPath", l_starsystem_get_body_by_path },
		{ "GetStars", l_starsystem_get_stars },
		{ "GetJumpable", l_starsystem_get_jumpable },

		{ "GetCommodityBasePriceAlterations", l_starsystem_get_commodity_base_price_alterations },
		{ "IsCommodityLegal", l_starsystem_is_commodity_legal },

		{ "GetNearbySystems", l_starsystem_get_nearby_systems },

		{ "DistanceTo", l_starsystem_distance_to },

		{ "ExportToLua", l_starsystem_export_to_lua },

		{ "Explore", l_starsystem_explore },

		{ 0, 0 }
	};

	static const luaL_Reg l_attrs[] = {
		{ "name", l_starsystem_attr_name },
		{ "other_names", l_starsystem_attr_other_names },
		{ "path", l_starsystem_attr_path },

		{ "lawlessness", l_starsystem_attr_lawlessness },
		{ "population", l_starsystem_attr_population },
		{ "faction", l_starsystem_attr_faction },
		{ "govtype", l_starsystem_attr_govtype },
		{ "explored", l_starsystem_attr_explored },
		{ "numberOfStars", l_starsystem_attr_number_of_stars },
		{ "numberOfStations", l_starsystem_attr_number_of_stations },
		{ "numberOfBodies", l_starsystem_attr_number_of_bodies },
		{ "rootSystemBody", l_starsystem_attr_root_system_body },
		{ "shortDescription", l_starsystem_attr_short_description },
		{ "govDescription", l_starsystem_attr_gov_description },
		{ "econDescription", l_starsystem_attr_econ_description },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, 0, l_methods, l_attrs, 0);
}
