// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Color.h"
#include "lua/LuaUtils.h"

#include <cassert>

const Color4f Color4f::BLACK = Color4f(0.0f, 0.0f, 0.0f, 1.0f);
const Color4f Color4f::WHITE = Color4f(1.0f, 1.0f, 1.0f, 1.0f);
const Color4f Color4f::RED = Color4f(1.0f, 0.0f, 0.0f, 1.0f);
const Color4f Color4f::GREEN = Color4f(0.0f, 1.0f, 0.0f, 1.0f);
const Color4f Color4f::BLUE = Color4f(0.0f, 0.0f, 1.0f, 1.0f);
const Color4f Color4f::YELLOW = Color4f(1.0f, 1.0f, 0.0f, 1.0f);
const Color4f Color4f::GRAY = Color4f(0.5f, 0.5f, 0.5f, 1.f);
const Color4f Color4f::STEELBLUE = Color4f(0.27f, 0.51f, 0.71f, 1.f);
const Color4f Color4f::BLANK = Color4f(0.0f, 0.0f, 0.0f, 0.0f);

const Color4ub Color::BLACK = Color(0, 0, 0, 255);
const Color4ub Color::WHITE = Color(255, 255, 255, 255);
const Color4ub Color::RED = Color(255, 0, 0, 255);
const Color4ub Color::GREEN = Color(0, 255, 0, 255);
const Color4ub Color::BLUE = Color(0, 0, 255, 255);
const Color4ub Color::YELLOW = Color(255, 255, 0, 255);
const Color4ub Color::GRAY = Color(128, 128, 128, 255);
const Color4ub Color::STEELBLUE = Color(68, 130, 181, 255);
const Color4ub Color::BLANK = Color(0, 0, 0, 0);
const Color4ub Color::PINK = Color(252, 15, 192, 255); // debug pink

const Color3ub Color3ub::BLACK = Color3ub(0, 0, 0);
const Color3ub Color3ub::WHITE = Color3ub(255, 255, 255);
const Color3ub Color3ub::RED = Color3ub(255, 0, 0);
const Color3ub Color3ub::GREEN = Color3ub(0, 255, 0);
const Color3ub Color3ub::BLUE = Color3ub(0, 0, 255);
const Color3ub Color3ub::YELLOW = Color3ub(255, 255, 0);
const Color3ub Color3ub::STEELBLUE = Color3ub(68, 130, 181);
const Color3ub Color3ub::BLANK = Color3ub(0, 0, 0);

float Color4f::GetLuminance() const
{
	return (0.299f * r) + (0.587f * g) + (0.114f * b);
}

void Color4f::ToLuaTable(lua_State *l)
{
	lua_newtable(l);
	pi_lua_settable(l, "r", r);
	pi_lua_settable(l, "g", g);
	pi_lua_settable(l, "b", b);
	pi_lua_settable(l, "a", a);
}

static inline bool _get_number(lua_State *l, int table, const char *key, float &output)
{
	lua_pushstring(l, key);
	lua_gettable(l, table);
	bool found = !lua_isnil(l, -1);
	output = lua_tonumber(l, -1);
	lua_pop(l, 1);
	return found;
}

Color4f Color4f::FromLuaTable(lua_State *l, int idx)
{
	const int table = lua_absindex(l, idx);
	assert(lua_istable(l, table));

	LUA_DEBUG_START(l);

	float r, g, b, a;
	_get_number(l, table, "r", r);
	_get_number(l, table, "g", g);
	_get_number(l, table, "b", b);
	if (!_get_number(l, table, "a", a)) a = 1.0f;

	LUA_DEBUG_END(l, 0);

	return Color4f(r, g, b, a);
}

void Color4ub::ToLuaTable(lua_State *l)
{
	lua_newtable(l);
	pi_lua_settable(l, "r", 255.0 / r);
	pi_lua_settable(l, "g", 255.0 / g);
	pi_lua_settable(l, "b", 255.0 / b);
	pi_lua_settable(l, "a", 255.0 / a);
}

Color4ub Color4ub::FromLuaTable(lua_State *l, int idx)
{
	const int table = lua_absindex(l, idx);
	assert(lua_istable(l, table));

	LUA_DEBUG_START(l);

	float r, g, b, a;
	_get_number(l, table, "r", r);
	_get_number(l, table, "g", g);
	_get_number(l, table, "b", b);
	if (!_get_number(l, table, "a", a)) a = 1.0f;

	LUA_DEBUG_END(l, 0);

	return Color4ub(r * 255, g * 255, b * 255, a * 255);
}

Uint8 Color4ub::GetLuminance() const
{
	// these weights are those used for the JPEG luma channel
	return (r * 299 + g * 587 + b * 114) / 1000;
}
