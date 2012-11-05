// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Color.h"
#include "LuaUtils.h"

const Color4f Color::BLACK = Color(0.0f,0.0f,0.0f,1.0f);
const Color4f Color::WHITE = Color(1.0f,1.0f,1.0f,1.0f);
const Color4f Color::RED   = Color(1.0f,0.0f,0.0f,1.0f);
const Color4f Color::BLUE  = Color(0.0f,0.0f,1.0f,1.0f);

const Color4ub Color4ub::BLACK = Color4ub(0, 0, 0, 255);
const Color4ub Color4ub::WHITE = Color4ub(255, 255, 255, 255);

float Color4f::GetLuminance() const
{
	return (0.299f * r) + (0.587f * g) + (0.114f * b);
}

static inline void _get_number(lua_State *l, int table, const char *key, float &output)
{
	lua_pushstring(l, key);
	lua_gettable(l, table);
	output = lua_tonumber(l, -1);
	lua_pop(l, 1);
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
	_get_number(l, table, "a", a);

	LUA_DEBUG_END(l, 0);

	return Color4f(r, g, b, a);
}
