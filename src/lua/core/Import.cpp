// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "CoreFwdDecl.h"
#include "FileSystem.h"
#include "utils.h"

#include <algorithm>
#include <deque>
#include <string>

#undef DEBUG_IMPORT
#ifdef DEBUG_IMPORT
#include "core/Log.h"
#define DEBUG_PRINTF Output
#define DEBUG_INDENTED_PRINTF Output
#define DEBUG_INDENT_INCREASE Log::IncreaseIndent
#define DEBUG_INDENT_DECREASE Log::DecreaseIndent
#else
#define DEBUG_PRINTF(...)
#define DEBUG_INDENTED_PRINTF(...)
#define DEBUG_INDENT_INCREASE()
#define DEBUG_INDENT_DECREASE()
#endif

/**
 *	We use Lua's require() functionality, but with a twist.
 *
 *	When specifying a module name to `require()`, beginning the module name
 *	with a separator (`.`) is syntactic sugar for prepending the source
 *	directory of the calling file to the module name, forming a system of
 *	relative name lookup.
 *
 *	Relative name lookup cannot ascend levels in the filesystem, nor can it be
 *	used to exit the virtual filesystem. It's merely a shorthand to implicitly
 *	specify the module name of the calling file as a prefix to the module name
 *	passed to `require()`. This is done as follows:
 *
 *		-- when called from myname/modname/mod-load.lua
 *		require '.module-A'
 *		-- this call is transformed into:
 *		require 'myname.modname.module-A'
 *
 *	Once the full module name has been established, it is run through several
 *	levels of lookup. First, the module name is tested against the import
 *	cache, then against the list of modules registered by C++ code. Finally,
 *	it is converted into a file path and tested against the filesystem to
 *	load a lua file from disk.
 *
 *	Filesystem lookup follows the usual rules; a file in a mod overrides files
 *	earlier in the mod load order, and the overriden files cannot be loaded
 *	from disk. If there is a C++ module with a specific name, it overrides all
 *	Lua files with that name.
 *
 *	The `package.core` object stores C++ modules for convenience. Access
 *	is by standard Lua notation - `package.core.PiGui.Modules.ModelSpinner`
 *	works just fine, as does `require 'PiGui.Modules.ModelSpinner'`.
 *
 *	The `package.reimport(name)` function purges the cache for a specific
 *	module name and repeats the loading process - useful for when a file
 *	changes on disk as part of development.
 */

// The cache table that stores file path -> module return value mappings
static const char *IMPORT_CACHE_NAME = "Imports";

// The cache table which contains module name -> file path mappings
static const char *MODULE_MAP_CACHE_NAME = "ImportPaths";

// simple struct to pass data between functions
struct ImportInfo {
	const std::string &importName;				 // original name argument from "import(importName)"
	std::string &fileName;						 // name of the existing file
	const bool &isFullName;						 // if true import will no try to load relative to the importDirectories
	std::vector<std::string> &importDirectories; // contans list of paths where file can be imported
};

/**
 * Returns path of last element in the lua stack
 */
static std::string get_caller(lua_State *L, int depth = 1)
{
	// XXX: Extraneous copy
	assert(L);
	LUA_DEBUG_START(L);

	std::string caller;
	lua_Debug ar;

	// if have stack and caller in the stack
	if (lua_getstack(L, depth, &ar) && lua_getinfo(L, "S", &ar) && ar.source) {
		int start = 0;
		int end = strlen(ar.source);

		// if we have an @, this is a file name. Strip it and return the file name
		if (ar.source[start] == '@') {
			start++;

			// strip the trusted annotation ("@[T] ") if present
			if ((ar.source[start] == '[') && (end > 5))
				start += 4;

			caller = std::string(&ar.source[start], end - start);
		}
	}

	// auto endTrimPos = caller.find_last_not_of(" \t\r\n\0");
	// if (endTrimPos != std::string::npos) {
	// 	caller.erase(endTrimPos + 1);
	// }

	LUA_DEBUG_END(L, 0);
	return caller;
}

