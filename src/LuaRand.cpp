#include "LuaRand.h"

/*
 * Class: Rand
 *
 * Class for a random number generator.
 */

/*
 * Function: New
 *
 * Creates a new random number generator.
 *
 * > rand = Rand:New(seed)
 *
 * Parameters:
 *
 *   seed - optional, the value to seed the generator with. If omitted it will
 *          be set to the current system (not game) time
 *
 * Return:
 *
 *   rand - the newly-created generator
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   stable
 */
static int l_rand_new(lua_State *l)
{
	int seed = int(time(NULL));
	if (lua_isnumber(l, 1))
		seed = lua_tointeger(l, 1);
	LuaObject<MTRand>::PushToLuaGC(new MTRand(seed));
	return 1;
}

/*
 * Method: Number
 *
 * Generates a real (non-integer) number.
 *
 * > number = rand:Number()
 * > number = rand:Number(max)
 * > number = rand:Number(min, max)
 *
 * Parameters:
 *
 *   min - optional, the minimum possible value for the generated number. If
 *         omitted, defaults to 0
 *
 *   max - optional, the maximum possible value for the generated number. If
 *         omitted, defaults to a very large number (currently 2^32-1)
 *
 * Return:
 *
 *   number - the random number
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   stable
 */
static int l_rand_number(lua_State *l)
{
	MTRand *rand = LuaObject<MTRand>::GetFromLua(1);

	double min, max;
	if (lua_isnumber(l, 2) && lua_isnumber(l, 3)) {
		min = lua_tonumber(l, 2);
		max = lua_tonumber(l, 3);
	}
	else if (lua_isnumber(l, 2)) {
		min = 0.0;
		max = lua_tonumber(l, 2);
	}
	else {
        lua_pushnumber(l, rand->Double());
        return 1;
	}

	if (min > max)
		luaL_error(l, "Max must be bigger than min in random number range");

	lua_pushnumber(l, rand->Double(min, max));
	return 1;
}

/*
 * Method: Integer
 *
 * Generates an integer number
 *
 * > number = rand:Integer()
 * > number = rand:Integer(max)
 * > number = rand:Integer(min, max)
 *
 * Parameters:
 *
 *   min - optional, the minimum possible value for the generated number. If
 *         omitted, defaults to 0
 *
 *   max - optional, the maximum possible value for the generated number. If
 *         omitted, defaults to a very large number (currently 2^32-1)
 *
 * Return:
 *
 *   number - the random number
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   stable
 */
static int l_rand_integer(lua_State *l)
{
	MTRand *rand = LuaObject<MTRand>::GetFromLua(1);

	int min, max;
	if (lua_isnumber(l, 2) && lua_isnumber(l, 3)) {
		min = lua_tointeger(l, 2);
		max = lua_tointeger(l, 3);
	}
	else if (lua_isnumber(l, 2)) {
		min = 0;
		max = lua_tointeger(l, 2);
	}
	else {
        lua_pushnumber(l, rand->Int32());
        return 1;
	}

	if (min > max)
		luaL_error(l, "Max must be bigger than min in random number range");

	lua_pushnumber(l, rand->Int32(min, max));
	return 1;
}

template <> const char *LuaObject<MTRand>::s_type = "Rand";

template <> void LuaObject<MTRand>::RegisterClass()
{
	static const luaL_reg l_methods[] = {
		{ "New",     l_rand_new     },
		{ "Number",  l_rand_number  },
		{ "Integer", l_rand_integer },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, NULL, l_methods, NULL, NULL);
}
