// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaUtils.h"
#include "DateTime.h"
#include "FileSystem.h"
#include "LuaPushPull.h"

#include "FloatComparison.h"

extern "C" {
#include "jenkins/lookup3.h"
}

/*
 * Interface: util
 *
 * Utility functions available in all Lua contexts.
 */

/*
 * Function: hash_random
 *
 * > r = util.hash_random(seed, m, n)
 *
 * Pick a number deterministically according to some input value.
 *
 * > accent = util.hash_random(person_seed .. 'accent', #accents)
 *
 * Parameters:
 *
 *   seed - A string or a number. The output is deterministic based on this value.
 *   m, n - optional. If called as hash_random(seed), the result is in the range 0 <= x < 1.
 *          If called as hash_random(seed, m, n), the result is an integer in the range m <= x <= n.
 *          m must be less than n. (n - m) must be less than 2^32.
 *
 * Availability:
 *
 *   alpha 24
 *
 * Status:
 *
 *   experimental
 */
static int l_hash_random(lua_State *L)
{
	// This function is intended to:
	//  1- Produce a 64-bit hash of the input value (which is either a string
	//     or a double-precision float).
	//  2- Take those 64 bits and use them to pick a random number
	//     (as indicated in the doc comment above).
	// It's also intended to produce the same output for the same input, across
	// any platforms Pioneer runs on, but there may be bugs.

	int numargs = lua_gettop(L);
	// Note according to hashlittle2 comments, hashA is better mixed than hashB.
	Uint32 hashA = 0, hashB = 0;

	luaL_checkany(L, 1);
	switch (lua_type(L, 1)) {
	case LUA_TSTRING: {
		size_t sz;
		const char *str = lua_tolstring(L, 1, &sz);
		// jenkins/lookup3
		lookup3_hashlittle2(str, sz, &hashA, &hashB);
		break;
	}
	case LUA_TNUMBER: {
		double n = lua_tonumber(L, 1);
		assert(!is_nan(n));
		// jenkins/lookup3
		// There are assumptions here that 'double' has the same in-memory
		// representation on all platforms we care about. Also since we're
		// taking a number as input, the source of that number (Lua code)
		// needs to compute it in a way that gives the same result on all
		// platforms, which may be tricky in some cases.
		lookup3_hashlittle2(&n, sizeof(n), &hashA, &hashB);
		break;
	}
	default: return luaL_error(L, "expected a string or a number for argument 1");
	}

	if (numargs == 1) {
		// Generate a value in the range 0 <= x < 1.
		// We have 64 random bits (in hashA and hashB). We take 27 bits from
		// hashA and 26 bits from hashB to give a 53-bit integer
		// (0 to 2**53-1 inclusive)
		// (53 bits chosen because IEEE double precision floats can exactly
		// represent integers up to 2**53).
		// 67108864 = 2**26
		double x = (hashA >> 5) * 67108864.0 + double(hashB >> 6);
		// 9007199254740992 = 2**53
		// Divide by 2**53 to get a value from 0 to (less than) 1.
		x *= 1.0 / 9007199254740992.0;
		// return a value x: 0 <= x < 1
		lua_pushnumber(L, x);
		return 1;
	} else if (numargs == 3) {
		Sint64 m = Sint64(lua_tonumber(L, 2));
		Sint64 n = Sint64(lua_tonumber(L, 3));

		if (m > n) {
			return luaL_error(L, "arguments invalid (m > n not allowed)");
		}

		// Restrict to 32-bit output. This is a bit weird because we allow both signed and unsigned.
		if (m < 0) {
			if (m < INT32_MIN || n > INT32_MAX) {
				return luaL_error(L, "arguments out of range for signed 32-bit int");
			}
		} else {
			if (n > UINT32_MAX) {
				return luaL_error(L, "arguments out of range for unsigned 32-bit int");
			}
		}

		Uint64 range = n - m + 1;
		Uint64 bits = (Uint64(hashB) << 32) | Uint64(hashA);
		// return a value x: m <= x <= n
		lua_pushnumber(L, double(Sint64(m) + Sint64(bits % range)));
		return 1;
	} else {
		return luaL_error(L, "wrong number of arguments");
	}
}

/*
 * Function: trim
 *
 * > s = util.trim(str)
 *
 * Trim leading and/or trailing whitespace from a string.
 *
 * Parameters:
 *
 *   str - A string.
 *
 * Availability:
 *
 *   November 2013
 *
 * Status:
 *
 *   experimental
 */
static int l_trim(lua_State *l)
{
	size_t len;
	const char *str = luaL_checklstring(l, 1, &len);

	if (len == 0 || (!isspace(str[0]) && !isspace(str[len - 1]))) {
		// empty string, or the string beings & ends with non-whitespace
		// just return the same value
		lua_pushvalue(l, 1);
		return 1;
	} else {
		const char *first = str;
		const char *last = str + (len - 1);
		while (len && isspace(*first)) {
			++first;
			--len;
		}
		while (len && isspace(*last)) {
			--last;
			--len;
		}
		lua_pushlstring(l, first, len);
		return 1;
	}
}

