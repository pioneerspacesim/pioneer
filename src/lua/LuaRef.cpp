// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaRef.h"
#include "GameSaveError.h"
#include "Json.h"
#include "Lua.h"
#include "LuaSerializer.h"
#include <cassert>

LuaRef::LuaRef(const LuaRef &ref) :
	m_lua(ref.m_lua),
	m_id(ref.m_id),
	m_copycount(ref.m_copycount)
{
	if (m_lua && m_id != LUA_NOREF)
		++(*m_copycount);
}

const LuaRef &LuaRef::operator=(const LuaRef &ref)
{
	if (&ref == this)
		return ref;
	Unref();

	m_lua = ref.m_lua;
	m_id = ref.m_id;
	m_copycount = ref.m_copycount;
	if (m_lua && m_id != LUA_NOREF)
		++(*m_copycount);
	return *this;
}

LuaRef::~LuaRef()
{
	Unref();
}

void LuaRef::Unref()
{
	if (m_id != LUA_NOREF && m_lua) {
		--(*m_copycount);
		CheckCopyCount();
	}
}

bool LuaRef::operator==(const LuaRef &ref) const
{
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

void LuaRef::CheckCopyCount()
{
	if (*m_copycount <= 0) {
		delete m_copycount;
		if (!m_lua)
			return;
		PushGlobalToStack();
		luaL_unref(m_lua, -1, m_id);
		lua_pop(m_lua, 1);
		m_id = LUA_NOREF;
	}
}

LuaRef::LuaRef(lua_State *l, int index) :
	m_lua(l),
	m_id(LUA_NOREF)
{
	assert(m_lua && index);
	index = lua_absindex(m_lua, index);

	PushGlobalToStack();
	lua_pushvalue(m_lua, index);
	m_id = luaL_ref(m_lua, -2);
	lua_pop(l, 1); // Pop global.

	m_copycount = new int(1);
}

void LuaRef::PushCopyToStack() const
{
	assert(m_lua && m_id != LUA_NOREF);
	PushGlobalToStack();
	lua_rawgeti(m_lua, -1, m_id);
	lua_remove(m_lua, -2);
}

void LuaRef::SaveToJson(Json &jsonObj)
{
	assert(m_lua && m_id != LUA_NOREF);

	LUA_DEBUG_START(m_lua);

	lua_getfield(m_lua, LUA_REGISTRYINDEX, "PiSerializer");
	LuaSerializer *serializer = static_cast<LuaSerializer *>(lua_touserdata(m_lua, -1));
	lua_pop(m_lua, 1);

	if (!serializer) {
		LUA_DEBUG_END(m_lua, 0);
		return;
	}

	Json out;
	PushCopyToStack();
	LuaSerializer::pickle_json(m_lua, -1, out);
	lua_pop(m_lua, 1);
	jsonObj["lua_ref_json"] = out;

	LUA_DEBUG_END(m_lua, 0);
}

void LuaRef::LoadFromJson(const Json &jsonObj)
{
	if (!m_lua) {
		m_lua = Lua::manager->GetLuaState();
	}

	LUA_DEBUG_START(m_lua);

	lua_getfield(m_lua, LUA_REGISTRYINDEX, "PiSerializer");
	LuaSerializer *serializer = static_cast<LuaSerializer *>(lua_touserdata(m_lua, -1));
	lua_pop(m_lua, 1);

	if (!serializer) {
		LUA_DEBUG_END(m_lua, 0);
		return;
	}

	if (jsonObj.count("lua_ref_json")) {
		LuaSerializer::unpickle_json(m_lua, jsonObj["lua_ref_json"]);
	} else if (jsonObj.count("lua_ref")) {
		// old text-based serialization
		throw SavedGameCorruptException();
	} else {
		throw SavedGameCorruptException();
	}

	// Lua stack: loaded
	lua_getfield(m_lua, LUA_REGISTRYINDEX, "PiLuaRefLoadTable"); // loaded, reftable
	lua_pushvalue(m_lua, -2); // loaded, reftable, copy
	lua_gettable(m_lua, -2); // loaded, reftable, luaref
	// Check whether this table has been referenced before
	if (lua_isnil(m_lua, -1)) {
		// If not, mark it as referenced
		*this = LuaRef(m_lua, -3);
		lua_pushvalue(m_lua, -3);
		lua_pushlightuserdata(m_lua, this);
		lua_settable(m_lua, -4);
	} else {
		LuaRef *origin = static_cast<LuaRef *>(lua_touserdata(m_lua, -1));
		*this = *origin;
	}

	lua_pop(m_lua, 3);

	LUA_DEBUG_END(m_lua, 0);
}
