#include "LuaRand.h"

static int l_rand_new(lua_State *l)
{
	int seed = time(NULL);
	if (lua_isnumber(l, 1))
		seed = lua_tonumber(l, 1);
	LuaObject<MTRand>::PushToLuaGC(new MTRand(seed));
	return 1;
}

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
		min = 0.0;
		max = 1.0;
	}

	if (min > max)
		luaL_error(l, "Max must be bigger than min in random number range");

	lua_pushnumber(l, rand->Double(min, max));
	return 1;
}

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
		min = 0;
		max = 1;
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
