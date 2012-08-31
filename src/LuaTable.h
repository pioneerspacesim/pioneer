#ifndef _LUATABLE_H
#define _LUATABLE_H

#include <exception>
#include <vector>
#include <cassert>

#include "lua/lua.hpp"

class UninitalizedLuaTable: std::exception {};

class LuaTable {
public:
	// For now, every lua_State * can only be NULL or Pi::LuaManager->GetLuaState();
	LuaTable(const LuaTable & ref): m_lua(ref.m_lua), m_index(ref.m_index) {} // Copy constructor.
	LuaTable(lua_State * l, int index): m_lua(l), m_index(lua_absindex(l, index)) {assert(lua_istable(m_lua, m_index));}
	explicit LuaTable(lua_State * l): m_lua(l) {
		lua_newtable(m_lua);
		m_index = lua_gettop(l); 
	}

	virtual ~LuaTable() {};

	const LuaTable & operator=(const LuaTable & ref) { m_lua = ref.m_lua; m_index = ref.m_index; return *this;}
	bool operator==(const LuaTable & ref) const;
	bool operator<(const LuaTable & ref) const {
		assert(m_lua && ref.m_lua == m_lua);
		return m_index < ref.m_index;
	}
	template <class Key> void PushValueToStack(const Key & key) const;
	template <class Value, class Key> Value Get(const Key & key) const;
	template <class Value, class Key> void Set(const Key & key, const Value & value) const;

	lua_State * GetLua() const { return m_lua; }
	int GetIndex() const { return m_index; }

protected:
	lua_State * m_lua;
	int m_index;

};

class PersistentTable {
public:
	static void Init(lua_State * l);
	static void Uninit(lua_State * l);

	PersistentTable(): m_lua(0), m_id(0) {}
	PersistentTable(lua_State * l, int index);
	PersistentTable(const PersistentTable & ref);
	~PersistentTable();
	const PersistentTable & operator=(const PersistentTable & ref);
	bool operator==(const PersistentTable & ref) const;

	const LuaTable GetLuaTable() { PushCopyToStack(); return LuaTable(m_lua, lua_gettop(m_lua)); }
	void PushCopyToStack() const;

	lua_State * GetLua() const { return m_lua; }
private:
	lua_State * m_lua;
	int m_id;
	static std::vector<int> g_copy_count;
	static lua_State * g_lua;

	void PushGlobalToStack() const;

	void CheckCopyCount();
};

#include "LuaPushPull.h"

template <class Key> void LuaTable::PushValueToStack(const Key & key) const {
	lua_push(m_lua, key);
	lua_gettable(m_lua, m_index);
}

template <class Value, class Key> Value LuaTable::Get(const Key & key) const {
	Value return_value;
	PushValueToStack(key);
	lua_pull(m_lua, -1, return_value);
	lua_pop(m_lua, 1);
	return return_value;
}

template <class Value, class Key> void LuaTable::Set(const Key & key, const Value & value) const {
	lua_push(m_lua, key);
	lua_push(m_lua, value);
	lua_settable(m_lua, m_index);
}

#endif
