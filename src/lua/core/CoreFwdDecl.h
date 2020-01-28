// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "../LuaUtils.h"

// Forward-declared from ../LuaUtils.cpp
int luaopen_utils(lua_State *L);

// Set up the import() functionality
int luaopen_import(lua_State *L);

// Internal functions corresponding to pi_lua_import, but which leave the error message on the stack for Lua
bool lua_import_core(lua_State *L, const std::string &importName);
bool lua_import(lua_State *L, const std::string &importName, bool isFullName = false);

// Forward-declared from Sandbox.cpp
void pi_lua_dofile(lua_State *l, const FileSystem::FileData &code, int nret = 0);