/**
 * Generate a lua module name from a given input path.
 * If the path contains a directory component containing a separator (period)
 * this function will return an empty string, as lua module names cannot contain
 * a non-separating period.
 * This function discards the trailing component of the directory path. If you
 * wish to retain it, ensure the path string ends with '/'.
 */
static std::string path_to_module(std::string path)
{
	std::string module_name;

	// Loop through each path fragment and append to the module name
	// e.g. '/data/modules/path/to/file.lua' would become 'data.modules.path.to'
	size_t start = 0;
	size_t end = path.find_first_of('/', 1); // use start=1 to generate string::npos when the path == "/"

	while (end != std::string::npos) {
		auto fragment = path.substr(start, end - start);

		if (!fragment.empty()) {
			// Paths containing the separator character cannot be translated into a module name
			// this means you can't do implicit lookup using a path starting in a hidden directory, e.g.
			// /data/.local/mylib.lua
			if (fragment.find('.') != std::string::npos)
				return "";
			module_name.append(fragment).push_back('.');
		}

		start = end + 1;
		end = path.find_first_of('/', start);
	}

	if (!module_name.empty() && module_name.back() == '.')
		module_name.pop_back();

	return module_name;
}

// If the given file path is cached, returns the cached object on the stack.
// Otherwise if the path points to a valid file, executes that file and sets
// the result in the module cache.
// In all cases, if this function returns false the stack is unmodified.
static bool load_file_cached(lua_State *L, const std::string &path, int cacheIdx)
{
	LUA_DEBUG_START(L);
	cacheIdx = lua_absindex(L, cacheIdx);

	// If the object is in the import cache, return it.
	lua_getfield(L, cacheIdx, path.c_str());
	if (!lua_isnil(L, -1)) {
		DEBUG_INDENTED_PRINTF("-> loaded module file %s from cache.\n", path.c_str());
		LUA_DEBUG_END(L, 1);
		return true;
	} else
		lua_pop(L, 1);

	// FileSystem::JoinPathBelow can't join a path starting with '/' for some reason
	// TODO: find out why and remove that restriction.
	std::string filePath = (path[0] == '/') ? path.substr(1) : path;
	auto fileInfo = FileSystem::gameDataFiles.Lookup(filePath);

	if (!fileInfo.Exists()) {
		LUA_DEBUG_END(L, 0);
		return false;
	}

	// Load and run the file from disk
	auto data = fileInfo.Read();
	lua_checkstack(L, 5);
	pi_lua_dofile(L, *data, 1);

	DEBUG_INDENTED_PRINTF("-> loaded module file %s from disk\n", path.c_str());
	DEBUG_INDENTED_PRINTF("-> module returned a %s value\n", lua_typename(L, lua_type(L, -1)));

	// Since we loaded the module, we don't want to re-load it again.
	// Replace the nil return value with 'true' to signify a successful
	// module load
	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);
		lua_pushboolean(L, true);
	}

	// Store the module in the import cache
	lua_pushvalue(L, -1);
	lua_setfield(L, cacheIdx, path.c_str());

	LUA_DEBUG_END(L, 1);
	return true;
}

// Check the path cache for a module name to filesystem path mapping
// If present, this module has been successfully loaded before.
// This function returns an empty string if the module name has not been
// loaded before.
static std::string get_path_cache(lua_State *L, const std::string &moduleName)
{
	LUA_DEBUG_START(L);

	lua_getfield(L, LUA_REGISTRYINDEX, MODULE_MAP_CACHE_NAME);
	lua_getfield(L, -1, moduleName.c_str());
	std::string path;
	if (lua_isstring(L, -1))
		path = lua_tostring(L, -1);
	lua_pop(L, 2);
	LUA_DEBUG_END(L, 0);
	return path;
}

