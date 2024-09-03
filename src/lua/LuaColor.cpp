// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaColor.h"
#include "Color.h"
#include "JsonUtils.h"
#include "LuaMetaType.h"
#include "LuaObject.h"
#include "LuaUtils.h"

void pi_lua_generic_pull(lua_State *l, int index, Color4ub *&out)
{
	out = LuaColor::CheckFromLua(l, index);
}

inline Color4ub ColorClamp(float r, float g, float b, float a)
{
	r = std::max(std::min(r, 255.0f), 0.0f);
	g = std::max(std::min(g, 255.0f), 0.0f);
	b = std::max(std::min(b, 255.0f), 0.0f);
	a = std::max(std::min(a, 255.0f), 0.0f);
	return Color4ub(r, g, b, a);
}

static uint8_t char_to_nibble(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	else if (c >= 'A' && c <= 'F')
		return (c - 'A') + 10;
	else if (c >= 'a' && c <= 'f')
		return (c - 'a') + 10;

	return 0xF;
}

static Color4ub parse_color_hex(const std::string &str)
{
	if (str.size() == 3) {
		uint8_t r = char_to_nibble(str[0]), g = char_to_nibble(str[1]), b = char_to_nibble(str[2]);
		return Color4ub(r | (r << 4), g | (g << 4), b | (b << 4), 255);
	} else if (str.size() == 6) {
		uint8_t r = (char_to_nibble(str[0]) << 4) | char_to_nibble(str[1]);
		uint8_t g = (char_to_nibble(str[2]) << 4) | char_to_nibble(str[3]);
		uint8_t b = (char_to_nibble(str[4]) << 4) | char_to_nibble(str[5]);
		return Color4ub(r, g, b, 255);
	} else if (str.size() == 8) {
		uint8_t r = (char_to_nibble(str[0]) << 4) | char_to_nibble(str[1]);
		uint8_t g = (char_to_nibble(str[2]) << 4) | char_to_nibble(str[3]);
		uint8_t b = (char_to_nibble(str[4]) << 4) | char_to_nibble(str[5]);
		uint8_t a = (char_to_nibble(str[6]) << 4) | char_to_nibble(str[7]);
		return Color4ub(r, g, b, a);
	}

	return {};
}

static int l_color_new(lua_State *L)
{
	LUA_DEBUG_START(L);
	if (lua_type(L, 1) == LUA_TSTRING) {
		std::string str = lua_tostring(L, 1);
		if (str.find_first_not_of("0123456789ABCDEFabcdef") != std::string::npos) {
			return luaL_error(L, "Color string '%s' cannot contain non-hexadecimal characters!", str.c_str());
		}

		LuaColor::PushToLua(L, parse_color_hex(str));
	} else {
		double r = luaL_checknumber(L, 1);
		double g = luaL_checknumber(L, 2);
		double b = luaL_checknumber(L, 3);
		double a = luaL_optnumber(L, 4, 255.0);
		LuaColor::PushToLua(L, ColorClamp(r, g, b, a));
	}

	LUA_DEBUG_END(L, 1);
	return 1;
}

static int l_color_call(lua_State *L)
{
	LUA_DEBUG_START(L);
	if (lua_type(L, 2) == LUA_TSTRING) {
		std::string str = lua_tostring(L, 2);
		if (str.find_first_not_of("0123456789ABCDEFabcdef") != std::string::npos) {
			return luaL_error(L, "Color string '%s' cannot contain non-hexadecimal characters!", str.c_str());
		}

		LuaColor::PushToLua(L, parse_color_hex(str));
	} else {
		double r = luaL_checknumber(L, 2);
		double g = luaL_checknumber(L, 3);
		double b = luaL_checknumber(L, 4);
		double a = luaL_optnumber(L, 5, 255.0);
		LuaColor::PushToLua(L, ColorClamp(r, g, b, a));
	}

	LUA_DEBUG_END(L, 1);
	return 1;
}

