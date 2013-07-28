// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Color.h"
#include "LuaUtils.h"

const Color4f Color::BLACK  = Color(0.0f,0.0f,0.0f,1.0f);
const Color4f Color::WHITE  = Color(1.0f,1.0f,1.0f,1.0f);
const Color4f Color::RED    = Color(1.0f,0.0f,0.0f,1.0f);
const Color4f Color::GREEN  = Color(0.0f,1.0f,0.0f,1.0f);
const Color4f Color::BLUE   = Color(0.0f,0.0f,1.0f,1.0f);
const Color4f Color::YELLOW = Color(1.0f,1.0f,0.0f,1.0f);

const Color4ub Color4ub::BLACK   = Color4ub(0, 0, 0, 255);
const Color4ub Color4ub::WHITE   = Color4ub(255, 255, 255, 255);
const Color4ub Color4ub::RED     = Color4ub(255, 0, 0, 255);
const Color4ub Color4ub::GREEN   = Color4ub(0, 255, 0, 255);
const Color4ub Color4ub::BLUE    = Color4ub(0, 0, 255, 255);
const Color4ub Color4ub::YELLOW  = Color4ub(255, 255, 0, 255);

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
