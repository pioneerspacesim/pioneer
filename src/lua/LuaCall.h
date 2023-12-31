// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

/*
	This file implements a generic method to call a C++ member function with
	arguments passed in from Lua. It's intended to back a generic method of
	binding code to Lua with minimal boilerplate needed.
*/

#include "Lua.h"
#include "LuaPushPull.h"

// Backend to bind call a C++ free function with arguments from Lua.
// Parameter `index` points to the first argument

template <typename Ret>
Ret pi_lua_multiple_call(lua_State *l, int index, Ret (*fn)())
{
	return fn();
}

template <typename Ret, typename Arg1>
Ret pi_lua_multiple_call(lua_State *l, int index, Ret (*fn)(Arg1))
{
	auto arg1 = LuaPull<Arg1>(l, index);
	return fn(arg1);
}

template <typename Ret, typename Arg1, typename Arg2>
Ret pi_lua_multiple_call(lua_State *l, int index, Ret (*fn)(Arg1, Arg2))
{
	auto arg1 = LuaPull<Arg1>(l, index);
	auto arg2 = LuaPull<Arg2>(l, index + 1);
	return fn(arg1, arg2);
}

template <typename Ret, typename Arg1, typename Arg2, typename Arg3>
Ret pi_lua_multiple_call(lua_State *l, int index, Ret (*fn)(Arg1, Arg2, Arg3))
{
	auto arg1 = LuaPull<Arg1>(l, index);
	auto arg2 = LuaPull<Arg2>(l, index + 1);
	auto arg3 = LuaPull<Arg3>(l, index + 2);
	return fn(arg1, arg2, arg3);
}

template <typename Ret, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
Ret pi_lua_multiple_call(lua_State *l, int index, Ret (*fn)(Arg1, Arg2, Arg3, Arg4))
{
	auto arg1 = LuaPull<Arg1>(l, index);
	auto arg2 = LuaPull<Arg2>(l, index + 1);
	auto arg3 = LuaPull<Arg3>(l, index + 2);
	auto arg4 = LuaPull<Arg4>(l, index + 3);
	return fn(arg1, arg2, arg3, arg4);
}

template <typename Ret, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
Ret pi_lua_multiple_call(lua_State *l, int index, Ret (*fn)(Arg1, Arg2, Arg3, Arg4, Arg5))
{
	auto arg1 = LuaPull<Arg1>(l, index);
	auto arg2 = LuaPull<Arg2>(l, index + 1);
	auto arg3 = LuaPull<Arg3>(l, index + 2);
	auto arg4 = LuaPull<Arg4>(l, index + 3);
	auto arg5 = LuaPull<Arg5>(l, index + 4);
	return fn(arg1, arg2, arg3, arg4, arg5);
}

template <typename Ret, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
Ret pi_lua_multiple_call(lua_State *l, int index, Ret (*fn)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6))
{
	auto arg1 = LuaPull<Arg1>(l, index);
	auto arg2 = LuaPull<Arg2>(l, index + 1);
	auto arg3 = LuaPull<Arg3>(l, index + 2);
	auto arg4 = LuaPull<Arg4>(l, index + 3);
	auto arg5 = LuaPull<Arg5>(l, index + 4);
	auto arg6 = LuaPull<Arg6>(l, index + 5);
	return fn(arg1, arg2, arg3, arg4, arg5, arg6);
}

template <typename Ret, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7>
Ret pi_lua_multiple_call(lua_State *l, int index, Ret (*fn)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7))
{
	auto arg1 = LuaPull<Arg1>(l, index);
	auto arg2 = LuaPull<Arg2>(l, index + 1);
	auto arg3 = LuaPull<Arg3>(l, index + 2);
	auto arg4 = LuaPull<Arg4>(l, index + 3);
	auto arg5 = LuaPull<Arg5>(l, index + 4);
	auto arg6 = LuaPull<Arg6>(l, index + 5);
	auto arg7 = LuaPull<Arg7>(l, index + 6);
	return fn(arg1, arg2, arg3, arg4, arg5, arg6, arg7);
}

// Backend to bind call a C++ member function with arguments from Lua.
// Parameter `index` points one-behind the first argument, where the object
// pointer should traditionally be

template <class T, typename Ret>
Ret pi_lua_multiple_call(lua_State *l, int index, T *ptr, Ret (T::*fn)())
{
	return (ptr->*fn)();
}

