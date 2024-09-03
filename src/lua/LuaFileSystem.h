// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LUAFILESYSTEM_H
#define _LUAFILESYSTEM_H

#include "lua/Lua.h"

namespace LuaFileSystem {
	void Register();

	void register_raw_io_open_function(lua_CFunction open);
	int l_patched_io_open(lua_State* L);
} // namespace LuaFileSystem

#endif
