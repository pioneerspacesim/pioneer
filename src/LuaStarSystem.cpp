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

static int l_starsystem_get_name(lua_State *l)
{
	StarSystem *s = LuaStarSystem::GetFromLua(1);
	lua_pushstring(l, s->GetName().c_str());
	return 1;
} 

static int l_starsystem_get_path(lua_State *l)
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

static int l_starsystem_get_lawlessness(lua_State *l)
{
	StarSystem *s = LuaStarSystem::GetFromLua(1);
	lua_pushnumber(l, s->GetSysPolit().lawlessness.ToDouble());
	return 1;
}

static int l_starsystem_get_population(lua_State *l)
{
	StarSystem *s = LuaStarSystem::GetFromLua(1);
	lua_pushnumber(l, s->m_totalPop.ToDouble());
	return 1;
}

static int l_starsystem_get_commodity_base_price_alterations(lua_State *l)
{
	LUA_DEBUG_START(l);

	StarSystem *s = LuaStarSystem::GetFromLua(1);

	lua_newtable(l);

	for (int type = Equip::FIRST_COMMODITY; type <= Equip::LAST_COMMODITY; type++)
		pi_lua_settable(l, static_cast<int>(type), s->GetCommodityBasePriceModPercent(type));
	
	LUA_DEBUG_END(l, 1);
	
	return 1;
}

static int l_starsystem_is_commodity_legal(lua_State *l)
{
	StarSystem *s = LuaStarSystem::GetFromLua(1);
	Equip::Type t = static_cast<Equip::Type>(luaL_checkinteger(l, 2));
	lua_pushboolean(l, Polit::IsCommodityLegal(s, t));
	return 1;
}

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
	int here_idx = s->SystemIdx();
	Sector here_sec(here_x, here_y);

	int diff_sec = ceil(dist_ly/Sector::SIZE);

	for (int x = here_x-diff_sec; x <= here_x+diff_sec; x++) {
		for (int y = here_y-diff_sec; y <= here_y+diff_sec; y++) {
			Sector sec(x, y);

			for (unsigned int idx = 0; idx < sec.m_systems.size(); idx++) {
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

template <> const char *LuaObject<StarSystem>::s_type = "StarSystem";

template <> void LuaObject<StarSystem>::RegisterClass()
{
	static const luaL_reg l_methods[] = {
		{ "GetName", l_starsystem_get_name },
		{ "GetPath", l_starsystem_get_path },

		{ "GetStationPaths", l_starsystem_get_station_paths },

		{ "GetLawlessness",                   l_starsystem_get_lawlessness                      },
		{ "GetPopulation",                    l_starsystem_get_population                       },
		{ "GetCommodityBasePriceAlterations", l_starsystem_get_commodity_base_price_alterations },
		{ "IsCommodityLegal",                 l_starsystem_is_commodity_legal                   },

        { "GetNearbySystems", l_starsystem_get_nearby_systems },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, NULL, l_methods, NULL);
}
