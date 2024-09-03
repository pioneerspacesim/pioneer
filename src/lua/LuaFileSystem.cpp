// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaFileSystem.h"
#include "FileSystem.h"
#include "LuaConstants.h"
#include "LuaObject.h"
#include "Pi.h"

#include "core/StringUtils.h"

struct SplitURI
{
	std::string_view full_path;
	std::string_view scheme;
	std::string_view rel_path;
};

SplitURI ParseURIArg(lua_State* l, int numArg)
{
	SplitURI res;
	res.full_path = LuaPull<std::string_view>(l, numArg);
	size_t sep_pos = res.full_path.find("://");
	if (sep_pos != std::string_view::npos)
	{
		res.scheme = res.full_path.substr(0, sep_pos);
		sep_pos += 3;
		res.rel_path = res.full_path.substr(sep_pos);
	}
	return res;
}

struct FileSource
{
	FileSystem::FileSource* source;
	bool					is_read_write;
};

FileSource DetermineFileSource(std::string_view scheme)
{
	FileSource res;
	if (compare_ci(scheme, "user"))
	{
		res.source = &FileSystem::userFiles;
		res.is_read_write = true;
	} else if (compare_ci(scheme, "data"))
	{
		res.source = &FileSystem::gameDataFiles;
		res.is_read_write = false;
	} else
	{
		res.source = nullptr;
		res.is_read_write = false;
	}
	return res;
}

static lua_CFunction l_original_io_open = nullptr;

void LuaFileSystem::register_raw_io_open_function(lua_CFunction open)
{
	l_original_io_open = open;
}

int LuaFileSystem::l_patched_io_open(lua_State* L)
{
	SplitURI split_uri = ParseURIArg(L, 1);
	const FileSource fs = DetermineFileSource(split_uri.scheme);
	if (!fs.source)
	{
		return luaL_error(L, "Path '%s' does not contain a valid scheme, must be user:// or data://", split_uri.full_path.data());
	}

	std::string_view mode_str = LuaPull<std::string_view>(L, 2, "r");
	if (mode_str.find_first_of("aw+") != std::string_view::npos && !fs.is_read_write) {
		return luaL_error(L, "File '%s' is read-only.", split_uri.full_path.data());
	}

	try
	{
		::std::string abs_path = fs.source->Lookup(std::string(split_uri.rel_path)).GetAbsolutePath();
		LuaPush(L, abs_path);
		lua_replace(L, 1);

		const int rv = l_original_io_open(L);

		return rv;
	}
	catch (const std::invalid_argument&)
	{
		return luaL_error(L, "'%s' is not a valid file in its scheme root' - Is the file location within the root?", split_uri.full_path.data());
	}
}

static void push_date_time(lua_State *l, const Time::DateTime &dt)
{
	int year, month, day, hour, minute, second;
	dt.GetDateParts(&year, &month, &day);
	dt.GetTimeParts(&hour, &minute, &second);

	lua_newtable(l);
	pi_lua_settable(l, "year", year);
	pi_lua_settable(l, "month", month);
	pi_lua_settable(l, "day", day);
	pi_lua_settable(l, "hour", hour);
	pi_lua_settable(l, "minute", minute);
	pi_lua_settable(l, "second", second);
	pi_lua_settable(l, "timestamp", dt.ToGameTime());
}

/*
 * Function: ReadDirectory
 *
 * > local files, dirs = FileSystem.ReadDirectory(path)
 *
 * Return a list of files and dirs in the specified directory.
 *
 * Parameters:
 *
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
	SplitURI split_uri = ParseURIArg(l, 1);
	const FileSource fs = DetermineFileSource(split_uri.scheme);
	if (!fs.source)
	{
		return luaL_error(l, "Path '%s' does not contain a valid scheme, must be user:// or data://", split_uri.full_path.data());
	}

	// shame these methods can't take a string_view.
	std::string rel_path = std::string(split_uri.rel_path);

	{
		try {
			FileSystem::FileInfo info = fs.source->Lookup(rel_path);
			if (!info.IsDir()) {
				return luaL_error(l, "'%s' is not a directory", split_uri.full_path.data());
			}
		} catch (const std::invalid_argument &) {
			return luaL_error(l, "'%s' is not a valid path", split_uri.full_path.data());
		}
	}

	lua_newtable(l);
	int filesTable = lua_gettop(l);
	lua_newtable(l);
	int dirsTable = lua_gettop(l);

	int dirs_len = 0;
	int files_len = 0;

	for (const auto &info : fs.source->Enumerate(rel_path, FileSystem::FileEnumerator::IncludeDirs))
	{
		lua_newtable(l);
		pi_lua_settable(l, "name", info.GetName().c_str());
		push_date_time(l, info.GetModificationTime());
		lua_setfield(l, -2, "mtime");

		if (info.IsDir())
			lua_rawseti(l, dirsTable, ++dirs_len);
		else
			lua_rawseti(l, filesTable, ++files_len);
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
	} catch (const std::invalid_argument &) {
		return luaL_error(l, "result is not a valid path");
	}
}


/*
 * Function: MakeDirectory
 *
 * > local success = FileSystem.MakeDirectory( dir_name )
 *
 * Creating the given directory if it's missing, returning a boolean
 * indicating success
 *
 * Availability:
 *
 *   Oct 2023
 *
 * Status:
 *
 *   experimental
 */
static int l_filesystem_make_directory(lua_State *l)
{
	SplitURI split_uri = ParseURIArg(l, 1);
	const FileSource fs = DetermineFileSource(split_uri.scheme);
	if (!fs.source)
	{
		return luaL_error(l, "Path '%s' does not contain a valid scheme, must be user:// or data://", split_uri.full_path.data());
	}

	if (!fs.is_read_write)
	{
		return luaL_error(l, "'%s' does not support file operations that modify it, only things in the user folder do", split_uri.full_path.data());
	}

	try {

		FileSystem::FileSourceFS& wfs = *static_cast<FileSystem::FileSourceFS*>(fs.source);

		// shame these methods can't take a string_view.
		std::string rel_path = std::string(split_uri.rel_path);
		// At the moment, anything with a filesourceFS is also writeable, so the write permission test
		// above also validates this...
		wfs.MakeDirectory(rel_path);

		FileSystem::FileInfo f = wfs.Lookup(rel_path);

		lua_pushboolean(l, f.IsDir());
	} catch (const std::invalid_argument&) {
		return luaL_error(l, "unable to create directory '%s' the argument is invalid", split_uri.full_path.data());
	}
	return 1;
}

void LuaFileSystem::Register()
{
	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);

	static const luaL_Reg l_methods[] = {
		{ "ReadDirectory", l_filesystem_read_dir },
		{ "JoinPath", l_filesystem_join_path },
		{ "MakeDirectory", l_filesystem_make_directory },
		{ "Open", l_patched_io_open },
		{ 0, 0 }
	};

	lua_getfield(l, LUA_REGISTRYINDEX, "CoreImports");
	LuaObjectBase::CreateObject(l_methods, 0, 0, true); // protected interface
	lua_setfield(l, -2, "FileSystem");
	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);
}
