// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LUAPUSHPULL_H
#define _LUAPUSHPULL_H

#include "Lua.h"
#include <lua.hpp>

#include <cstddef>
#include <string>
#include <tuple>
#include <cstdint>
#include <type_traits>

// Prevent automatic conversion of pointer values to boolean (never what we want when pushing values to lua)
// sadly requires type_traits so compile times suffer
template<typename T, typename = std::enable_if_t<std::is_same_v<T, bool>>>
inline void pi_lua_generic_push(lua_State *l, T value) { lua_pushboolean(l, value); }

inline void pi_lua_generic_push(lua_State *l, int32_t value) { lua_pushinteger(l, value); }
inline void pi_lua_generic_push(lua_State *l, int64_t value) { lua_pushinteger(l, value); }
inline void pi_lua_generic_push(lua_State *l, uint32_t value) { lua_pushinteger(l, value); }
inline void pi_lua_generic_push(lua_State *l, uint64_t value) { lua_pushinteger(l, value); }
inline void pi_lua_generic_push(lua_State *l, double value) { lua_pushnumber(l, value); }
inline void pi_lua_generic_push(lua_State *l, const char *value) { lua_pushstring(l, value); }
inline void pi_lua_generic_push(lua_State *l, const std::string &value)
{
	lua_pushlstring(l, value.c_str(), value.size());
}
inline void pi_lua_generic_push(lua_State *l, const std::string_view &value)
{
	lua_pushlstring(l, value.data(), value.size());
}
inline void pi_lua_generic_push(lua_State *l, const std::nullptr_t &value) { lua_pushnil(l); }

inline void pi_lua_generic_pull(lua_State *l, int index, bool &out) { out = lua_toboolean(l, index); }
inline void pi_lua_generic_pull(lua_State *l, int index, int32_t &out) { out = luaL_checkinteger(l, index); }
inline void pi_lua_generic_pull(lua_State *l, int index, int64_t &out) { out = luaL_checkinteger(l, index); }
inline void pi_lua_generic_pull(lua_State *l, int index, uint32_t &out) { out = luaL_checkinteger(l, index); }
inline void pi_lua_generic_pull(lua_State *l, int index, uint64_t &out) { out = luaL_checkinteger(l, index); }
inline void pi_lua_generic_pull(lua_State *l, int index, float &out) { out = luaL_checknumber(l, index); }
inline void pi_lua_generic_pull(lua_State *l, int index, double &out) { out = luaL_checknumber(l, index); }
inline void pi_lua_generic_pull(lua_State *l, int index, const char *&out) { out = luaL_checkstring(l, index); }
inline void pi_lua_generic_pull(lua_State *l, int index, std::string &out)
{
	size_t len;
	const char *buf = luaL_checklstring(l, index, &len);
	std::string(buf, len).swap(out);
}
// the returned string view is only valid until the lua object is removed from the stack
inline void pi_lua_generic_pull(lua_State *l, int index, std::string_view &out)
{
	size_t len;
	const char *buf = luaL_checklstring(l, index, &len);
	out = { buf, len };
}

template <typename Type>
inline void LuaPush(lua_State *l, Type value)
{
	pi_lua_generic_push(l, value);
}

template <typename Type>
inline std::remove_reference_t<Type> LuaPull(lua_State *l, int index)
{
	std::decay_t<Type> value;
	pi_lua_generic_pull(l, index, value);
	return value;
}

// Pull a value with an optional default.
template <typename Type>
inline std::remove_reference_t<Type> LuaPull(lua_State *l, int index, Type defaultVal)
{
	Type value = defaultVal;
	if (lua_gettop(l) >= index && !lua_isnil(l, index))
		pi_lua_generic_pull(l, index, value);
	return value;
}

inline bool pi_lua_strict_pull(lua_State *l, int index, bool &out)
{
	if (lua_type(l, index) == LUA_TBOOLEAN) {
		out = lua_toboolean(l, index);
		return true;
	}
	return false;
}
inline bool pi_lua_strict_pull(lua_State *l, int index, int &out)
{
	if (lua_type(l, index) == LUA_TNUMBER) {
		out = lua_tointeger(l, index);
		return true;
	}
	return false;
}
inline bool pi_lua_strict_pull(lua_State *l, int index, float &out)
{
	if (lua_type(l, index) == LUA_TNUMBER) {
		out = lua_tonumber(l, index);
		return true;
	}
	return false;
}
inline bool pi_lua_strict_pull(lua_State *l, int index, double &out)
{
	if (lua_type(l, index) == LUA_TNUMBER) {
		out = lua_tonumber(l, index);
		return true;
	}
	return false;
}
inline bool pi_lua_strict_pull(lua_State *l, int index, const char *&out)
{
	if (lua_type(l, index) == LUA_TSTRING) {
		out = lua_tostring(l, index);
		return true;
	}
	return false;
}
inline bool pi_lua_strict_pull(lua_State *l, int index, std::string &out)
{
	if (lua_type(l, index) == LUA_TSTRING) {
		size_t len;
		const char *buf = lua_tolstring(l, index, &len);
		std::string(buf, len).swap(out);
		return true;
	}
	return false;
}

// Variadic push/pull support

template<typename... Types>
inline void pi_lua_multiple_push(lua_State *l, Types... args)
{
	(pi_lua_generic_push(l, args), ...);
}

inline void pi_lua_multiple_push(lua_State *l)
{
	return;
}

template<typename... Types>
inline std::tuple<std::remove_reference_t<Types>...> pi_lua_multiple_pull(lua_State *l, int beg)
{
	// List initialization (braces) has a defined sequence of operations
	// thus the increment of `beg` over the parameter pack is well-defined
	return std::tuple<std::remove_reference_t<Types>...>{ LuaPull<Types>(l, beg++)... };
}

#endif
