// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LUAREF_H
#define _LUAREF_H

#include "JsonFwd.h"
#include <cassert>
#include <lua.hpp>
#include <vector>

class LuaRef {
public:
	LuaRef() :
		m_lua(0),
		m_id(LUA_NOREF),
		m_copycount(nullptr) {}
	LuaRef(lua_State *l, int index);
	LuaRef(const LuaRef &ref);
	~LuaRef();
	const LuaRef &operator=(const LuaRef &ref);
	bool operator==(const LuaRef &ref) const;

	void PushCopyToStack() const;

	lua_State *GetLua() const { return m_lua; }

	bool IsValid() const { return m_lua && m_id != LUA_NOREF; }
	bool IsNil() const { return m_lua && m_id == LUA_REFNIL; }

	void SaveToJson(Json &jsonObj);
	void LoadFromJson(const Json &jsonObj);
	void Unref();

private:
	lua_State *m_lua;
	int m_id;
	int *m_copycount;

	// Does everything we need: get the table, or create it if it doesn't exist.
	// Yay lua aux lib !
	void PushGlobalToStack() const { luaL_getsubtable(m_lua, LUA_REGISTRYINDEX, "LuaRef"); }

	void CheckCopyCount();
};

inline void pi_lua_generic_push(lua_State *l, const LuaRef &r)
{
	assert(r.GetLua() == l);
	r.PushCopyToStack();
}

inline void pi_lua_generic_pull(lua_State *l, int index, LuaRef &r)
{
	r = LuaRef(l, index);
}

inline bool pi_lua_strict_pull(lua_State *l, int index, LuaRef &r)
{
	r = LuaRef(l, index);
	return true;
}

#endif
