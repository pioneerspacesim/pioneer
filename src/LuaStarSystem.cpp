#include "LuaObject.h"
#include "LuaStar.h"
#include "LuaPlanet.h"
#include "LuaSpaceStation.h"
#include "LuaUtils.h"
#include "StarSystem.h"
#include "EquipType.h"
#include "Pi.h"
#include "Space.h"
#include "Star.h"
#include "Planet.h"
#include "SpaceStation.h"

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

static int l_starsystem_get_random_starport(lua_State *l)
{
	StarSystem *s = LuaStarSystem::GetFromLua(1);

	SBodyPath *path = new SBodyPath;
	if (s->GetRandomStarport(Pi::rng, path)) {
		LuaSBodyPath::PushToLuaGC(path);
		return 1;
	}

	delete path;
	return 0;
}

// return a body for a sbodypath. not actually a function of starsystem, but
// people think that way so might as well keep the lie alive
static int l_starsystem_get_body(lua_State *l)
{
	LUA_DEBUG_START(l);

	StarSystem *s = LuaStarSystem::GetFromLua(1);
	if (s != Pi::currentSystem) {
		lua_pushstring(l, "get_body can only be called on the current system");
		lua_error(l);
	}

	SBodyPath *path = LuaSBodyPath::GetFromLua(2);
	if (!s->IsSystem(path->sectorX, path->sectorY, path->systemNum)) {
		lua_pushstring(l, "requested body is not in this system");
		lua_error(l);
	}

	Body *b = Space::FindBodyForSBodyPath(path);
	if (!b) return 0;

	switch (b->GetType()) {
		case Object::STAR:
			LuaStar::PushToLua(dynamic_cast<Star*>(b));
			break;
		case Object::PLANET:
			LuaPlanet::PushToLua(dynamic_cast<Planet*>(b));
			break;
		case Object::SPACESTATION:
			LuaSpaceStation::PushToLua(dynamic_cast<SpaceStation*>(b));
			break;
		default: {
			std::string s = stringf(256, "matched path to unknown body type %d", static_cast<int>(b->GetType()));
			lua_pushstring(l, s.c_str());
			lua_error(l);
		}
	}

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

static int l_starsystem_get_random_starport_near_but_not_in(lua_State *l)
{
	StarSystem *s = LuaStarSystem::GetFromLua(1);

	SBodyPath *path = new SBodyPath;
	if (s->GetRandomStarportNearButNotIn(Pi::rng, path)) {
		LuaSBodyPath::PushToLuaGC(path);
		return 1;
	}

	delete path;
	return 0;
}

template <> const char *LuaObject<StarSystem>::s_type = "StarSystem";

template <> void LuaObject<StarSystem>::RegisterClass()
{
	static const luaL_reg l_methods[] = {
		{ "GetName",                          l_starsystem_get_name                             },
		{ "GetPath",                          l_starsystem_get_path                             },
		{ "GetLawlessness",                   l_starsystem_get_lawlessness                      },
		{ "GetPopulation",                    l_starsystem_get_population                       },
		{ "GetCommodityBasePriceAlterations", l_starsystem_get_commodity_base_price_alterations },
		{ "GetRandomStarport",                l_starsystem_get_random_starport                  },
		{ "GetBody",                          l_starsystem_get_body                             },
		{ "IsCommodityLegal",                 l_starsystem_is_commodity_legal                   },
		{ "GetRandomStarportNearButNotIn",    l_starsystem_get_random_starport_near_but_not_in  },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, NULL, l_methods, NULL);
}
