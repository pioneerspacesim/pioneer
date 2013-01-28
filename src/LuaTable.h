// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LUATABLE_H
#define _LUATABLE_H

#include <cassert>
#include <map>
#include <vector>

#include "lua/lua.hpp"
#include "LuaRef.h"

class LuaTable {
public:
	// For now, every lua_State * can only be NULL or Pi::LuaManager->GetLuaState();
	LuaTable(const LuaTable & ref): m_lua(ref.m_lua), m_index(ref.m_index) {} // Copy constructor.
	LuaTable(const LuaRef & r): m_lua(r.GetLua()) {
		r.PushCopyToStack();
		m_index = lua_gettop(m_lua);
	}

	LuaTable(lua_State * l, int index): m_lua(l), m_index(lua_absindex(l, index)) {assert(lua_istable(m_lua, m_index));}
	explicit LuaTable(lua_State * l): m_lua(l) {
		lua_newtable(m_lua);
		m_index = lua_gettop(l);
	}

	~LuaTable() {};

	const LuaTable & operator=(const LuaTable & ref) { m_lua = ref.m_lua; m_index = ref.m_index; return *this;}
	template <class Key> void PushValueToStack(const Key & key) const;
	template <class Value, class Key> Value Get(const Key & key) const;
	template <class Value, class Key> void Set(const Key & key, const Value & value) const;

	template <class Key, class Value> std::map<Key, Value> GetMap() const;
	template <class Key, class Value> void LoadMap(const std::map<Key, Value> & m) const;
	template <class Value> std::vector<Value> GetVector() const;
	template <class Value> void LoadVector(const std::vector<Value> & m) const;

	lua_State * GetLua() const { return m_lua; }
	int GetIndex() const { return m_index; }

private:
	LuaTable(): m_lua(0), m_index(0) {} //Protected : invalid tables shouldn't be out there.
	lua_State * m_lua;
	int m_index;

};

#include "LuaPushPull.h"

template <class Key> void LuaTable::PushValueToStack(const Key & key) const {
	pi_lua_generic_push(m_lua, key);
	lua_gettable(m_lua, m_index);
}

template <class Value, class Key> Value LuaTable::Get(const Key & key) const {
	Value return_value;
	PushValueToStack(key);
	pi_lua_generic_pull(m_lua, -1, return_value);
	lua_pop(m_lua, 1);
	return return_value;
}

template <class Value, class Key> void LuaTable::Set(const Key & key, const Value & value) const {
	pi_lua_generic_push(m_lua, key);
	pi_lua_generic_push(m_lua, value);
	lua_settable(m_lua, m_index);
}

template <class Key, class Value> std::map<Key, Value> LuaTable::GetMap() const {
	std::map<Key, Value> ret;
	lua_pushnil(m_lua);
	while(lua_next(m_lua, m_index)) {
		Key k;
		Value v;
		if (pi_lua_strict_pull(m_lua, -2, k)) {
			pi_lua_strict_pull(m_lua, -1, v);
			ret[k] = v;
		}
		lua_pop(m_lua, 1);
	}
	return ret;
}

template <class Value> std::vector<Value> LuaTable::GetVector() const {
	std::vector<Value> ret;
	lua_len(m_lua, m_index);
	int len = lua_tointeger(m_lua, -1);
	lua_pop(m_lua, 1);
	ret.reserve(len);
	for(int i = 1; i <= len; ++i) {
		Value val;
		lua_rawgeti(m_lua, m_index, i);
		if (pi_lua_strict_pull(m_lua, -1, val))
			ret.push_back(val);
		lua_pop(m_lua, 1);
	}
    return ret;
}

template <class Key, class Value> void LuaTable::LoadMap(const std::map<Key, Value> & m) const {
	for (typename std::map<Key, Value>::const_iterator it = m.begin();
			it != m.end() ; ++it)
		Set(it->first, it->second);
}

template <class Value> void LuaTable::LoadVector(const std::vector<Value> & v) const {
	lua_len(m_lua, m_index);
	int current_length = lua_tointeger(m_lua, -1);
	lua_pop(m_lua, 1);
	for (unsigned int i = 0;  i < v.size() ; ++i)
		Set(current_length+i+1, v[i]);
}

#endif
