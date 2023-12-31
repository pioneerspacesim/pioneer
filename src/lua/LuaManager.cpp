// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaManager.h"
#include "FileSystem.h"
#include "utils.h"

#include <cstdlib>

bool instantiated = false;

LuaManager::LuaManager() :
	m_lua(0)
{
	if (instantiated) {
		Output("Can't instantiate more than one LuaManager");
		abort();
	}

	m_lua = luaL_newstate();
	pi_lua_open_standard_base(m_lua);
	lua_atpanic(m_lua, pi_lua_panic);

	// this will print nothing currently because there's no stack yet, but it means that the function is included
	// in the codebase and thus available via the "immediate" window in the MSVC debugger for us in any C++ lua function
	pi_lua_stacktrace(m_lua);

	// this will print the lua version as a string and ensure the function is included for use while debugging.
	lua_pushstring(m_lua, LUA_VERSION);
	pi_lua_printvalue(m_lua, -1);
	lua_pop(m_lua, 1);

	instantiated = true;
}

LuaManager::~LuaManager()
{
	lua_close(m_lua);

	instantiated = false;
}

size_t LuaManager::GetMemoryUsage() const
{
	int kb = lua_gc(m_lua, LUA_GCCOUNT, 0);
	int b = lua_gc(m_lua, LUA_GCCOUNTB, 0);
	return (size_t(kb) * 1024) + b;
}

void LuaManager::CollectGarbage()
{
	lua_gc(m_lua, LUA_GCCOLLECT, 0);
}
