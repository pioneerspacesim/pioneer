// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaVector2.h"

#include "LuaUtils.h"

static int l_vector_new(lua_State *L)
{
	LUA_DEBUG_START(L);
	double x = luaL_checknumber(L, 1);
	double y = luaL_checknumber(L, 2);
	LuaVector2::PushToLua(L, vector2d(x, y));
	LUA_DEBUG_END(L, 1);
	return 1;
}

vector2d construct_vec2(lua_State *L)
{
	double x, y;
	x = luaL_checknumber(L, 2);
	if (lua_gettop(L) == 2)
		y = x;
	else
		y = luaL_checknumber(L, 3);

	return vector2d(x, y);
}

static int l_vector_call(lua_State *L)
{
	LUA_DEBUG_START(L);
	LuaVector2::PushToLua(L, construct_vec2(L));
	LUA_DEBUG_END(L, 1);
	return 1;
}

// Set the values of a vector2 without allocating new memory.
// E.g. instead of
// 		vec = Vector2(1, 2); vec = Vector2(2, 3)
// Use this method
//		vec = Vector2(1, 2); vec(2, 3)
static int l_vector_set(lua_State *L)
{
	LUA_DEBUG_START(L);
	*LuaVector2::CheckFromLua(L, 1) = construct_vec2(L);
	lua_pushvalue(L, 1);
	LUA_DEBUG_END(L, 1);
	return 1;
}

static int l_vector_unit(lua_State *L)
{
	LUA_DEBUG_START(L);
	if (lua_isnumber(L, 1)) {
		double x = luaL_checknumber(L, 1);
		double y = luaL_checknumber(L, 2);
		const vector2d v = vector2d(x, y);
		LuaVector2::PushToLua(L, v.NormalizedSafe());
	} else {
		const vector2d *v = LuaVector2::CheckFromLua(L, 1);
		LuaVector2::PushToLua(L, v->NormalizedSafe());
	}
	LUA_DEBUG_END(L, 1);
	return 1;
}

static int l_vector_tostring(lua_State *L)
{
	const vector2d *v = LuaVector2::CheckFromLua(L, 1);
	luaL_Buffer buf;
	luaL_buffinit(L, &buf);
	char *bufstr = luaL_prepbuffer(&buf);
	int len = snprintf(bufstr, LUAL_BUFFERSIZE, "vector(%g, %g)", v->x, v->y);
	assert(len < LUAL_BUFFERSIZE); // XXX should handle this condition more gracefully
	luaL_addsize(&buf, len);
	luaL_pushresult(&buf);
	return 1;
}

static int l_vector_add(lua_State *L)
{
	const vector2d *a = LuaVector2::CheckFromLua(L, 1);
	const vector2d *b = LuaVector2::CheckFromLua(L, 2);
	LuaVector2::PushToLua(L, *a + *b);
	return 1;
}

static int l_vector_sub(lua_State *L)
{
	const vector2d *a = LuaVector2::CheckFromLua(L, 1);
	const vector2d *b = LuaVector2::CheckFromLua(L, 2);
	LuaVector2::PushToLua(L, *a - *b);
	return 1;
}

static int l_vector_mul(lua_State *L)
{
	if (lua_isnumber(L, 1)) {
		const double s = lua_tonumber(L, 1);
		const vector2d *v = LuaVector2::CheckFromLua(L, 2);
		LuaVector2::PushToLua(L, s * *v);
	} else if (lua_isnumber(L, 2)) {
		const vector2d *v = LuaVector2::CheckFromLua(L, 1);
		const double s = lua_tonumber(L, 2);
		LuaVector2::PushToLua(L, *v * s);
	} else {
		return luaL_error(L, "general vector product doesn't exist; please use dot() or cross()");
	}
	return 1;
}

static int l_vector_div(lua_State *L)
{
	if (lua_isnumber(L, 2)) {
		const vector2d *v = LuaVector2::CheckFromLua(L, 1);
		const double s = lua_tonumber(L, 2);
		LuaVector2::PushToLua(L, *v / s);
		return 1;
	} else if (lua_isnumber(L, 1)) {
		return luaL_error(L, "cannot divide a scalar by a vector");
	} else {
		return luaL_error(L, "vector div not involving a vector (huh?)");
	}
}

static int l_vector_unm(lua_State *L)
{
	const vector2d *v = LuaVector2::CheckFromLua(L, 1);
	LuaVector2::PushToLua(L, -*v);
	return 1;
}

static int l_vector_new_index(lua_State *L)
{
	vector2d *v = LuaVector2::CheckFromLua(L, 1);
	size_t attr_len;
	const char *attr = nullptr;
	if (lua_type(L, 2) == LUA_TSTRING)
		attr = lua_tolstring(L, 2, &attr_len);

	if (attr && attr_len == 1) {
		if (attr[0] == 'x') {
			v->x = luaL_checknumber(L, 3);
		} else if (attr[0] == 'y') {
			v->y = luaL_checknumber(L, 3);
		} else {
			luaL_error(L, "Index '%s' is not available: use 'x' or 'y'", attr);
		}
	} else if (attr) {
		luaL_error(L, "Index '%s' is not available: use 'x' or 'y'", attr);
	} else {
		luaL_error(L, "Attempted to index Vector2 with a non-string type '%s'", luaL_typename(L, 2));
	}

	// __newindex metamethods don't return a value.
	return 0;
}

