#include "libs.h"
#include "LuaFixed.h"
#include "LuaUtils.h"

extern "C" {
#include "lua/lua.h"
#include "lua/lauxlib.h"
#include "lua/lualib.h"
}

static int l_fixed_new(lua_State *L)
{
	LUA_DEBUG_START(L);
	int num = luaL_checkinteger(L, 1);
	int denom = luaL_checkinteger(L, 2);
	LuaFixed::PushToLua(L, fixed(num, denom));
	LUA_DEBUG_END(L, 1);
	return 1;
}

static int l_fixed_tostring(lua_State *L)
{
	const fixed *v = LuaFixed::CheckFromLua(L, 1);
	luaL_Buffer buf;
	luaL_buffinit(L, &buf);
	char *bufstr = luaL_prepbuffer(&buf);
	int len = snprintf(bufstr, LUAL_BUFFERSIZE, "fixed(%g)", v->ToDouble());
	assert(len < LUAL_BUFFERSIZE); // XXX should handle this condition more gracefully
	luaL_addsize(&buf, len);
	luaL_pushresult(&buf);
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

static luaL_Reg LuaFixed_lib[] = {
	{ "new", &l_fixed_new },
	{ "deg2rad", &l_fixed_deg2rad },
	{ 0, 0 }
};

static luaL_Reg LuaFixed_meta[] = {
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

	// put the 'math' table on the top of the stack
	luaL_register(L, LuaFixed::LibName, LuaFixed_lib);

	luaL_newmetatable(L, LuaFixed::TypeName);
	luaL_register(L, 0, LuaFixed_meta);
	// hide the metatable to thwart crazy exploits
	lua_pushstring(L, "__metatable");
	lua_pushboolean(L, 0);
	lua_rawset(L, -3);
	// map index back to the metatable
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);
	lua_rawset(L, -3);

	lua_pop(L, 2); // pop the metatable and the math library table

	LUA_DEBUG_END(L, 0);
}

void LuaFixed::PushToLua(lua_State *L, const fixed &v)
{
	LUA_DEBUG_START(L);
	fixed *ptr = reinterpret_cast<fixed*>(lua_newuserdata(L, sizeof(fixed)));
	*ptr = v;
	luaL_getmetatable(L, LuaFixed::TypeName);
	lua_setmetatable(L, -2);
	LUA_DEBUG_END(L, 1);
}

const fixed *LuaFixed::GetFromLua(lua_State *L, int idx)
{
	if (lua_type(L, idx) != LUA_TUSERDATA) { return 0; }
	if (!lua_getmetatable(L, idx)) { return 0; }
	lua_getfield(L, LUA_REGISTRYINDEX, LuaFixed::TypeName);
	bool eq = lua_rawequal(L, -1, -2);
	lua_pop(L, 2);
	if (!eq) { return 0; }
	return reinterpret_cast<fixed*>(lua_touserdata(L, idx));
}

const fixed *LuaFixed::CheckFromLua(lua_State *L, int idx)
{
	return reinterpret_cast<fixed*>(luaL_checkudata(L, idx, LuaFixed::TypeName));
}