static int l_color_tostring(lua_State *L)
{
	const Color4ub *c = LuaColor::CheckFromLua(L, 1);
	lua_pushfstring(L, "Color(%d, %d, %d, %d)", (int)c->r, (int)c->g, (int)c->b, (int)c->a);
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
		lua_getfield(L, -1, "methods");
		lua_pushvalue(L, 2);
		lua_rawget(L, -2);
		lua_remove(L, -2);
	} else {
		luaL_error(L, "Attempted to index Color with a non-string type '%s'", luaL_typename(L, 2));
	}
	return 1;
}

// Set a Color's values without allocating new memory.
// c = Color(1, 2, 3, 4); c(120, 140, 255, 255)
static int l_color_set(lua_State *L)
{
	LUA_DEBUG_START(L);
	Color4ub *col = LuaColor::CheckFromLua(L, 1);

	if (lua_type(L, 2) == LUA_TSTRING) {
		std::string str = lua_tostring(L, 2);
		if (str.find_first_not_of("0123456789ABCDEFabcdef") != std::string::npos) {
			return luaL_error(L, "Color string '%s' cannot contain non-hexadecimal characters!", str.c_str());
		}

		*col = parse_color_hex(str);
	} else {
		double r = luaL_checknumber(L, 2);
		double g = luaL_checknumber(L, 3);
		double b = luaL_checknumber(L, 4);
		double a = luaL_optnumber(L, 5, 255.0);
		*col = ColorClamp(r, g, b, a);
	}

	lua_pushvalue(L, 1); // return the same color value.
	LUA_DEBUG_END(L, 1);
	return 1;
}


static luaL_Reg l_color_meta[] = {
	{ "__tostring", &l_color_tostring },
	{ "__add", &l_color_add },
	{ "__mul", &l_color_mul },
	{ "__index", &l_color_index },
	{ "__newindex", &l_color_new_index },
	{ "__call", &l_color_set },
	{ 0, 0 }
};

static bool _serialize_color(lua_State *l, Json &out)
{
	Color4ub *c = LuaColor::CheckFromLua(l, -1);
	ColorToJson(out, *c);
	return true;
}

static bool _deserialize_color(lua_State *l, const Json &obj)
{
	Color4ub *c = LuaColor::PushNewToLua(l);
	JsonToColor(c, obj);
	return true;
}

const char LuaColor::LibName[] = "Color";
const char LuaColor::TypeName[] = "Color";

void LuaColor::Register(lua_State *L)
{
	LUA_DEBUG_START(L);

	LuaMetaType<Color4ub> metaType(LuaColor::TypeName);
	metaType.CreateMetaType(L);
	metaType.StartRecording()
		.AddCallCtor(&l_color_call)
		.AddNewCtor(&l_color_new)
		.AddFunction("shade", &Color4ub::Shade)
		.AddFunction("tint", &Color4ub::Tint)
		.AddFunction("opacity", &Color4ub::Opacity)
		.StopRecording();

	metaType.GetMetatable();
	luaL_setfuncs(L, l_color_meta, 0);
	// hide the metatable to thwart crazy exploits
	lua_pushboolean(L, 0);
	lua_setfield(L, -2, "__metatable");

	lua_getfield(L, -1, "methods");
	lua_setglobal(L, LuaColor::LibName);
	lua_pop(L, 1);

	LuaObjectBase::RegisterSerializer(LuaColor::TypeName, { _serialize_color, _deserialize_color });

	LUA_DEBUG_END(L, 0);
}

Color4ub *LuaColor::PushNewToLua(lua_State *L)
{
	Color4ub *ptr = static_cast<Color4ub *>(lua_newuserdata(L, sizeof(Color4ub)));
	LuaMetaTypeBase::GetMetatableFromName(L, LuaColor::TypeName);
	lua_setmetatable(L, -2);
	return ptr;
}

const Color4ub *LuaColor::GetFromLua(lua_State *L, int idx)
{
	return static_cast<Color4ub *>(LuaMetaTypeBase::TestUserdata(L, idx, LuaColor::TypeName));
}

Color4ub *LuaColor::CheckFromLua(lua_State *L, int idx)
{
	return static_cast<Color4ub *>(LuaMetaTypeBase::CheckUserdata(L, idx, LuaColor::TypeName));
}
