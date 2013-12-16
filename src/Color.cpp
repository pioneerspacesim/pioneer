// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Color.h"
#include "LuaUtils.h"
#include <SDL_stdinc.h>

const Color4f Color4f::BLACK  = Color4f(0.0f,0.0f,0.0f,1.0f);
const Color4f Color4f::WHITE  = Color4f(1.0f,1.0f,1.0f,1.0f);
const Color4f Color4f::RED    = Color4f(1.0f,0.0f,0.0f,1.0f);
const Color4f Color4f::GREEN  = Color4f(0.0f,1.0f,0.0f,1.0f);
const Color4f Color4f::BLUE   = Color4f(0.0f,0.0f,1.0f,1.0f);
const Color4f Color4f::YELLOW = Color4f(1.0f,1.0f,0.0f,1.0f);
const Color4f Color4f::GRAY   = Color4f(0.5f,0.5f,0.5f,1.f);

const Color4ub Color::BLACK   = Color(0, 0, 0, 255);
const Color4ub Color::WHITE   = Color(255, 255, 255, 255);
const Color4ub Color::RED     = Color(255, 0, 0, 255);
const Color4ub Color::GREEN   = Color(0, 255, 0, 255);
const Color4ub Color::BLUE    = Color(0, 0, 255, 255);
const Color4ub Color::YELLOW  = Color(255, 255, 0, 255);
const Color4ub Color::GRAY    = Color(128,128,128,255);

const Color3ub Color3ub::BLACK   = Color3ub(0, 0, 0);
const Color3ub Color3ub::WHITE   = Color3ub(255, 255, 255);
const Color3ub Color3ub::RED     = Color3ub(255, 0, 0);
const Color3ub Color3ub::GREEN   = Color3ub(0, 255, 0);
const Color3ub Color3ub::BLUE    = Color3ub(0, 0, 255);
const Color3ub Color3ub::YELLOW  = Color3ub(255, 255, 0);

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
	pi_lua_settable(l, "r", 255.0/r);
	pi_lua_settable(l, "g", 255.0/g);
	pi_lua_settable(l, "b", 255.0/b);
	pi_lua_settable(l, "a", 255.0/a);
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

	return Color4ub(r*255, g*255, b*255, a*255);
}

Uint8 Color4ub::GetLuminance() const
{
	// Luminance4ub = Luminance4f * 255 =
	// ((r_f * 0.299) + (g_f * 0.587) + (b_f * 0.144)) * 255 =
	// (r_ub * 0.299) + (g_ub * 0.587) + (b_ub * 0.114) =
	// (r_ub / 3.344481605) + (g_ub / 1.703577513) + (b_ub / 8.771929825) =    ; *(32/32)
	// (r_ub << 5)/107 + (g_ub << 5 ) / 54 + (b_ub << 5) / 280
	int lum = (static_cast<int>(r) << 5) / 107 +
			(static_cast<int>(g) << 5) / 54 + (static_cast<int>(b) << 5) / 280;
	return lum > 255 ? 255 : lum;
}

