// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaColor.h"
#include "LuaUtils.h"
#include "libs.h"

inline Color4ub ColorClamp(float r, float g, float b, float a)
{
	r = std::max(std::min(r, 255.0f), 0.0f);
	g = std::max(std::min(g, 255.0f), 0.0f);
	b = std::max(std::min(b, 255.0f), 0.0f);
	a = std::max(std::min(a, 255.0f), 0.0f);
	return Color4ub(r, g, b, a);
}

static int l_color_new(lua_State *L)
{
	LUA_DEBUG_START(L);
	double r = luaL_checknumber(L, 1);
	double g = luaL_checknumber(L, 2);
	double b = luaL_checknumber(L, 3);
	double a = luaL_optnumber(L, 4, 255.0);
	LuaColor::PushToLua(L, ColorClamp(r, g, b, a));
	LUA_DEBUG_END(L, 1);
	return 1;
}

static int l_color_call(lua_State *L)
{
	LUA_DEBUG_START(L);
	double r = luaL_checknumber(L, 2);
	double g = luaL_checknumber(L, 3);
	double b = luaL_checknumber(L, 4);
	double a = luaL_optnumber(L, 5, 255.0);
	LuaColor::PushToLua(L, ColorClamp(r, g, b, a));
	LUA_DEBUG_END(L, 1);
	return 1;
}

static int l_color_tostring(lua_State *L)
{
	const Color4ub *c = LuaColor::CheckFromLua(L, 1);
	luaL_Buffer buf;
	luaL_buffinit(L, &buf);
	char *bufstr = luaL_prepbuffer(&buf);
	int len = snprintf(bufstr, LUAL_BUFFERSIZE, "Color(%i, %i, %i, %i)", c->r, c->g, c->b, c->a);
	assert(len < LUAL_BUFFERSIZE); // XXX should handle this condition more gracefully
	luaL_addsize(&buf, len);
	luaL_pushresult(&buf);
	return 1;
}

static int l_color_add(lua_State *L)
{
	const Color4ub *a = LuaColor::CheckFromLua(L, 1);
	const Color4ub *b = LuaColor::CheckFromLua(L, 2);
	LuaColor::PushToLua(L, *a + *b);
	return 1;
}

static int l_color_mul(lua_State *L)
{
	if (lua_isnumber(L, 1)) {
		const double s = lua_tonumber(L, 1);
		const Color4ub *c = LuaColor::CheckFromLua(L, 2);
		LuaColor::PushToLua(L, *c * s);
	} else if (lua_isnumber(L, 2)) {
		const Color4ub *c = LuaColor::CheckFromLua(L, 1);
		const double s = lua_tonumber(L, 2);
		LuaColor::PushToLua(L, *c * s);
	} else {
		return luaL_error(L, "Attempt to multiply a Color with an incompatible value (%s * %s)", luaL_typename(L, 1), luaL_typename(L, 2));
	}
	return 1;
}

static int l_color_new_index(lua_State *L)
{
	Color4ub *c = LuaColor::CheckFromLua(L, 1);
	size_t attr_len;
	const char *attr = nullptr;
	if (lua_type(L, 2) == LUA_TSTRING)
		attr = lua_tolstring(L, 2, &attr_len);

	if (attr && attr_len == 1) {
		if (attr[0] == 'r') {
			c->r = luaL_checknumber(L, 3);
		} else if (attr[0] == 'g') {
			c->g = luaL_checknumber(L, 3);
		} else if (attr[0] == 'b') {
			c->b = luaL_checknumber(L, 3);
		} else if (attr[0] == 'a') {
			c->a = luaL_checknumber(L, 3);
		} else {
			luaL_error(L, "Index '%s' is not available: use 'r', 'g', 'b' or 'a'", attr);
		}
	} else if (attr) {
		luaL_error(L, "Index '%s' is not available: use 'r', 'g', 'b' or 'a'", attr);
	} else {
		luaL_error(L, "Attempted to index Color with a non-string type '%s'.", luaL_typename(L, 2));
	}

	// the __newindex metamethod returns nothing
	return 0;
}

