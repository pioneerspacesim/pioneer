#include "LuaObject.h"
#include "LuaUtils.h"
#include "Ship.h"
#include "SpaceStation.h"

static int l_ship_get_label(lua_State *l)
{
	Ship *s = LuaShip::PullFromLua(l);
	lua_pushstring(l, s->GetLabel().c_str());
	return 1;
} 

static int l_ship_get_stats(lua_State *l)
{
	Ship *s = LuaShip::PullFromLua(l);
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
	Ship *s = LuaShip::PullFromLua(l);
	lua_pushnumber(l, s->GetMoney()*0.01);
	return 1;
} 

static int l_ship_set_money(lua_State *l)
{
	Ship *s = LuaShip::PullFromLua(l);
	float m = luaL_checknumber(l, 1);
	s->SetMoney((Sint64)(m*100.0));
	return 0;
} 

static int l_ship_add_money(lua_State *l)
{
	Ship *s = LuaShip::PullFromLua(l);
	float a = luaL_checknumber(l, 1);
	Sint64 m = s->GetMoney() + (Sint64)(a*100.0);
	lua_pushnumber(l, m*0.01);
	return 1;
}

static int l_ship_get_docked_with(lua_State *l)
{
	Ship *s = LuaShip::PullFromLua(l);
	SpaceStation *station = s->GetDockedWith();
	if (!station) return 0;
	LuaSpaceStation::PushToLua(station);
	return 1;
}

static int l_ship_ai_do_kill(lua_State *l)
{
	Ship *s = LuaShip::PullFromLua(l);
	Ship *target = LuaShip::PullFromLua(l);
	s->AIKill(target);
	return 0;
}

static int l_ship_ai_do_flyto(lua_State *l)
{
	/*
	Ship *s = LuaShip::PullFromLua(l);
	Body *target = LuaBody::PullFromLua(l);
	s->AIDoFlyTo(target);
	*/
	return 0;
}

static int l_ship_ai_do_dock(lua_State *l)
{
	Ship *s = LuaShip::PullFromLua(l);
	SpaceStation *target = LuaSpaceStation::PullFromLua(l);
	s->AIDock(target);
	return 0;
}

static int l_ship_ai_do_loworbit(lua_State *l)
{
	/*
	Ship *s = LuaShip::PullFromLua(l);
	Body *target = LuaBody::PullFromLua(l);
	s->AIOrbit(target, 1.1);
	*/
	return 0;
}

static int l_ship_ai_do_mediumorbit(lua_State *l)
{
	/*
	Ship *s = LuaShip::PullFromLua(l);
	Body *target = LuaBody::PullFromLua(l);
	s->AIOrbit(target, 2.0);
	*/
	return 0;
}

static int l_ship_ai_do_highorbit(lua_State *l)
{
	/*
	Ship *s = LuaShip::PullFromLua(l);
	Body *target = LuaBody::PullFromLua(l);
	s->AIOrbit(target, 5.0);
	*/
	return 0;
}

static int l_ship_ai_do_journey(lua_State *l)
{
	/*
	
	XXX is this even required anymore?

	Ship *s = LuaShip::PullFromLua(l);
	SBodyPath *dest = LuaSBodyPath::PullFromLua(l);
	s->AIJourney(dest);
	*/
	return 0;
}

template <> const char *LuaSubObject<Ship>::s_type = "Ship";

template <> const luaL_reg LuaSubObject<Ship>::s_methods[] = {
	{ "get_label", l_ship_get_label },

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

template <> const luaL_reg LuaSubObject<Ship>::s_meta[] = {
	{ 0, 0 }
};
