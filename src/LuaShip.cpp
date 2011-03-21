#include "LuaObject.h"
#include "LuaUtils.h"
#include "Ship.h"

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

template <> const char *LuaSubObject<Ship>::s_type = "Ship";

template <> const luaL_reg LuaSubObject<Ship>::s_methods[] = {
	{ "get_label", l_ship_get_label },
	{ "get_stats", l_ship_get_stats },
	{ 0, 0 }
};

template <> const luaL_reg LuaSubObject<Ship>::s_meta[] = {
	{ 0, 0 }
};