static int l_color_index(lua_State *L)
{
	const Color4ub *c = LuaColor::CheckFromLua(L, 1);
	size_t attr_len = 0;
	const char *attr = nullptr;
	if (lua_type(L, 2) == LUA_TSTRING)
		attr = lua_tolstring(L, 2, &attr_len);

	if (attr && attr_len == 1) {
		if (attr[0] == 'r') {
			lua_pushnumber(L, c->r);
			return 1;
		} else if (attr[0] == 'g') {
			lua_pushnumber(L, c->g);
			return 1;
		} else if (attr[0] == 'b') {
			lua_pushnumber(L, c->b);
			return 1;
		} else if (attr[0] == 'a') {
			lua_pushnumber(L, c->a);
			return 1;
		}
	} else if (attr) {
		lua_getmetatable(L, 1);
		lua_pushvalue(L, 2);
		lua_rawget(L, -2);
		lua_remove(L, -2);
	} else {
		luaL_error(L, "Attempted to index Color with a non-string type '%s'", luaL_typename(L, 2));
	}
	return 1;
}

static int l_color_shade(lua_State *L)
{
	const Color4ub *c = LuaColor::CheckFromLua(L, 1);
	float factor = luaL_checknumber(L, 2);
	Color4ub r(*c);
	r.Shade(factor);
	LuaColor::PushToLua(L, r);
	return 1;
}

static int l_color_tint(lua_State *L)
{
	const Color4ub *c = LuaColor::CheckFromLua(L, 1);
	float factor = luaL_checknumber(L, 2);
	Color4ub r(*c);
	r.Tint(factor);
	LuaColor::PushToLua(L, r);
	return 1;
}

// Set a Color's values without allocating new memory.
// c = Color(1, 2, 3, 4); c(120, 140, 255, 255)
static int l_color_set(lua_State *L)
{
	LUA_DEBUG_START(L);
	Color4ub *col = LuaColor::CheckFromLua(L, 1);
	double r = luaL_checknumber(L, 2);
	double g = luaL_checknumber(L, 3);
	double b = luaL_checknumber(L, 4);
	double a = luaL_optnumber(L, 5, 255.0);
	*col = ColorClamp(r, g, b, a);
	lua_pushvalue(L, 1); // return the same color value.
	LUA_DEBUG_END(L, 1);
	return 1;
}

static luaL_Reg l_vector_lib[] = {
	{ "new", &l_color_new },
	{ "shade", &l_color_shade },
	{ "tint", &l_color_tint },
	{ 0, 0 }
};

static luaL_Reg l_vector_meta[] = {
	{ "__tostring", &l_color_tostring },
	{ "__add", &l_color_add },
	{ "__mul", &l_color_mul },
	{ "__index", &l_color_index },
	{ "__newindex", &l_color_new_index },
	{ "__call", &l_color_set },
	{ "shade", &l_color_shade },
	{ "tint", &l_color_tint },
	{ 0, 0 }
};

const char LuaColor::LibName[] = "Color";
const char LuaColor::TypeName[] = "Color";

void LuaColor::Register(lua_State *L)
{
	LUA_DEBUG_START(L);

	luaL_newlib(L, l_vector_lib);

	lua_newtable(L);
	lua_pushcfunction(L, &l_color_call);
	lua_setfield(L, -2, "__call");
	lua_setmetatable(L, -2);

	lua_setglobal(L, LuaColor::LibName);

	luaL_newmetatable(L, LuaColor::TypeName);
	luaL_setfuncs(L, l_vector_meta, 0);
	// hide the metatable to thwart crazy exploits
	lua_pushboolean(L, 0);
	lua_setfield(L, -2, "__metatable");
	lua_pop(L, 1);

	LUA_DEBUG_END(L, 0);
}

Color4ub *LuaColor::PushNewToLua(lua_State *L)
{
	Color4ub *ptr = static_cast<Color4ub *>(lua_newuserdata(L, sizeof(Color4ub)));
	luaL_setmetatable(L, LuaColor::TypeName);
	return ptr;
}

const Color4ub *LuaColor::GetFromLua(lua_State *L, int idx)
{
	return static_cast<Color4ub *>(luaL_testudata(L, idx, LuaColor::TypeName));
}

Color4ub *LuaColor::CheckFromLua(lua_State *L, int idx)
{
	return static_cast<Color4ub *>(luaL_checkudata(L, idx, LuaColor::TypeName));
}
