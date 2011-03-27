#include "LuaObject.h"
#include "LuaUtils.h"
#include "Ship.h"
#include "SpaceStation.h"

static int l_ship_get_stats(lua_State *l)
{
	Ship *s = LuaShip::PullFromLua();
	const shipstats_t *stats = s->CalcStats();
	
	lua_newtable(l);
	pi_lua_settable(l, "max_capacity",         stats->max_capacity);
	pi_lua_settable(l, "used_capacity",        stats->used_capacity);
	pi_lua_settable(l, "used_cargo",           stats->used_cargo);
	pi_lua_settable(l, "free_capacity",        stats->free_capacity);
	pi_lua_settable(l, "total_mass",           stats->total_mass);
	pi_lua_settable(l, "hull_mass_left",       stats->hull_mass_left);
	pi_lua_settable(l, "hyperspace_range",     stats->hyperspace_range);
	pi_lua_settable(l, "hyperspace_range_max", stats->hyperspace_range_max);
	pi_lua_settable(l, "shield_mass",          stats->shield_mass);
	pi_lua_settable(l, "shield_mass_left",     stats->shield_mass_left);

	return 1;
}

static int l_ship_get_money(lua_State *l)
{
	Ship *s = LuaShip::PullFromLua();
	lua_pushnumber(l, s->GetMoney()*0.01);
	return 1;
} 

static int l_ship_set_money(lua_State *l)
{
	Ship *s = LuaShip::PullFromLua();
	float m = LuaFloat::PullFromLua();
	s->SetMoney((Sint64)(m*100.0));
	return 0;
} 

static int l_ship_add_money(lua_State *l)
{
	Ship *s = LuaShip::PullFromLua();
	float a = LuaFloat::PullFromLua();
	Sint64 m = s->GetMoney() + (Sint64)(a*100.0);
	s->SetMoney(m);
	lua_pushnumber(l, m*0.01);
	return 1;
}

static int l_ship_get_docked_with(lua_State *l)
{
	Ship *s = LuaShip::PullFromLua();
	SpaceStation *station = s->GetDockedWith();
	if (!station) return 0;
	LuaSpaceStation::PushToLua(station);
	return 1;
}

static int l_ship_ai_do_kill(lua_State *l)
{
	Ship *s = LuaShip::PullFromLua();
	Ship *target = LuaShip::PullFromLua();
	s->AIKill(target);
	return 0;
}

static int l_ship_ai_do_flyto(lua_State *l)
{
	/*
	Ship *s = LuaShip::PullFromLua();
	Body *target = LuaBody::PullFromLua();
	s->AIDoFlyTo(target);
	*/
	return 0;
}

static int l_ship_ai_do_dock(lua_State *l)
{
	Ship *s = LuaShip::PullFromLua();
	SpaceStation *target = LuaSpaceStation::PullFromLua();
	s->AIDock(target);
	return 0;
}

static int l_ship_ai_do_loworbit(lua_State *l)
{
	/*
	Ship *s = LuaShip::PullFromLua();
	Body *target = LuaBody::PullFromLua();
	s->AIOrbit(target, 1.1);
	*/
	return 0;
}

static int l_ship_ai_do_mediumorbit(lua_State *l)
{
	/*
	Ship *s = LuaShip::PullFromLua();
	Body *target = LuaBody::PullFromLua();
	s->AIOrbit(target, 2.0);
	*/
	return 0;
}

static int l_ship_ai_do_highorbit(lua_State *l)
{
	/*
	Ship *s = LuaShip::PullFromLua();
	Body *target = LuaBody::PullFromLua();
	s->AIOrbit(target, 5.0);
	*/
	return 0;
}

static int l_ship_ai_do_journey(lua_State *l)
{
	/*
	
	XXX is this even required anymore?

	Ship *s = LuaShip::PullFromLua();
	SBodyPath *dest = LuaSBodyPath::PullFromLua();
	s->AIJourney(dest);
	*/
	return 0;
}

template <> const char *LuaObject<Ship>::s_type = "Ship";
template <> const char *LuaObject<Ship>::s_inherit = "Body";

template <> const luaL_reg LuaObject<Ship>::s_methods[] = {
	{ "get_stats", l_ship_get_stats },

	{ "get_money", l_ship_get_money },
	{ "set_money", l_ship_set_money },
	{ "add_money", l_ship_add_money },

	{ "get_docked_with", l_ship_get_docked_with },

	{ "ai_do_kill",        l_ship_ai_do_kill        },
	{ "ai_do_flyto",       l_ship_ai_do_flyto       },
	{ "ai_do_dock",        l_ship_ai_do_dock        },
	{ "ai_do_loworbit",    l_ship_ai_do_loworbit    },
	{ "ai_do_mediumorbit", l_ship_ai_do_mediumorbit },
	{ "ai_do_highorbit",   l_ship_ai_do_highorbit   },
	{ "ai_do_journey",     l_ship_ai_do_journey     },

	{ 0, 0 }
};

template <> const luaL_reg LuaObject<Ship>::s_meta[] = {
	{ 0, 0 }
};
