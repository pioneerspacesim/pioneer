#include "LuaShip.h"

static const char *s_type = "Ship";

const char *LuaShip::GetType() const
{
	return s_type;
}

static int l_ship_get_label(lua_State *l)
{
	Ship *s = unpack_object<Ship*>(l,s_type);
	lua_pushstring(l, s->GetLabel().c_str());
	return 1;
} 

static int l_ship_get_stats(lua_State *l)
{
	Ship *s = unpack_object<Ship*>(l,s_type);
	const shipstats_t *stats = s->CalcStats();
	
	lua_newtable(l);

	lua_pushstring(l, "max_capacity");
	lua_pushinteger(l, stats->max_capacity);
	lua_settable(l, -3);

	lua_pushstring(l, "used_capacity");
	lua_pushinteger(l, stats->used_capacity);
	lua_settable(l, -3);

	lua_pushstring(l, "used_cargo");
	lua_pushinteger(l, stats->used_cargo);
	lua_settable(l, -3);

	lua_pushstring(l, "free_capacity");
	lua_pushinteger(l, stats->free_capacity);
	lua_settable(l, -3);

	lua_pushstring(l, "total_mass");
	lua_pushinteger(l, stats->total_mass);
	lua_settable(l, -3);

	lua_pushstring(l, "hull_mass_left");
	lua_pushnumber(l, stats->hull_mass_left);
	lua_settable(l, -3);

	lua_pushstring(l, "hyperspace_range");
	lua_pushnumber(l, stats->hyperspace_range);
	lua_settable(l, -3);

	lua_pushstring(l, "hyperspace_range_max");
	lua_pushnumber(l, stats->hyperspace_range_max);
	lua_settable(l, -3);

	lua_pushstring(l, "shield_mass");
	lua_pushnumber(l, stats->shield_mass);
	lua_settable(l, -3);

	lua_pushstring(l, "shield_mass_left");
	lua_pushnumber(l, stats->shield_mass_left);
	lua_settable(l, -3);

	return 1;
}

static const luaL_reg s_methods[] = {
	{ "get_label", l_ship_get_label },
	{ "get_stats", l_ship_get_stats },
	{ 0, 0 }
};

static const luaL_reg s_meta[] = {
	{ 0, 0 }
};

void LuaShip::RegisterClass()
{
	CreateClass(s_type, s_methods, s_meta);
}

void LuaShip::PushToLua()
{
	LuaObject::PushToLua(s_type);
}
