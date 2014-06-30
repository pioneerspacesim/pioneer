// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaObject.h"
#include "LuaConstants.h"
#include "LuaTable.h"
#include "LuaUtils.h"
#include "EnumStrings.h"
#include "LuaUtils.h"
#include "galaxy/StarSystem.h"
#include "EquipType.h"
#include "Pi.h"
#include "Space.h"
#include "Star.h"
#include "Planet.h"
#include "SpaceStation.h"
#include "galaxy/Galaxy.h"
#include "galaxy/Sector.h"
#include "galaxy/GalaxyCache.h"
#include "Factions.h"
#include "FileSystem.h"

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

	for (const SystemBody *station : s->GetSpaceStations())
	{
		lua_pushinteger(l, lua_rawlen(l, -1)+1);
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

	for (RefCountedPtr<const SystemBody> sb : s->GetBodies())
	{
		lua_pushinteger(l, lua_rawlen(l, -1)+1);
		LuaObject<SystemPath>::PushToLua(&sb->GetPath());
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
 * > alteration = system:GetCommodityBasePriceAlterations(cargo_item)
 *
 * Parameters:
 *
 *   cargo_item - The cargo item for which one wants to know the alteration
 * Return:
 *
 *   percentage -  percentage change to the cargo base price. Loosely,
 *                 positive values make the commodity more expensive,
 *                 indicating it is in demand, while negative values make the
 *                 commodity cheaper, indicating a surplus.
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
	LuaTable equip(l, 2);

	if (!equip.CallMethod<bool>("IsValidSlot", "cargo")) {
		luaL_error(l, "GetCommodityBasePriceAlterations takes a valid cargo item as argument.");
		return 0;
	}
	equip.PushValueToStack("l10n_key"); // For now let's just use this poor man's hack.
	Equip::Type e = static_cast<Equip::Type>(LuaConstants::GetConstantFromArg(l, "EquipType", -1));
	lua_pop(l, 1);
	lua_pushnumber(l, s->GetCommodityBasePriceModPercent(e));

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
 *   cargo - the wanted commodity (for instance, Equipment.cargo.hydrogen)
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
	PROFILE_SCOPED()
	StarSystem *s = LuaObject<StarSystem>::CheckFromLua(1);
	// XXX: Don't use the l10n_key hack, this is just UGLY!!
	luaL_checktype(l, 2, LUA_TTABLE);
	LuaTable(l, 2).PushValueToStack("l10n_key");
	Equip::Type e = static_cast<Equip::Type>(LuaConstants::GetConstantFromArg(l, "EquipType", 3));
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
	RefCountedPtr<const Sector> here_sec = Pi::GetGalaxy()->GetSector(here);

	const int diff_sec = int(ceil(dist_ly/Sector::SIZE));

	for (int x = here_x-diff_sec; x <= here_x+diff_sec; x++) {
		for (int y = here_y-diff_sec; y <= here_y+diff_sec; y++) {
			for (int z = here_z-diff_sec; z <= here_z+diff_sec; z++) {
				RefCountedPtr<const Sector> sec = Pi::GetGalaxy()->GetSector(SystemPath(x, y, z));

				for (unsigned int idx = 0; idx < sec->m_systems.size(); idx++) {
					if (x == here_x && y == here_y && z == here_z && idx == here_idx)
						continue;

					if (Sector::DistanceBetween(here_sec, here_idx, sec, idx) > dist_ly)
						continue;

					RefCountedPtr<StarSystem> sys = Pi::GetGalaxy()->GetStarSystem(SystemPath(x, y, z, idx));
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

					lua_pushinteger(l, lua_rawlen(l, -1)+1);
					LuaObject<StarSystem>::PushToLua(sys.Get());
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
	PROFILE_SCOPED()
	LUA_DEBUG_START(l);

	StarSystem *s = LuaObject<StarSystem>::CheckFromLua(1);
	const SystemPath *loc1 = &(s->GetPath());

	const SystemPath *loc2 = LuaObject<SystemPath>::GetFromLua(2);
	if (!loc2) {
		StarSystem *s2 = LuaObject<StarSystem>::CheckFromLua(2);
		loc2 = &(s2->GetPath());
	}

	RefCountedPtr<const Sector> sec1 = Pi::GetGalaxy()->GetSector(*loc1);
	RefCountedPtr<const Sector> sec2 = Pi::GetGalaxy()->GetSector(*loc2);

	double dist = Sector::DistanceBetween(sec1, loc1->systemIndex, sec2, loc2->systemIndex);

	lua_pushnumber(l, dist);

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
		LuaObject<Faction>::PushToLua(s->GetFaction());
		return 1;
	} else {
		return 0;
	}
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

template <> const char *LuaObject<StarSystem>::s_type = "StarSystem";

template <> void LuaObject<StarSystem>::RegisterClass()
{
	static const luaL_Reg l_methods[] = {
		{ "GetStationPaths", l_starsystem_get_station_paths },
		{ "GetBodyPaths", l_starsystem_get_body_paths },

		{ "GetCommodityBasePriceAlterations", l_starsystem_get_commodity_base_price_alterations },
		{ "IsCommodityLegal",                 l_starsystem_is_commodity_legal                   },

		{ "GetNearbySystems", l_starsystem_get_nearby_systems },

		{ "DistanceTo", l_starsystem_distance_to },

		{ "ExportToLua", l_starsystem_export_to_lua },

		{ 0, 0 }
	};

	static const luaL_Reg l_attrs[] = {
		{ "name", l_starsystem_attr_name },
		{ "path", l_starsystem_attr_path },

		{ "lawlessness", l_starsystem_attr_lawlessness },
		{ "population",  l_starsystem_attr_population  },
		{ "faction",     l_starsystem_attr_faction     },
		{ "explored",    l_starsystem_attr_explored    },

		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, 0, l_methods, l_attrs, 0);
}