// Update the module name -> file path cache with a new entry
// This should only be called if the module was sucessfully loaded
static void set_path_cache(lua_State *L, const std::string &name, const std::string &path)
{
	LUA_DEBUG_START(L);

	lua_getfield(L, LUA_REGISTRYINDEX, MODULE_MAP_CACHE_NAME);
	lua_pushstring(L, path.c_str());
	lua_setfield(L, -2, name.c_str());
	lua_pop(L, 1);

	DEBUG_INDENTED_PRINTF("-> new module cache: %s -> %s\n", name.c_str(), path.c_str());

	LUA_DEBUG_END(L, 0);
}

// Follow the chain of module.name.table down the C++ core modules table
// If this finds a non-nil value at the end of a table (or userdata) chain,
// this function returns true and pushes that value on the stack.
static bool load_from_core(lua_State *L, const std::string &moduleName)
{
	LUA_DEBUG_START(L);

	lua_getfield(L, LUA_REGISTRYINDEX, "CoreImports");

	size_t start = 0;
	size_t end = moduleName.find_first_of('.');
	while (end != std::string::npos) {
		auto fragment = moduleName.substr(start, end - start);
		lua_getfield(L, -1, fragment.c_str());

		if (!lua_istable(L, -1) && !lua_isuserdata(L, -1)) {
			lua_pop(L, 2);
			LUA_DEBUG_END(L, 0);
			return false;
		}
		lua_remove(L, lua_gettop(L) - 1);

		start = end + 1;
		end = moduleName.find_first_of('.', start);
	}

	lua_getfield(L, -1, &moduleName[start]);
	lua_remove(L, lua_gettop(L) - 1);
	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);
		LUA_DEBUG_END(L, 0);
		return false;
	}

	LUA_DEBUG_END(L, 1);
	return true;
}

// Load, execute, and cache a lua module referred to with the given
// fully-qualified module name.
// This function DOES NOT accept filesystem paths; please convert them to a
// module name before calling this function. Thank you.
static bool lua_import_module(lua_State *L, const std::string &moduleName)
{
	LUA_DEBUG_START(L);

	DEBUG_INDENTED_PRINTF("[require '%s']: loading module started...\n", moduleName.c_str());
	DEBUG_INDENT_INCREASE();

	lua_checkstack(L, 2);

	// Load the import cache table
	lua_getfield(L, LUA_REGISTRYINDEX, IMPORT_CACHE_NAME);
	int importCacheIdx = lua_gettop(L);

	// If we have a path cache entry, this module has been previously loaded
	std::string cached_path = get_path_cache(L, moduleName);
	if (!cached_path.empty()) {
		// return cached module object or (re)load file from disk
		if (load_file_cached(L, cached_path, importCacheIdx)) {
			lua_remove(L, importCacheIdx);

			DEBUG_INDENT_DECREASE();
			LUA_DEBUG_END(L, 1);
			return true;
		}
	}

	// cache the paths we tried (for error messages)
	std::vector<std::string> triedPaths = {};

	// TODO: maybe allow more module search paths? package.path maybe?
	std::vector<std::string> searchPaths = {
		"/",
		"/libs/"
	};

	// convert module.name into module/name
	std::string pathname = moduleName;
	std::replace(pathname.begin(), pathname.end(), '.', '/');

	// look up which file this module maps to
	for (auto &prefix : searchPaths) {
		// check the /file/path.lua form
		std::string path = FileSystem::NormalisePath(prefix + pathname + ".lua");
		triedPaths.push_back(path);

		if (load_file_cached(L, path, importCacheIdx)) {
			lua_remove(L, importCacheIdx);
			set_path_cache(L, moduleName, path);

			DEBUG_INDENT_DECREASE();
			LUA_DEBUG_END(L, 1);
			return true;
		}

		// check the /file/path/init.lua form
		path = FileSystem::NormalisePath(prefix + pathname + "/init.lua");
		triedPaths.push_back(path);
		if (load_file_cached(L, path, importCacheIdx)) {
			lua_remove(L, importCacheIdx);
			set_path_cache(L, moduleName, path);

			DEBUG_INDENT_DECREASE();
			LUA_DEBUG_END(L, 1);
			return true;
		}
	}
	// pop the import cache table
	lua_pop(L, 1);

	// Load the module from the Core c++ module cache
	if (load_from_core(L, moduleName.c_str())) {
		DEBUG_INDENTED_PRINTF("-> loaded module %s from C++ core modules\n", moduleName.c_str());
		DEBUG_INDENT_DECREASE();
		LUA_DEBUG_END(L, 1);
		return true;
	}

	// Generate an error message to pass to the caller.
	std::string errorMsg = "could not load module '" + moduleName + "'";
	errorMsg += "\n\tno entry Imports[\"" + moduleName + "\"]";
	errorMsg += "\n\tno field package.core." + moduleName;
	for (auto path : triedPaths) {
		errorMsg += "\n\tno file '" + path + "'";
	}
	lua_pushstring(L, errorMsg.c_str());

	DEBUG_INDENTED_PRINTF("-> %s\n", errorMsg.c_str());
	DEBUG_INDENT_DECREASE();
	LUA_DEBUG_END(L, 1);
	return false;
}

