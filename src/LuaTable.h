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
	LuaTable();	// Noop.
	LuaTable(const LuaTable & ref); // Copy constructor.
	explicit LuaTable(lua_State * l, int index); // Use an existing table on the stack. Index 0 will create a new table.
	~LuaTable();

	const LuaTable & operator=(const LuaTable & ref);
	bool operator==(const LuaTable & ref) const;
	bool operator<(const LuaTable & ref) const {
		assert(g_lua && m_lua == g_lua && ref.m_lua == g_lua);
		return m_id < ref.m_id;
	}
	template <class Key> void PushValueToStack(const Key & key) const;
	template <class Value, class Key> Value Get(const Key & key) const;
	template <class Value, class Key> void Set(const Key & key, const Value & value);

	// Push a copy of the current table on the top of the stack.
	// The caller is expected to clean it up itself.
	void PushCopyToStack() const;

	lua_State * GetLua() const { return m_lua; }

	static void Init(lua_State * l);
	static void Uninit(lua_State * l);

private:
	int m_id;
	lua_State * m_lua;

	static lua_State * g_lua;
	static std::vector<int> g_copy_count;

	void PushGlobalToStack() const;

	void CheckCopyCount();
};

#include "LuaPushPull.h"

template <class Key> void LuaTable::PushValueToStack(const Key & key) const {
	PushCopyToStack();
	lua_push(m_lua, key);
	lua_gettable(m_lua, -2);
	lua_remove(m_lua, -2);
}

template <class Value, class Key> Value LuaTable::Get(const Key & key) const {
	Value return_value;
	PushValueToStack(key);
	lua_pull(m_lua, -1, return_value);
	lua_pop(m_lua, 1);
	return return_value;
}

template <class Value, class Key> void LuaTable::Set(const Key & key, const Value & value) {
	PushCopyToStack();
	lua_push(m_lua, key);
	lua_push(m_lua, value);
	lua_settable(m_lua, -3);
	lua_pop(m_lua, 1);
}

#endif
