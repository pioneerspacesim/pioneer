// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "../LuaUtils.h"

// Forward-declared from ../LuaUtils.cpp
int luaopen_utils(lua_State *L);

// Set up the import() functionality
int luaopen_import(lua_State *L);

// Forward-declared from Sandbox.cpp
void pi_lua_dofile(lua_State *l, const FileSystem::FileData &code, int nret = 0);
