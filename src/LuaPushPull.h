#ifndef _LUAPUSHPULL_H
#define _LUAPUSHPULL_H

#include "lua/lua.hpp"
#include "LuaTable.h"
#include "LuaObject.h"
#include "Pi.h"
#include <string>

inline void lua_push(lua_State * l, bool value) { lua_pushboolean(l, value); }
inline void lua_push(lua_State * l, int value) { lua_pushinteger(l, value); }
inline void lua_push(lua_State * l, double value) { lua_pushnumber(l, value); }
inline void lua_push(lua_State * l, const char * value) { lua_pushstring(l, value); }
inline void lua_push(lua_State * l, const std::string & value) { lua_pushstring(l, value.c_str()); }
inline void lua_push(lua_State * l, LuaTable value) {
	assert(value.GetLua() == l);
	lua_pushvalue(l, value.GetIndex());
}
template <class T> void lua_push(lua_State * l, T* value) {
	assert(l == Pi::luaManager->GetLuaState());
	if (value)
		LuaObject<T>::PushToLua(value);
	else
		lua_pushnil(l);
}

inline void lua_pull(lua_State * l, int index, bool & out) { out = lua_toboolean(l, index); }
inline void lua_pull(lua_State * l, int index, int & out) { out = lua_tointeger(l, index); }
inline void lua_pull(lua_State * l, int index, double & out) { out = lua_tonumber(l, index); }
inline void lua_pull(lua_State * l, int index, const char * & out) { out = lua_tostring(l, index); }
inline void lua_pull(lua_State * l, int index, std::string & out) { out = lua_tostring(l, index); }
inline void lua_pull(lua_State * l, int index, LuaTable & out) {
	out.SetLua(l);
	out.SetIndex(index);
}
template <class T> void lua_pull(lua_State * l, int index, T* & out) {
	assert(l == Pi::luaManager->GetLuaState());
	out = LuaObject<T>::GetFromLua(index);
}

#endif
