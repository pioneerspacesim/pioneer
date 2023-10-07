// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LUAFILESYSTEM_H
#define _LUAFILESYSTEM_H

#include <string>

struct lua_State;

namespace LuaFileSystem {
	void Register();

	enum Root { // <enum scope='LuaFileSystem' name=FileSystemRoot prefix=ROOT_ public>
		ROOT_USER,
		ROOT_DATA
	};

	// will throw lua errors if not allowed
	// and return an empty string
	std::string lua_path_to_fs_path(lua_State* l, const char* root, const char* path, const char* access);
} // namespace LuaFileSystem

#endif
