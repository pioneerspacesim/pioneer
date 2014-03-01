// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaRef.h"
#include <cassert>

LuaRef::LuaRef(const LuaRef & ref): m_lua(ref.m_lua), m_id(ref.m_id), m_copycount(ref.m_copycount) {
	if (m_lua && m_id != LUA_NOREF)
		++(*m_copycount);
}

const LuaRef & LuaRef::operator=(const LuaRef & ref) {
	if (&ref == this)
		return ref;
	if (m_id != LUA_NOREF && m_lua) {
		--(*m_copycount);
	}
	CheckCopyCount();
	m_lua = ref.m_lua;
	m_id = ref.m_id;
	m_copycount = ref.m_copycount;
	if(m_lua && m_id)
		++(*m_copycount);
	return *this;
}

LuaRef::~LuaRef() {
	if (m_id != LUA_NOREF && m_lua) {
		--(*m_copycount);
		CheckCopyCount();
	}
}

bool LuaRef::operator==(const LuaRef & ref) const {
	if (ref.m_lua != m_lua)
		return false;
	if (ref.m_id == m_id)
		return true;

	ref.PushCopyToStack();
	PushCopyToStack();
	bool return_value = lua_compare(m_lua, -1, -2, LUA_OPEQ);
	lua_pop(m_lua, 2);
	return return_value;
}

void LuaRef::CheckCopyCount() {
	if (*m_copycount <= 0) {
		delete m_copycount;
		if (!m_lua)
			return;
		PushGlobalToStack();
		luaL_unref(m_lua, -1, m_id);
		lua_pop(m_lua, 1);
	}
}

LuaRef::LuaRef(lua_State * l, int index): m_lua(l), m_id(0) {
	assert(m_lua && index);
	index = lua_absindex(m_lua, index);

	PushGlobalToStack();
	lua_pushvalue(m_lua, index);
	m_id = luaL_ref(m_lua, -2);
	lua_pop(l, 1); // Pop global.

	m_copycount = new int(1);
}


void LuaRef::PushCopyToStack() const {
	assert(m_lua && m_id != LUA_NOREF);
	PushGlobalToStack();
	lua_rawgeti(m_lua, -1, m_id);
	lua_remove(m_lua, -2);
}

