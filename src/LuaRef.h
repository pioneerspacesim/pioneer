#ifndef _LUAREF_H
#define _LUAREF_H

#include "lua/lua.hpp"
#include <vector>

class LuaRef {
public:
	static void Init(lua_State * l);
	static void Uninit(lua_State * l);

	LuaRef(): m_lua(0), m_id(0) {}
	LuaRef(lua_State * l, int index);
	LuaRef(const LuaRef & ref);
	~LuaRef();
	const LuaRef & operator=(const LuaRef & ref);
	bool operator==(const LuaRef & ref) const;

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

#endif