static int l_vector_index(lua_State *L)
{
	const vector2d *v = LuaVector2::CheckFromLua(L, 1);
	size_t attr_len = 0;
	const char *attr = nullptr;
	if (lua_type(L, 2) == LUA_TSTRING)
		attr = lua_tolstring(L, 2, &attr_len);

	if (attr && attr_len == 1) {
		if (attr[0] == 'x') {
			lua_pushnumber(L, v->x);
			return 1;
		} else if (attr[0] == 'y') {
			lua_pushnumber(L, v->y);
			return 1;
		}
	} else if (attr) {
		lua_getmetatable(L, 1);
		lua_pushvalue(L, 2);
		lua_rawget(L, -2);
		lua_remove(L, -2);
	} else {
		luaL_error(L, "Expected a string as argument, but type is '%s'", luaL_typename(L, 2));
	}
	return 1;
}

static int l_vector_normalised(lua_State *L)
{
	const vector2d *v = LuaVector2::CheckFromLua(L, 1);
	LuaVector2::PushToLua(L, v->NormalizedSafe());
	return 1;
}

static int l_vector_length(lua_State *L)
{
	const vector2d *v = LuaVector2::CheckFromLua(L, 1);
	lua_pushnumber(L, v->Length());
	return 1;
}

static int l_vector_length_sqr(lua_State *L)
{
	const vector2d *v = LuaVector2::CheckFromLua(L, 1);
	lua_pushnumber(L, v->LengthSqr());
	return 1;
}

static int l_vector_angle(lua_State *L)
{
	const vector2d *v = LuaVector2::CheckFromLua(L, 1);
	lua_pushnumber(L, M_PI * 2 - atan2(v->x, v->y));
	return 1;
}

static int l_vector_rotate(lua_State *L)
{
	vector2d *v = LuaVector2::CheckFromLua(L, 1);
	const double angle = luaL_checknumber(L, 2);
	LuaVector2::PushToLua(L, v->Rotate(angle));
	return 1;
}

static int l_vector_rot_90_left(lua_State *L)
{
	const vector2d *v = LuaVector2::CheckFromLua(L, 1);
	const vector2d rot(-v->y, v->x);
	LuaVector2::PushToLua(L, rot);
	return 1;
}

static int l_vector_rot_90_right(lua_State *L)
{
	const vector2d *v = LuaVector2::CheckFromLua(L, 1);
	const vector2d rot(v->y, -v->x);
	LuaVector2::PushToLua(L, rot);
	return 1;
}

static luaL_Reg l_vector_lib[] = {
	{ "new", &l_vector_new },
	{ "unit", &l_vector_unit },
	{ "length", &l_vector_length },
	{ 0, 0 }
};

static luaL_Reg l_vector_meta[] = {
	{ "__tostring", &l_vector_tostring },
	{ "__add", &l_vector_add },
	{ "__sub", &l_vector_sub },
	{ "__mul", &l_vector_mul },
	{ "__div", &l_vector_div },
	{ "__unm", &l_vector_unm },
	{ "__index", &l_vector_index },
	{ "__newindex", &l_vector_new_index },
	{ "__call", &l_vector_set },
	{ "normalised", &l_vector_normalised },
	{ "normalized", &l_vector_normalised },
	{ "unit", &l_vector_unit },
	{ "length", &l_vector_length },
	{ "lengthSqr", &l_vector_length_sqr },
	{ "dot", &l_vector_length_sqr },
	{ "rotate", &l_vector_rotate },
	{ "angle", &l_vector_angle },
	{ "left", &l_vector_rot_90_left },
	{ "right", &l_vector_rot_90_right },
	{ 0, 0 }
};

const char LuaVector2::LibName[] = "Vector2";
const char LuaVector2::TypeName[] = "Vector2";

void LuaVector2::Register(lua_State *L)
{
	LUA_DEBUG_START(L);

	luaL_newlib(L, l_vector_lib);

	lua_newtable(L);
	lua_pushcfunction(L, &l_vector_call);
	lua_setfield(L, -2, "__call");
	lua_setmetatable(L, -2);

	lua_setglobal(L, LuaVector2::LibName);

	luaL_newmetatable(L, LuaVector2::TypeName);
	luaL_setfuncs(L, l_vector_meta, 0);
	// hide the metatable to thwart crazy exploits
	lua_pushboolean(L, 0);
	lua_setfield(L, -2, "__metatable");
	lua_pop(L, 1);

	LUA_DEBUG_END(L, 0);
}

vector2d *LuaVector2::PushNewToLua(lua_State *L)
{
	vector2d *ptr = static_cast<vector2d *>(lua_newuserdata(L, sizeof(vector2d)));
	luaL_setmetatable(L, LuaVector2::TypeName);
	return ptr;
}

const vector2d *LuaVector2::GetFromLua(lua_State *L, int idx)
{
	return static_cast<vector2d *>(luaL_testudata(L, idx, LuaVector2::TypeName));
}

vector2d *LuaVector2::CheckFromLua(lua_State *L, int idx)
{
	return static_cast<vector2d *>(luaL_checkudata(L, idx, LuaVector2::TypeName));
}
