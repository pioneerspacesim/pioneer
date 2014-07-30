// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaFormat.h"
#include "LuaObject.h"
#include "LuaUtils.h"
#include "Lang.h"
#include "StringF.h"
#include "utils.h"
#include <math.h>

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
 * > string = Format.Money(money, showCents)
 *
 * Parameters:
 *
 *   money - a money value, in dollars
 *
 *   showCents - A boolean. If true (default), includes the fractinoal
 *               part of the amount. If false, omitts the fractional
 *               part.
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
	double intpart;
	modf(t*100.0, &intpart);
	if (lua_isboolean(l, 2)){
		bool show_cents = lua_toboolean(l, 2);
		lua_pushstring(l, format_money(intpart, show_cents).c_str());
	}
	else
		lua_pushstring(l, format_money(intpart).c_str());

	return 1;
}

static int l_format_accel_g(lua_State *l)
{
	double a = luaL_checknumber(l, 1);
	const std::string str = stringf(Lang::NUMBER_G, formatarg("acceleration", a));
	lua_pushlstring(l, str.c_str(), str.size());
	return 1;
}

static int l_format_mass_tonnes(lua_State *l)
{
	double t = luaL_checknumber(l, 1);
	const std::string str = stringf(Lang::NUMBER_TONNES, formatarg("mass", t));
	lua_pushlstring(l, str.c_str(), str.size());
	return 1;
}

void LuaFormat::Register()
{
	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);

	static const luaL_Reg l_methods[] = {
		{ "Date",       l_format_date        },
		{ "Distance",   l_format_distance    },
		{ "Money",      l_format_money       },
		{ "AccelG",     l_format_accel_g     },
		{ "MassTonnes", l_format_mass_tonnes },
		{ 0, 0 }
	};

	lua_getfield(l, LUA_REGISTRYINDEX, "CoreImports");
	LuaObjectBase::CreateObject(l_methods, 0, 0);
	lua_setfield(l, -2, "Format");
	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);
}
