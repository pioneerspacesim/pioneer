// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LUAPUSHPULL_H
#define _LUAPUSHPULL_H

#include "lua/lua.hpp"
#include "LuaTable.h"
#include "LuaObject.h"
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
inline void pi_lua_generic_push(lua_State * l, LuaTable value) {
	assert(value.GetLua() == l);
	lua_pushvalue(l, value.GetIndex());
}
template <class T> void pi_lua_generic_push(lua_State * l, T* value) {
	assert(l == Lua::manager->GetLuaState());
	if (value)
		LuaObject<T>::PushToLua(value);
	else
		lua_pushnil(l);
}

inline void pi_lua_generic_pull(lua_State * l, int index, bool & out) { out = lua_toboolean(l, index); }
inline void pi_lua_generic_pull(lua_State * l, int index, int & out) { out = lua_tointeger(l, index); }
inline void pi_lua_generic_pull(lua_State * l, int index, unsigned int & out) { out = lua_tounsigned(l, index); }
inline void pi_lua_generic_pull(lua_State * l, int index, double & out) { out = lua_tonumber(l, index); }
inline void pi_lua_generic_pull(lua_State * l, int index, const char * & out) { out = lua_tostring(l, index); }
inline void pi_lua_generic_pull(lua_State * l, int index, std::string & out) {
	size_t len;
	const char *buf = lua_tolstring(l, index, &len);
	std::string(buf, len).swap(out);
}
inline void pi_lua_generic_pull(lua_State * l, int index, LuaTable & out) {
	out = LuaTable(l, index);
}
template <class T> void pi_lua_generic_pull(lua_State * l, int index, T* & out) {
	assert(l == Lua::manager->GetLuaState());
	out = LuaObject<T>::GetFromLua(index);
}

#endif
