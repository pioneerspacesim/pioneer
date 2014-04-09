// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LUAPUSHPULL_H
#define _LUAPUSHPULL_H

#include "lua/lua.hpp"
#include "Lua.h"
#include <string>

inline void pi_lua_generic_push(lua_State * l, bool value) { lua_pushboolean(l, value); }
inline void pi_lua_generic_push(lua_State * l, int value) { lua_pushinteger(l, value); }
inline void pi_lua_generic_push(lua_State * l, unsigned int value) { lua_pushinteger(l, value); }
inline void pi_lua_generic_push(lua_State * l, double value) { lua_pushnumber(l, value); }
inline void pi_lua_generic_push(lua_State * l, const char * value) { lua_pushstring(l, value); }
inline void pi_lua_generic_push(lua_State * l, const std::string & value) {
	lua_pushlstring(l, value.c_str(), value.size());
}

inline void pi_lua_generic_pull(lua_State * l, int index, bool & out) { out = lua_toboolean(l, index); }
inline void pi_lua_generic_pull(lua_State * l, int index, int & out) { out = luaL_checkinteger(l, index); }
inline void pi_lua_generic_pull(lua_State * l, int index, unsigned int & out) { out = luaL_checkunsigned(l, index); }
inline void pi_lua_generic_pull(lua_State * l, int index, float & out) { out = luaL_checknumber(l, index); }
inline void pi_lua_generic_pull(lua_State * l, int index, double & out) { out = luaL_checknumber(l, index); }
inline void pi_lua_generic_pull(lua_State * l, int index, const char * & out) { out = luaL_checkstring(l, index); }
inline void pi_lua_generic_pull(lua_State * l, int index, std::string & out) {
	size_t len;
	const char *buf = luaL_checklstring(l, index, &len);
	std::string(buf, len).swap(out);
}
inline bool pi_lua_strict_pull(lua_State * l, int index, bool & out) {
	if (lua_type(l, index) == LUA_TBOOLEAN) {
		out = lua_toboolean(l, index);
		return true;
	}
	return false;
}
inline bool pi_lua_strict_pull(lua_State * l, int index, int & out) {
	if (lua_type(l, index) == LUA_TNUMBER) {
		out = lua_tointeger(l, index);
		return true;
	}
	return false;
}
inline bool pi_lua_strict_pull(lua_State * l, int index, float & out) {
	if (lua_type(l, index) == LUA_TNUMBER) {
		out = lua_tonumber(l, index);
		return true;
	}
	return false;
}
inline bool pi_lua_strict_pull(lua_State * l, int index, double & out) {
	if (lua_type(l, index) == LUA_TNUMBER) {
		out = lua_tonumber(l, index);
		return true;
	}
	return false;
}
inline bool pi_lua_strict_pull(lua_State * l, int index, const char * & out) {
	if (lua_type(l, index) == LUA_TSTRING) {
		out = lua_tostring(l, index);
		return true;
	}
	return false;
}
inline bool pi_lua_strict_pull(lua_State * l, int index, std::string & out) {
	if (lua_type(l, index) == LUA_TSTRING) {
		size_t len;
		const char *buf = lua_tolstring(l, index, &len);
		std::string(buf, len).swap(out);
		return true;
	}
	return false;
}

template <typename ...Types>
inline void pi_lua_multiple_push(lua_State *l, Types ...args);

template <typename Head, typename ...Tail>
inline void pi_lua_multiple_push(lua_State *l, Head arg1, Tail ...rest) {
	pi_lua_generic_push(l, arg1);
	pi_lua_multiple_push(l, rest...);
}

template <>
inline void pi_lua_multiple_push(lua_State *l) {
	return;
}

#endif
