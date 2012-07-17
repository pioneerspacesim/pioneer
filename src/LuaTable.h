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

	// Push a copy of the current table on the top of the stack.
	// The caller is expected to clean it up itself.
	void PushCopyToStack() const;

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

#endif
