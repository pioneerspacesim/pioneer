#include "LuaRand.h"

static int l_rand_new(lua_State *l)
{
	int seed = lua_tonumber(l, 1);
	LuaObject<MTRand>::PushToLuaGC(new MTRand(seed));
	return 1;
}

static int l_rand_number(lua_State *l)
{
	MTRand *rand = LuaObject<MTRand>::GetFromLua(1);

	double n;
	if (lua_gettop(l) == 1)
		n = rand->Double();
	else if(lua_gettop(l) == 2)
		n = rand->Double(luaL_checknumber(l, 2));
	else
		n = rand->Double(luaL_checknumber(l, 2), luaL_checknumber(l, 3));

	lua_pushnumber(l, n);
	return 1;
}

static int l_rand_integer(lua_State *l)
{
	MTRand *rand = LuaObject<MTRand>::GetFromLua(1);

	int n;
	if (lua_gettop(l) == 1)
		n = rand->Int32();
	else if(lua_gettop(l) == 2)
		n = rand->Int32(luaL_checkinteger(l, 2));
	else
		n = rand->Int32(luaL_checkinteger(l, 2), luaL_checkinteger(l, 3));

	lua_pushinteger(l, n);
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

	LuaObjectBase::CreateClass(s_type, NULL, l_methods, NULL);
}
