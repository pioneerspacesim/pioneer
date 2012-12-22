// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LUATABLE_H
#define _LUATABLE_H

#include <cassert>

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

#endif
