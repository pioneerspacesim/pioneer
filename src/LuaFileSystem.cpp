#include "LuaFileSystem.h"
#include "LuaObject.h"
#include "LuaConstants.h"
#include "FileSystem.h"
#include "Pi.h"

/*
 * Interface: FileSystem
 *
 * A global table that provides access to the filesystem.
 */

/*
 * Method: ReadDirectory
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

	FileSystem::FileSourceFS userFiles(FileSystem::FileSourceFS(FileSystem::GetUserDir()));
	FileSystem::FileSource *fs;
	switch (root) {
		case LuaFileSystem::ROOT_USER:
			fs = &userFiles;
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

		lua_pushlstring(l, info.GetPath().c_str(), info.GetPath().size());
		
		if (info.IsDir())
			lua_rawseti(l, dirsTable, lua_rawlen(l, dirsTable)+1);
		else
			lua_rawseti(l, filesTable, lua_rawlen(l, filesTable)+1);
	}

	return 2;
}

void LuaFileSystem::Register()
{
	static const luaL_Reg l_methods[] = {
		{ "ReadDirectory", l_filesystem_read_dir },
		{ 0, 0 }
	};

	LuaObjectBase::CreateObject(l_methods, 0, 0);
	lua_setglobal(Pi::luaManager->GetLuaState(), "FileSystem");
}