bool pi_lua_import(lua_State *L, const std::string &importName, bool popImported)
{
	if (!lua_import_module(L, importName)) {
#ifndef DEBUG_IMPORT // already have extended info
		Output("%s\n", lua_tostring(L, -1));
#endif
		lua_pop(L, 1);
		return false;
	}

	if (popImported)
		lua_pop(L, 1);

	return true;
}

void pi_lua_import_recursive(lua_State *L, const std::string &moduleName)
{
	LUA_DEBUG_START(L);
	DEBUG_INDENTED_PRINTF("import recursive [%s]: started\n", moduleName.c_str());
	DEBUG_INDENT_INCREASE();

	std::string modulePath = moduleName;
	std::replace(modulePath.begin(), modulePath.end(), '.', '/');
	std::deque<std::pair<std::string, std::string>> queue = {
		{ modulePath, moduleName }
	};

	while (!queue.empty()) {
		auto &current = queue.front();

		for (FileSystem::FileEnumerator files(FileSystem::gameDataFiles, current.first, FileSystem::FileEnumerator::IncludeDirs); !files.Finished(); files.Next()) {
			const FileSystem::FileInfo &info = files.Current();
			const std::string &fpath = info.GetPath();
			const std::string &fname = info.GetName();
			const size_t extpos = fname.find('.');
			const std::string moduleName = current.second + "." + fname.substr(0, extpos);

			if (info.IsDir() && extpos == std::string::npos) {
				queue.push_back({ fpath, moduleName });
			} else if (info.IsFile() && ends_with_ci(fpath, ".lua")) {
				pi_lua_import(L, moduleName, true);
			}
			LUA_DEBUG_CHECK(L, 0);
		}

		queue.pop_front();
	}

	DEBUG_INDENT_DECREASE();
	DEBUG_INDENTED_PRINTF("import recursive [%s]: ended\n\n", moduleName.c_str());
	LUA_DEBUG_END(L, 0);
}

// Handle caller-based module name aliasing.
// Takes a '.module.name' and returns 'caller.directory.module.name'
static std::string make_module_name(lua_State *L, int idx)
{
	// Names starting with '.' inherit the module name of their source directory
	std::string name = luaL_checkstring(L, idx);
	if (name[0] == '.') {
		std::string caller = get_caller(L);
		name = path_to_module(caller) + name;
	}

	// if it's still '.', then we don't have a source directory.
	if (name[0] == '.') {
		name.clear();
		luaL_error(L, "A fully-qualified module name cannot start with a '.' character. (%s)", name.c_str());
	}

	return name;
}

