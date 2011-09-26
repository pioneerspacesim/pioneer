#include "LuaFormat.h"
#include "LuaObject.h"
#include "LuaUtils.h"
#include "Pi.h"
#include "utils.h"

/*
 * Interface: Format
 *
 * String formatting functions for various game values.
 *
 * While much of the time you can do value conversions yourself its
 * recommended that you use these functions instead to ensure that the values
 * have a consistent display throughout the interface.
 */

/*
 * Function: Date
 *
 * Create a string representation of the given date/time value.
 *
 * > string = Format.Date(date)
 *
 * Parameters:
 *
 *   date - a date/time value, as seconds since 12:00 01-01-3200
 *
 * Return:
 *
 *   string - the string representation
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   stable
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
 * > string = Format.Distance(distance)
 *
 * Parameters:
 *
 *   distance - a distance in metres
 *
 * Return:
 *
 *   string - the string representation
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   stable
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
 * > string = Format.Money(money)
 *
 * Parameters:
 *
 *   money - a money value, in dollars
 *
 * Return:
 *
 *   string - the string representation
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   stable
 */
static int l_format_money(lua_State *l)
{
	double t = luaL_checknumber(l, 1);
	lua_pushstring(l, format_money(Sint64(t*100.0)).c_str());
	return 1;
}

void LuaFormat::Register()
{
	lua_State *l = Pi::luaManager->GetLuaState();

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
