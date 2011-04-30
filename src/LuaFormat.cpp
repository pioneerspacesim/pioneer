#include "LuaFormat.h"
#include "LuaObject.h"
#include "LuaUtils.h"
#include "Pi.h"
#include "utils.h"

/*
 * Interface: Format
 *
 * String formatting functions for various game values.
 */

/*
 * Function: Date
 *
 * Create a string representation of the given date/time value.
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  experimental
 */
static int l_format_date(lua_State *l)
{
	double t = luaL_checknumber(l, 1);
	lua_pushstring(l, format_date(t).c_str());
	return 1;
}

/*
 * Function: Distance
 *
 * Create a string representation of the given distance value.
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  experimental
 */
static int l_format_distance(lua_State *l)
{
	double t = luaL_checknumber(l, 1);
	lua_pushstring(l, format_distance(t).c_str());
	return 1;
}

/*
 * Function: Money
 *
 * Create a string representation of the given money value.
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  experimental
 */
static int l_format_money(lua_State *l)
{
	double t = luaL_checknumber(l, 1);
	lua_pushstring(l, format_money((Sint64)(t*100.0)).c_str());
	return 1;
}

void LuaFormat::Register()
{
	lua_State *l = Pi::luaManager.GetLuaState();

	LUA_DEBUG_START(l);

	static const luaL_reg methods[] = {
		{ "Date",     l_format_date     },
		{ "Distance", l_format_distance },
		{ "Money",    l_format_money    },
		{ 0, 0 }
	};

	luaL_register(l, "Format", methods);
	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);
}
