// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LUAPUSHPULL_H
#define _LUAPUSHPULL_H

#include "Lua.h"
#include <lua.hpp>

#include <cstddef>
#include <string>
#include <tuple>

inline void pi_lua_generic_push(lua_State *l, bool value) { lua_pushboolean(l, value); }
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
inline void pi_lua_generic_push(lua_State *l, std::string_view &value)
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
inline Type LuaPull(lua_State *l, int index, Type defaultVal)
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
template <typename... Types>
inline void pi_lua_multiple_push(lua_State *l, Types... args);

#if defined(_MSC_VER) // Non-variadic version for MSVC
template <typename Arg1>
inline void pi_lua_multiple_push(lua_State *l, Arg1 arg1)
{
	pi_lua_generic_push(l, arg1);
}

template <typename Arg1, typename Arg2>
inline void pi_lua_multiple_push(lua_State *l, Arg1 arg1, Arg2 arg2)
{
	pi_lua_generic_push(l, arg1);
	pi_lua_generic_push(l, arg2);
}

template <typename Arg1, typename Arg2, typename Arg3>
inline void pi_lua_multiple_push(lua_State *l, Arg1 arg1, Arg2 arg2, Arg3 arg3)
{
	pi_lua_generic_push(l, arg1);
	pi_lua_generic_push(l, arg2);
	pi_lua_generic_push(l, arg3);
}

template <typename Arg1, typename Arg2, typename Arg3, typename Arg4>
inline void pi_lua_multiple_push(lua_State *l, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4)
{
	pi_lua_generic_push(l, arg1);
	pi_lua_generic_push(l, arg2);
	pi_lua_generic_push(l, arg3);
	pi_lua_generic_push(l, arg4);
}
#else // Just use the normal variadic version
template <typename Head, typename... Tail>
inline void pi_lua_multiple_push(lua_State *l, Head arg1, Tail... rest)
{
	pi_lua_generic_push(l, arg1);
	pi_lua_multiple_push(l, rest...);
}
#endif

inline void pi_lua_multiple_push(lua_State *l)
{
	return;
}

#if defined(_MSC_VER)
template <typename Arg1, typename Arg2>
inline std::tuple<Arg1, Arg2> pi_lua_multiple_pull(lua_State *l, int beg)
{
	beg = lua_absindex(l, beg);
	Arg1 arg1;
	Arg2 arg2;
	pi_lua_generic_pull(l, beg, arg1);
	pi_lua_generic_pull(l, beg + 1, arg2);
	return std::make_tuple(arg1, arg2);
}
template <typename Arg1, typename Arg2, typename Arg3>
inline std::tuple<Arg1, Arg2, Arg3> pi_lua_multiple_pull(lua_State *l, int beg)
{
	beg = lua_absindex(l, beg);
	Arg1 arg1;
	Arg2 arg2;
	Arg3 arg3;
	pi_lua_generic_pull(l, beg, arg1);
	pi_lua_generic_pull(l, beg + 1, arg2);
	pi_lua_generic_pull(l, beg + 2, arg3);
	return std::make_tuple(arg1, arg2, arg3);
}
template <typename Arg1, typename Arg2, typename Arg3, typename Arg4>
inline std::tuple<Arg1, Arg2, Arg3, Arg4> pi_lua_multiple_pull(lua_State *l, int beg)
{
	beg = lua_absindex(l, beg);
	Arg1 arg1;
	Arg2 arg2;
	Arg3 arg3;
	Arg4 arg4;
	pi_lua_generic_pull(l, beg, arg1);
	pi_lua_generic_pull(l, beg + 1, arg2);
	pi_lua_generic_pull(l, beg + 2, arg3);
	pi_lua_generic_pull(l, beg + 3, arg4);
	return std::make_tuple(arg1, arg2, arg3, arg4);
}
#else
// The _bogus parameter is used to bring the empty type list case into the template world
// to solve name resolution problems.
template <int _bogus, typename Head, typename... Tail>
inline std::tuple<Head, Tail...> __helper_pi_lua_multiple_pull(lua_State *l, int beg)
{
	beg = lua_absindex(l, beg);
	std::tuple<Tail...> rest = __helper_pi_lua_multiple_pull<_bogus, Tail...>(l, beg + 1);
	Head hd;
	pi_lua_generic_pull(l, beg, hd);
	std::tuple<Head> first(hd);
	return std::tuple_cat(first, rest);
}

template <int _bogus>
inline std::tuple<> __helper_pi_lua_multiple_pull(lua_State *l, int beg)
{
	return std::tuple<>();
}

template <typename... Types>
inline std::tuple<Types...> pi_lua_multiple_pull(lua_State *l, int beg)
{
	return __helper_pi_lua_multiple_pull<0, Types...>(l, beg);
}
#endif

#endif
