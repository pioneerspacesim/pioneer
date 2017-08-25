// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "LuaFixed.h"
#include "LuaUtils.h"

static int l_fixed_new(lua_State *L)
{
	LUA_DEBUG_START(L);
	Sint64 num = (Sint64) luaL_checknumber(L, 1);
	Sint64 denom = (Sint64) luaL_checknumber(L, 2);		// use checknumber for >32-bit precision
	if (!denom)
		return luaL_error(L, "cannot construct a fixed-point value with a zero denominator");
	LuaFixed::PushToLua(L, fixed(num, denom));
	LUA_DEBUG_END(L, 1);
	return 1;
}

static int l_fixed_tostring(lua_State *L)
{
	const fixed *v = LuaFixed::CheckFromLua(L, 1);
	lua_pushfstring(L, "fixed(%f)", lua_Number(v->ToDouble()));
	return 1;
}

static int l_fixed_add(lua_State *L)
{
	const fixed *a = LuaFixed::CheckFromLua(L, 1);
	const fixed *b = LuaFixed::CheckFromLua(L, 2);
	LuaFixed::PushToLua(L, *a + *b);
	return 1;
}

static int l_fixed_sub(lua_State *L)
{
	const fixed *a = LuaFixed::CheckFromLua(L, 1);
	const fixed *b = LuaFixed::CheckFromLua(L, 2);
	LuaFixed::PushToLua(L, *a - *b);
	return 1;
}

static int l_fixed_mul(lua_State *L)
{
	const fixed *a = LuaFixed::CheckFromLua(L, 1);
	const fixed *b = LuaFixed::CheckFromLua(L, 2);
	LuaFixed::PushToLua(L, *a * *b);
	return 1;
}

static int l_fixed_div(lua_State *L)
{
	const fixed *a = LuaFixed::CheckFromLua(L, 1);
	const fixed *b = LuaFixed::CheckFromLua(L, 2);
	LuaFixed::PushToLua(L, *a / *b);
	return 1;
}

static int l_fixed_unm(lua_State *L)
{
	const fixed *v = LuaFixed::CheckFromLua(L, 1);
	LuaFixed::PushToLua(L, -*v);
	return 1;
}

static int l_fixed_eq(lua_State *L)
{
	const fixed *a = LuaFixed::CheckFromLua(L, 1);
	const fixed *b = LuaFixed::CheckFromLua(L, 2);
	lua_pushboolean(L, *a == *b);
	return 1;
}

static int l_fixed_lt(lua_State *L)
{
	const fixed *a = LuaFixed::CheckFromLua(L, 1);
	const fixed *b = LuaFixed::CheckFromLua(L, 2);
	lua_pushboolean(L, *a < *b);
	return 1;
}

static int l_fixed_le(lua_State *L)
{
	const fixed *a = LuaFixed::CheckFromLua(L, 1);
	const fixed *b = LuaFixed::CheckFromLua(L, 2);
	lua_pushboolean(L, *a <= *b);
	return 1;
}

static int l_fixed_tonumber(lua_State *L)
{
	const fixed *v = LuaFixed::CheckFromLua(L, 1);
	lua_pushnumber(L, v->ToDouble());
	return 1;
}

static int l_fixed_deg2rad(lua_State *L)
{
	const fixed *v = LuaFixed::CheckFromLua(L, 1);
	LuaFixed::PushToLua(L, (*v) * fixed(31416,1800000));
	return 1;
}

static luaL_Reg l_fixed_lib[] = {
	{ "new", &l_fixed_new },
	{ "deg2rad", &l_fixed_deg2rad },
	{ 0, 0 }
};

static luaL_Reg l_fixed_meta[] = {
	{ "__tostring", &l_fixed_tostring },
	{ "__add", &l_fixed_add },
	{ "__sub", &l_fixed_sub },
	{ "__mul", &l_fixed_mul },
	{ "__div", &l_fixed_div },
	{ "__unm", &l_fixed_unm },
	{ "__eq", &l_fixed_eq },
	{ "__lt", &l_fixed_lt },
	{ "__le", &l_fixed_le },
	{ "tonumber", &l_fixed_tonumber },
	{ 0, 0 }
};

const char LuaFixed::LibName[] = "fixed";
const char LuaFixed::TypeName[] = "fixed";

void LuaFixed::Register(lua_State *L)
{
	LUA_DEBUG_START(L);

	luaL_newlib(L, l_fixed_lib);
	lua_setglobal(L, LuaFixed::LibName);

	luaL_newmetatable(L, LuaFixed::TypeName);
	luaL_setfuncs(L, l_fixed_meta, 0);
	// hide the metatable to thwart crazy exploits
	lua_pushboolean(L, 0);
	lua_setfield(L, -2, "__metatable");
	// map index back to the metatable
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	// pop the metatable
	lua_pop(L, 1);

	LUA_DEBUG_END(L, 0);
}

void LuaFixed::PushToLua(lua_State *L, const fixed &v)
{
	LUA_DEBUG_START(L);
	fixed *ptr = static_cast<fixed*>(lua_newuserdata(L, sizeof(fixed)));
	*ptr = v;
	luaL_setmetatable(L, LuaFixed::TypeName);
	LUA_DEBUG_END(L, 1);
}

const fixed *LuaFixed::GetFromLua(lua_State *L, int idx)
{
	return static_cast<fixed*>(luaL_testudata(L, idx, LuaFixed::TypeName));
}

const fixed *LuaFixed::CheckFromLua(lua_State *L, int idx)
{
	return static_cast<fixed*>(luaL_checkudata(L, idx, LuaFixed::TypeName));
}
