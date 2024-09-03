// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaObject.h"
#include "Random.h"

extern "C" {
#include "jenkins/lookup3.h"
}

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
 * > rand = Rand.New(seed)
 *
 * Parameters:
 *
 *   seed - optional, the value to seed the generator with. If omitted it will
 *          be set to the current system (not game) time.
 *          seed can be a number or a string.
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
	std::unique_ptr<Random> rng(new Random());
	switch (lua_type(l, 1)) {
	case LUA_TSTRING: {
		size_t sz;
		const char *str = lua_tolstring(l, 1, &sz);

		// Note, these are inputs as well as outputs! They must be initialised.
		Uint32 hashes[2] = { 0u, 0u };
		lookup3_hashlittle2(str, sz, hashes + 0, hashes + 1);
		rng->seed(hashes, 2);
		break;
	}
	case LUA_TNUMBER:
		rng->seed(Uint32(lua_tonumber(l, 1)));
		break;
	case LUA_TNIL: // fallthrough
	case LUA_TNONE:
		rng->seed(Uint32(time(0)));
		break;
	default:
		return luaL_error(l, "seed must be a number or a string");
	}
	LuaObject<Random>::PushToLua(rng.release());
	return 1;
}

/*
 * Method: Number
 *
 * Generates a real (non-integer) number in an open interval
 *
 * > number = rand:Number()
 * > number = rand:Number(max)
 * > number = rand:Number(min, max)
 *
 * Parameters:
 *
 *   min - optional, lower (exclusive) bound for random number.
 *         If omitted, defaults to 0.
 *
 *   max - optional, upper (exclusive) bound for random number.
 *         If omitted, defaults to 1.
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
	Random *rand = LuaObject<Random>::CheckFromLua(1);

	double min, max;
	if (lua_isnumber(l, 2) && lua_isnumber(l, 3)) {
		min = lua_tonumber(l, 2);
		max = lua_tonumber(l, 3);
	} else if (lua_isnumber(l, 2)) {
		min = 0.0;
		max = lua_tonumber(l, 2);
	} else {
		lua_pushnumber(l, rand->Double());
		return 1;
	}

	if (min > max)
		luaL_error(l, "Max must be bigger than min in random number range");

	lua_pushnumber(l, rand->Double(min, max));
	return 1;
}

/*
 * Method: Poisson
 *
 * Generates a random integer drawn from a Poisson distribution.
 *
 * > number = rand:Poisson(lambda)
 *
 * Parameters:
 *
 *   lambda - the mean of the distribution.
 *
 * Return:
 *
 *   number - the random integer number
 *
 * Availability:
 *
 *   November 2014
 *
 * Status:
 *
 *   Experimental
 */
static int l_rand_poisson(lua_State *l)
{
	Random *rand = LuaObject<Random>::CheckFromLua(1);

	double lambda = 0;
	if (lua_isnumber(l, 2))
		lambda = lua_tonumber(l, 2);
	else
		luaL_error(l, "Error reading mean for Poisson");

	lua_pushnumber(l, rand->Poisson(lambda));
	return 1;
}

/*
 * Method: Normal
 *
 * Generates a random number drawn from a Gaussian distribution.
 *
 * > number = rand:Normal()
 * > number = rand:Normal(mean)
 * > number = rand:Normal(mean, stddev)
 *
 * Parameters:
 *
 *   mean - optional, the mean (center) of the distribution. If omitted, defaults to 0.
 *
 *   stddev - optional, the standard deviation (width) of the distribution. If
 *            omitted, defaults to 1.
 *
 * Return:
 *
 *   number - the random number
 *
 * Availability:
 *
 *   January 2014
 *
 * Status:
 *
 *   Experimental
 */
static int l_rand_normal(lua_State *l)
{
	Random *rand = LuaObject<Random>::CheckFromLua(1);

	double mean, stddev;
	if (lua_isnumber(l, 2) && lua_isnumber(l, 3)) {
		mean = lua_tonumber(l, 2);
		stddev = lua_tonumber(l, 3);
	} else if (lua_isnumber(l, 2)) {
		mean = lua_tonumber(l, 2);
		stddev = 1;
	} else {
		lua_pushnumber(l, rand->Normal());
		return 1;
	}

	if (stddev < 0)
		luaL_error(l, "Standard deviation should not be negative.");

	lua_pushnumber(l, rand->Normal(mean, stddev));
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
	Random *rand = LuaObject<Random>::CheckFromLua(1);

	int min, max;
	if (lua_isnumber(l, 2) && lua_isnumber(l, 3)) {
		min = lua_tointeger(l, 2);
		max = lua_tointeger(l, 3);
	} else if (lua_isnumber(l, 2)) {
		min = 0;
		max = lua_tointeger(l, 2);
	} else {
		lua_pushnumber(l, rand->Int32());
		return 1;
	}

	if (min > max)
		luaL_error(l, "Max must be bigger than min in random number range");

	lua_pushnumber(l, rand->Int32(min, max));
	return 1;
}

template <>
const char *LuaObject<Random>::s_type = "Rand";

template <>
void LuaObject<Random>::RegisterClass()
{
	static const luaL_Reg l_methods[] = {
		{ "New", l_rand_new },
		{ "Number", l_rand_number },
		{ "Poisson", l_rand_poisson },
		{ "Normal", l_rand_normal },
		{ "Integer", l_rand_integer },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, 0, l_methods, 0, 0);
}
