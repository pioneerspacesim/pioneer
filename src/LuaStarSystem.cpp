#include "LuaObject.h"
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
	StarSystem *s = LuaStarSystem::PullFromLua();
	lua_pushstring(l, s->GetName().c_str());
	return 1;
} 

static int l_starsystem_get_lawlessness(lua_State *l)
{
	StarSystem *s = LuaStarSystem::PullFromLua();
	LuaFloat::PushToLua(s->GetSysPolit().lawlessness.ToDouble());
	return 1;
}

static int l_starsystem_get_population(lua_State *l)
{
	StarSystem *s = LuaStarSystem::PullFromLua();
	LuaFloat::PushToLua(s->m_totalPop.ToDouble());
	return 1;
}

static int l_starsystem_get_commodity_base_price_alterations(lua_State *l)
{
	StarSystem *s = LuaStarSystem::PullFromLua();

	lua_newtable(l);

	for (int type = Equip::FIRST_COMMODITY; type <= Equip::LAST_COMMODITY; type++)
		pi_lua_settable(l, static_cast<int>(type), s->GetCommodityBasePriceModPercent(type));
	
	return 1;
}

static int l_starsystem_get_random_starport(lua_State *l)
{
	StarSystem *s = LuaStarSystem::PullFromLua();

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
	StarSystem *s = LuaStarSystem::PullFromLua();
	if (s != Pi::currentSystem) {
		LuaString::PushToLua("get_body can only be called on the current system");
		lua_error(l);
	}

	SBodyPath *path = LuaSBodyPath::PullFromLua();
	if (!s->IsSystem(path->sectorX, path->sectorY, path->systemNum)) {
		LuaString::PushToLua("requested body is not in this system");
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
			LuaString::PushToLua(s.c_str());
			lua_error(l);
		}
	}

	return 1;
}

template <> const char *LuaObject<StarSystem>::s_type = "StarSystem";
template <> const char *LuaObject<StarSystem>::s_inherit = NULL;

template <> const luaL_reg LuaObject<StarSystem>::s_methods[] = {
	{ "get_name",                             l_starsystem_get_name },
	{ "get_lawlessness",                      l_starsystem_get_lawlessness },
	{ "get_population",                       l_starsystem_get_population },
	{ "get_commodity_base_price_alterations", l_starsystem_get_commodity_base_price_alterations },
	{ "get_random_starport",                  l_starsystem_get_random_starport },
	{ "get_body",                             l_starsystem_get_body },
	{ 0, 0 }
};

template <> const luaL_reg LuaObject<StarSystem>::s_meta[] = {
	{ 0, 0 }
};
