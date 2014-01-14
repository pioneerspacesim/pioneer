// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaFileSystem.h"
#include "LuaObject.h"
#include "LuaConstants.h"
#include "FileSystem.h"
#include "Pi.h"

/*
 * Interface: FileSystem
 *
 * A global table that provides access to the filesystem.
 *
 * This interface is protected. It can only be used by scripts in Pioneer's
 * data directory. Mods and scripts in the user directory that try to use it
 * will get a Lua error.
 */

/*
 * Function: ReadDirectory
 *
 * > local files, dirs = FileSystem.ReadDirectory(root, path)
 *
 * Return a list of files and dirs in the specified directory.
 *
 * Parameters:
 *
 *   root - a <FileSystemRoot> constant for the root of the dir to read from
 *   path - optional. a directory under the root
 *
 * Returns:
 *
 *   files - a list of files as full paths from the root
 *   dirs - a list of dirs as full paths from the root
 *
 * Availability:
 *
 *   alpha 26
 *
 * Status:
 *
 *   experimental
 */
static int l_filesystem_read_dir(lua_State *l)
{
	LuaFileSystem::Root root = static_cast<LuaFileSystem::Root>(LuaConstants::GetConstantFromArg(l, "FileSystemRoot", 1));
	std::string path;
	if (lua_gettop(l) > 1)
		path = luaL_checkstring(l, 2);

	FileSystem::FileSource *fs = 0;
	switch (root) {
		case LuaFileSystem::ROOT_USER:
			fs = &FileSystem::userFiles;
			break;

		case LuaFileSystem::ROOT_DATA:
			fs = &FileSystem::gameDataFiles;
			break;
	}

	{
		try {
			const FileSystem::FileInfo &info = fs->Lookup(path);
			if (!info.IsDir()) {
				luaL_error(l, "'%s' is not a directory", path.c_str());
				return 0;
			}
		}
		catch (std::invalid_argument) {
			luaL_error(l, "'%s' is not a valid path", path.c_str());
			return 0;
		}
	}

	FileSystem::FileEnumerator files(*fs, FileSystem::FileEnumerator::IncludeDirs);
	files.AddSearchRoot(path);

	lua_newtable(l);
	int filesTable = lua_gettop(l);
	lua_newtable(l);
	int dirsTable = lua_gettop(l);

	for (; !files.Finished(); files.Next()) {
		const FileSystem::FileInfo &info = files.Current();

		lua_pushlstring(l, info.GetName().c_str(), info.GetName().size());

		if (info.IsDir())
			lua_rawseti(l, dirsTable, lua_rawlen(l, dirsTable)+1);
		else
			lua_rawseti(l, filesTable, lua_rawlen(l, filesTable)+1);
	}

	return 2;
}

/*
 * Function: JoinPath
 *
 * > local path = FileSystem.JoinPath(...)
 *
 * Join the passed arguments into a path, correctly handling separators and .
 * and .. special dirs.
 *
 * Availability:
 *
 *   alpha 26
 *
 * Status:
 *
 *   experimental
 */
static int l_filesystem_join_path(lua_State *l)
{
	try {
		std::string path;
		for (int i = 1; i <= lua_gettop(l); i++)
			path = FileSystem::JoinPath(path, luaL_checkstring(l, i));
		path = FileSystem::NormalisePath(path);
		lua_pushlstring(l, path.c_str(), path.size());
		return 1;
	}
	catch (std::invalid_argument) {
		luaL_error(l, "result is not a valid path");
		return 0;
	}
}

void LuaFileSystem::Register()
{
	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);

	static const luaL_Reg l_methods[] = {
		{ "ReadDirectory", l_filesystem_read_dir },
		{ "JoinPath",      l_filesystem_join_path },
		{ 0, 0 }
	};

	lua_getfield(l, LUA_REGISTRYINDEX, "CoreImports");
	LuaObjectBase::CreateObject(l_methods, 0, 0, true); // protected interface
	lua_setfield(l, -2, "FileSystem");
	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);
}