template <class T, typename Ret, typename Arg1>
Ret pi_lua_multiple_call(lua_State *l, int index, T *ptr, Ret (T::*fn)(Arg1))
{
	auto arg1 = LuaPull<Arg1>(l, index + 1);
	return (ptr->*fn)(arg1);
}

template <class T, typename Ret, typename Arg1, typename Arg2>
Ret pi_lua_multiple_call(lua_State *l, int index, T *ptr, Ret (T::*fn)(Arg1, Arg2))
{
	auto arg1 = LuaPull<Arg1>(l, index + 1);
	auto arg2 = LuaPull<Arg2>(l, index + 2);
	return (ptr->*fn)(arg1, arg2);
}

template <class T, typename Ret, typename Arg1, typename Arg2, typename Arg3>
Ret pi_lua_multiple_call(lua_State *l, int index, T *ptr, Ret (T::*fn)(Arg1, Arg2, Arg3))
{
	auto arg1 = LuaPull<Arg1>(l, index + 1);
	auto arg2 = LuaPull<Arg2>(l, index + 2);
	auto arg3 = LuaPull<Arg3>(l, index + 3);
	return (ptr->*fn)(arg1, arg2, arg3);
}

template <class T, typename Ret, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
Ret pi_lua_multiple_call(lua_State *l, int index, T *ptr, Ret (T::*fn)(Arg1, Arg2, Arg3, Arg4))
{
	auto arg1 = LuaPull<Arg1>(l, index + 1);
	auto arg2 = LuaPull<Arg2>(l, index + 2);
	auto arg3 = LuaPull<Arg3>(l, index + 3);
	auto arg4 = LuaPull<Arg4>(l, index + 4);
	return (ptr->*fn)(arg1, arg2, arg3, arg4);
}

template <class T, typename Ret, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
Ret pi_lua_multiple_call(lua_State *l, int index, T *ptr, Ret (T::*fn)(Arg1, Arg2, Arg3, Arg4, Arg5))
{
	auto arg1 = LuaPull<Arg1>(l, index + 1);
	auto arg2 = LuaPull<Arg2>(l, index + 2);
	auto arg3 = LuaPull<Arg3>(l, index + 3);
	auto arg4 = LuaPull<Arg4>(l, index + 4);
	auto arg5 = LuaPull<Arg5>(l, index + 5);
	return (ptr->*fn)(arg1, arg2, arg3, arg4, arg5);
}

template <class T, typename Ret, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
Ret pi_lua_multiple_call(lua_State *l, int index, T *ptr, Ret (T::*fn)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6))
{
	auto arg1 = LuaPull<Arg1>(l, index + 1);
	auto arg2 = LuaPull<Arg2>(l, index + 2);
	auto arg3 = LuaPull<Arg3>(l, index + 3);
	auto arg4 = LuaPull<Arg4>(l, index + 4);
	auto arg5 = LuaPull<Arg5>(l, index + 5);
	auto arg6 = LuaPull<Arg6>(l, index + 6);
	return (ptr->*fn)(arg1, arg2, arg3, arg4, arg5, arg6);
}

template <class T, typename Ret, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7>
Ret pi_lua_multiple_call(lua_State *l, int index, T *ptr, Ret (T::*fn)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7))
{
	auto arg1 = LuaPull<Arg1>(l, index + 1);
	auto arg2 = LuaPull<Arg2>(l, index + 2);
	auto arg3 = LuaPull<Arg3>(l, index + 3);
	auto arg4 = LuaPull<Arg4>(l, index + 4);
	auto arg5 = LuaPull<Arg5>(l, index + 5);
	auto arg6 = LuaPull<Arg6>(l, index + 6);
	auto arg7 = LuaPull<Arg7>(l, index + 7);
	return (ptr->*fn)(arg1, arg2, arg3, arg4, arg5, arg6, arg7);
}

// Pull an object and arguments for the passed function from Lua.
// Intended to ease the amount of boilerplate code needed to bind a method to Lua.

template <typename Ret, typename... Args>
Ret pi_lua_generic_call(lua_State *l, int index, Ret (*fn)(Args...))
{
	return pi_lua_multiple_call<Ret, Args...>(l, index, fn);
}

template <class T, typename Ret, typename... Args>
Ret pi_lua_generic_call(lua_State *l, int index, Ret (T::*fn)(Args...))
{
	T *ptr = LuaPull<T>(l, index);
	return pi_lua_multiple_call<T, Ret, Args...>(l, index, ptr, fn);
}
