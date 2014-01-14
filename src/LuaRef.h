// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LUAREF_H
#define _LUAREF_H

#include "lua/lua.hpp"
#include <vector>

class LuaRef {
public:
	LuaRef(): m_lua(0), m_id(LUA_NOREF), m_copycount(new int(0)) {}
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
	int * m_copycount;

	// Does everything we need: get the table, or create it if it doesn't exist.
	// Yay lua aux lib !
	void PushGlobalToStack() const {luaL_getsubtable(m_lua, LUA_REGISTRYINDEX, "LuaRef");}

	void CheckCopyCount();
};

#endif