static std::string get_caller_module_name(lua_State *L, int depth = 1)
{
	std::string caller = get_caller(L, depth);
	std::string_view sv(caller);
	if (ends_with(sv, ".lua"))
		sv.remove_suffix(4);

	return path_to_module(std::string(sv) + "/");
}

static int l_reimport_package(lua_State *L)
{
	std::string name = lua_gettop(L) ? make_module_name(L, 1) : get_caller_module_name(L);

	// Get the module name -> file path mapping
	std::string path = get_path_cache(L, name);

	// clear the file path cache for this file
	lua_getfield(L, LUA_REGISTRYINDEX, IMPORT_CACHE_NAME);
	if (!path.empty()) {
		lua_pushnil(L);
		lua_setfield(L, -2, path.c_str());
	}
	lua_pop(L, 1);

	if (lua_import_module(L, name))
		return 1;
	else {
		lua_pushnil(L);
		lua_insert(L, lua_gettop(L) - 1);
		return 2;
	}
}

static int l_get_module_name(lua_State *L)
{
	int depth = luaL_optinteger(L, 1, 1);
	lua_pushstring(L, get_caller_module_name(L, depth > 1 ? depth : 1).c_str());
	return 1;
}

static int l_require_package(lua_State *L)
{
	std::string module_name = make_module_name(L, 1);
	if (lua_import_module(L, module_name))
		return 1;
	return lua_error(L);
}

static int l_deprecated_import(lua_State *L)
{
	std::string importPath = luaL_checkstring(L, 1);
	std::string moduleName = importPath;

	// trim ".lua" from the end
	size_t last_dot = importPath.find_last_of('.');
	if (last_dot != std::string::npos) {
		if (!importPath.compare(last_dot, 4, ".lua"))
			moduleName = importPath.substr(0, last_dot);
	}

	// convert path to module name
	std::replace(moduleName.begin(), moduleName.end(), '/', '.');

	if (moduleName != importPath)
		Log::Warning("the use of import(\"{}\") is deprecated; use require '{}' instead.\n\t({})\n", importPath.c_str(), moduleName.c_str(), get_caller(L).c_str());
	else
		Log::Warning("the use of import() is deprecated; use require '{}' instead.\n\t({})\n", moduleName.c_str(), get_caller(L).c_str());

	if (moduleName[0] == '.')
		moduleName = path_to_module(get_caller(L)) + moduleName;

	if (lua_import_module(L, moduleName))
		return 1;

	return lua_error(L);
}

int luaopen_import(lua_State *L)
{
	LUA_DEBUG_START(L);
	// package = {}
	lua_newtable(L);
	int package_table = lua_gettop(L);

	// import cache table
	lua_newtable(L);
	lua_setfield(L, LUA_REGISTRYINDEX, IMPORT_CACHE_NAME);

	// module->path mapping cache table
	lua_newtable(L);
	lua_setfield(L, LUA_REGISTRYINDEX, "ImportPaths");

	// TODO: deprecate the global use of `import()`
	lua_pushcfunction(L, l_deprecated_import);
	lua_setglobal(L, "import");

	// global require() function
	lua_pushcfunction(L, l_require_package);
	lua_setglobal(L, "require");

	// core imports cache table
	lua_newtable(L);
	pi_lua_readonly_table_proxy(L, -1);
	lua_setfield(L, package_table, "core");
	lua_setfield(L, LUA_REGISTRYINDEX, "CoreImports");

	lua_pushcfunction(L, l_reimport_package);
	lua_setfield(L, package_table, "reimport");

	lua_pushcfunction(L, l_get_module_name);
	lua_setfield(L, package_table, "thisModule");

	lua_pushcfunction(L, l_get_module_name);
	lua_setfield(L, package_table, "modulename");

	LUA_DEBUG_END(L, 1);
	return 1;
}