/*
 * Function: timePartsToGameTime
 *
 * > gameTime = util.timePartsToGameTime(year, month, day, hour, minute, second)
 *
 * Parameters:
 *
 *   year, month, day, hour, minute, second - numbers
 *
 * Return:
 *
 *   gameTime - number, 0 means 3200-01-01 00:00:00
 *
 * Availability:
 *
 *   2023
 *
 * Status:
 *
 *   experimental
 */
static int l_time_parts_to_game_time(lua_State *L)
{
	Time::DateTime t{
		LuaPull<int>(L, 1), LuaPull<int>(L, 2), LuaPull<int>(L, 3),
		LuaPull<int>(L, 4), LuaPull<int>(L, 5), LuaPull<int>(L, 6)
	};
	LuaPush(L, t.ToGameTime());
	return 1;
}

/*
 * Function: gameTimeToTimeParts
 *
 * > gameTime = util.gameTimeToTimeParts(gameTime)
 *
 * Parameters:
 *
 *   gameTime - number, 0 means 3200-01-01 00:00:00
 *
 * Return:
 *
 *   year, month, day, hour, minute, second - numbers
 *
 * Availability:
 *
 *   2023
 *
 * Status:
 *
 *   experimental
 */
static int l_game_time_to_time_parts(lua_State *L)
{
	Time::DateTime t{ LuaPull<double>(L, 1) };
	int year, month, day, hour, minute, second;
	t.GetDateParts(&year, &month, &day);
	t.GetTimeParts(&hour, &minute, &second);
	LuaPush(L, year); LuaPush(L, month);  LuaPush(L, day);
	LuaPush(L, hour); LuaPush(L, minute); LuaPush(L, second);
	return 6;
}

/*
 * Function: standardGameStartTime
 *
 * > gameTime = util.standardGameStartTime()
 *
 * Returns the time, offset from the current system time by 1200 years.
 *
 * Return:
 *
 *   gameTime - number, 0 means 3200-01-01 00:00:00
 *
 * Availability:
 *
 *   2023
 *
 * Status:
 *
 *   experimental
 */
static int l_standard_game_start_time(lua_State *L)
{
	time_t now;
	time(&now);
	double start_time = difftime(now, 946684799); // <--- Friday, 31 December 1999 23:59:59 GMT+00:00 as UNIX epoch time in seconds
	LuaPush(L, start_time);
	return 1;
}

static const luaL_Reg UTIL_FUNCTIONS[] = {
	{ "trim", l_trim },
	{ "hash_random", l_hash_random },
	{ "timePartsToGameTime", l_time_parts_to_game_time },
	{ "gameTimeToTimeParts", l_game_time_to_time_parts },
	{ "standardGameStartTime", l_standard_game_start_time },
	{ 0, 0 }
};

int luaopen_utils(lua_State *L)
{
	luaL_newlib(L, UTIL_FUNCTIONS);
	return 1;
}

static int l_readonly_table_newindex(lua_State *l)
{
	return luaL_error(l, "attempting to write to a read-only table");
}

static int l_readonly_table_len(lua_State *l)
{
	lua_getuservalue(l, 1);
	lua_pushunsigned(l, lua_rawlen(l, -1));
	return 1;
}

static int l_readonly_table_pairs(lua_State *l)
{
	lua_getglobal(l, "pairs");
	lua_getuservalue(l, 1);
	lua_call(l, 1, 3);
	return 3;
}

static int l_readonly_table_ipairs(lua_State *l)
{
	lua_getglobal(l, "ipairs");
	lua_getuservalue(l, 1);
	lua_call(l, 1, 3);
	return 3;
}

void pi_lua_readonly_table_proxy(lua_State *l, int table_idx)
{
	table_idx = lua_absindex(l, table_idx);

	LUA_DEBUG_START(l);
	lua_newuserdata(l, 0); // proxy
	lua_pushvalue(l, table_idx);
	lua_setuservalue(l, -2);

	lua_createtable(l, 0, 5); // metatable
	lua_pushliteral(l, "__index");
	lua_pushvalue(l, table_idx);
	lua_rawset(l, -3);
	lua_pushliteral(l, "__len");
	lua_pushcfunction(l, &l_readonly_table_len);
	lua_rawset(l, -3);
	lua_pushliteral(l, "__pairs");
	lua_pushcfunction(l, &l_readonly_table_pairs);
	lua_rawset(l, -3);
	lua_pushliteral(l, "__ipairs");
	lua_pushcfunction(l, &l_readonly_table_ipairs);
	lua_rawset(l, -3);
	lua_pushliteral(l, "__newindex");
	lua_pushcfunction(l, &l_readonly_table_newindex);
	lua_rawset(l, -3);
	lua_pushliteral(l, "__metatable");
	lua_pushboolean(l, false);
	lua_rawset(l, -3);

	lua_setmetatable(l, -2);
	LUA_DEBUG_END(l, 1);
}

void pi_lua_readonly_table_original(lua_State *l, int index)
{
	lua_getuservalue(l, index);
}
