// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "CoreFwdDecl.h"
#include "FileSystem.h"
#include <algorithm>
#include <deque>
#include <string>

#undef DEBUG_IMPORT
#ifdef DEBUG_IMPORT
#define DEBUG_PRINTF Output
#define DEBUG_INDENTED_PRINTF IndentedOutput
#define DEBUG_INDENT_INCREASE IndentIncrease
#define DEBUG_INDENT_DECREASE IndentDecrease
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
	const std::string &importName; // original name argument from "import(importName)"
	std::string &fileName; // name of the existing file
	const bool &isFullName; // if true import will no try to load relative to the importDirectories
	std::vector<std::string> &importDirectories; // contans list of paths where file can be imported
};

/**
* Returns path of last element in the lua stack
*/
static std::string get_caller(lua_State *L)
{
	// XXX: Extraneous copy
	assert(L);
	LUA_DEBUG_START(L);

	std::string caller;
	lua_Debug ar;

	// if have stack and caller in the stack
	if (lua_getstack(L, 1, &ar) && lua_getinfo(L, "S", &ar) && ar.source) {
		int start = 0;
		int end = strlen(ar.source);

		if ((ar.source[start] != '@') && (end > 5))
			start = 4;

		if (ar.source[start] == '@') {
			start++;

			return std::string(&ar.source[start], end - start + 1);
		}
	}

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
	// simple allocation optimizations
	if (path.size() > 8 && path.find_first_of('/') != std::string::npos) {
		module_name.reserve(8);
	}

	// Loop through each path fragment and append to the module name
	// e.g. '/data/modules/path/to/file.lua' would become 'data.modules.path.to'
	size_t start = 0;
	size_t end = path.find_first_of('/', 1); // use start=1 to generate string::npos when the path == "/"

	while (end != std::string::npos) {
		auto fragment = path.substr(start, end - start);

		if (fragment.size() > 0) {
			// Paths containing the separator character cannot be translated into a module name
			// this means you can't do implicit lookup using a path starting in a hidden directory, e.g.
			// /data/.local/mylib.lua
			if (fragment.find('.') != std::string::npos)
				return "";
			module_name += module_name.empty() ? fragment : '.' + fragment;
		}

		start = end + 1;
		end = path.find_first_of('/', start);
	}

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
	if (fileInfo.Exists()) {
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

	LUA_DEBUG_END(L, 0);
	return false;
}

// Check the path cache for a module name to filesystem path mapping
// If present, this module has been successfully loaded before.
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
	if (!lua_isnil(L, -1)) {
		LUA_DEBUG_END(L, 1);
		return true;
	}

	lua_pop(L, 1);
	LUA_DEBUG_END(L, 0);
	return false;
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

/**
* Finds field in table.
* Requires a existing table in the stack at index -1.
* Returns true if field exists and not nil, puts value to lua stack.
*/
static bool get_cached(lua_State *L, const std::string &name)
{
	LUA_DEBUG_START(L);
	assert(lua_istable(L, -1));

	// try to get it
	lua_getfield(L, -1, name.c_str());

	// if got cached field
	bool isCached = !lua_isnil(L, -1);

	if (!isCached)
		lua_pop(L, 1); // remove found "nil" from stack

	LUA_DEBUG_END(L, (isCached ? 1 : 0));
	return isCached;
}

/**
	Convert a module path in the form `module.submodule.abc` into a file path
	in the form `module/submodule/abc`.
*/
static std::string table_path_to_file_name(const std::string &path)
{
	std::string out = path;

	// If we have an old-style 'module/path/file.lua' path, return it normalized.
	// Otherwise, transform 'module.path.file' -> 'module/path/file'.
	if (path.find('/') == std::string::npos) {
		std::replace(out.begin(), out.end(), '.', '/');
	}
	return FileSystem::NormalisePath(out);
}

/**
* Finds field in the cache.
* Require a existing cache table in the stack at index -1.
* Returns true if field not nil (exists), puts value to lua stack.
*/
static bool import_from_cache(lua_State *L, const ImportInfo &importInfo)
{
	LUA_DEBUG_START(L);
	assert(lua_istable(L, -1));
	DEBUG_INDENTED_PRINTF("import [%s]: trying to load from cache...", importInfo.importName.c_str());

	bool isImported = false;
	std::string realName = importInfo.importName;

	size_t dirsToCheck = 1; // check by original name
	if (!importInfo.isFullName) {
		dirsToCheck += importInfo.importDirectories.size(); // check also relative names
		realName = table_path_to_file_name(realName);
	}

	// check original name, with lua extension and relative by dirs by same rule
	// if isFullName is true, loop pass once and check only original importName
	for (size_t i = 0; i < dirsToCheck; i++) {
		std::string cacheName = realName;
		if (i > 0) cacheName = FileSystem::NormalisePath(
					   // normalize for relative paths as "../target"
					   FileSystem::JoinPath(importInfo.importDirectories[i - 1], realName));

		// check original name
		isImported = get_cached(L, cacheName);

		// check name with extension
		if (!isImported && !importInfo.isFullName) {
			isImported = get_cached(L, cacheName + ".lua");
			if (isImported) cacheName += ".lua";
		}

		if (!isImported && !importInfo.isFullName) {
			cacheName = FileSystem::JoinPath(cacheName, "init.lua");
			isImported = get_cached(L, cacheName);
		}

		if (isImported) {
			DEBUG_PRINTF(" found cached %s\n", cacheName.c_str());
			break;
		}
	}

	if (!isImported)
		DEBUG_PRINTF(" not cached\n");

	LUA_DEBUG_END(L, (isImported ? 1 : 0));
	return isImported;
}

/**
* Tries to load file.
* Returns true if file exists and puts return of dofile to stack.
*/
static bool load_file(lua_State *L, const std::string &name)
{
	LUA_DEBUG_START(L);

	RefCountedPtr<FileSystem::FileData> fileData = FileSystem::gameDataFiles.ReadFile(name);

	// if file exists
	if (fileData) {
		DEBUG_PRINTF(" loading %s...\n", name.c_str());
		pi_lua_dofile(L, *fileData, 1);
	}

	LUA_DEBUG_END(L, (fileData ? 1 : 0));
	return static_cast<bool>(fileData);
}

/**
* Tries to load file.
* If file found returns true, puts return of dofile to stack, no matter nil it or value.
* If import was successful, changes fileName in the importInfo to the filename that was found.
*/
static bool import_from_file(lua_State *L, ImportInfo &importInfo)
{
	LUA_DEBUG_START(L);
	DEBUG_INDENTED_PRINTF("import [%s]: trying to load a file...", importInfo.importName.c_str());

	bool isImported = false;
	std::string realName = importInfo.importName;

	size_t dirsToCheck = 1; // check by original name
	if (!importInfo.isFullName) {
		dirsToCheck += importInfo.importDirectories.size(); // check also relative names
		realName = table_path_to_file_name(realName);
	}

	// load file with original name, with lua extension and relative by dirs by same rule
	// if isFullName is true, loop pass once and check only original importName
	for (size_t i = 0; i < dirsToCheck; i++) {
		std::string fileName = realName;
		if (i > 0) fileName = FileSystem::NormalisePath( // normalize for relative paths as "../target"
					   FileSystem::JoinPath(importInfo.importDirectories[i - 1], realName));

		// check original name
		isImported = load_file(L, fileName);

		// check name with extension
		if (!isImported && !importInfo.isFullName) {
			isImported = load_file(L, fileName + ".lua");
			if (isImported) fileName += ".lua";
		}

		if (!isImported && !importInfo.isFullName) {
			fileName = FileSystem::JoinPath(fileName, "init.lua");
			isImported = load_file(L, fileName);
		}

		if (isImported) {
			DEBUG_INDENTED_PRINTF("import [%s]: file %s loaded\n", importInfo.importName.c_str(), fileName.c_str());
			importInfo.fileName = fileName;
			break;
		}
	}

	if (!isImported)
		DEBUG_PRINTF(" file not found\n");

	LUA_DEBUG_END(L, (isImported ? 1 : 0));
	return isImported;
}

/**
* Tries to import from core.
* Returns true if imported, puts value to the stack.
*/
bool import_from_core(lua_State *L, const ImportInfo &importInfo)
{
	LUA_DEBUG_START(L);
	DEBUG_INDENTED_PRINTF("import [%s]: trying core import...", importInfo.importName.c_str());

	bool isImported = lua_import_core(L, importInfo.importName);

	// remove import_core error
	if (!isImported)
		lua_pop(L, 1);

	DEBUG_PRINTF(" %s\n", (isImported ? "imported" : "not imported"));
	LUA_DEBUG_END(L, (isImported ? 1 : 0));
	return isImported;
}

/**
* Saves value with index -1 to table with index -2
*/
static void save_to_cache(lua_State *L, const ImportInfo &importInfo)
{
	LUA_DEBUG_START(L);

	DEBUG_INDENTED_PRINTF("import [%s]: entering into cache...", importInfo.importName.c_str());

	// cache if got not nil
	if (!lua_isnil(L, -1)) {
		lua_pushvalue(L, -1);
		lua_setfield(L, -3, importInfo.fileName.c_str());
		DEBUG_PRINTF(" saved with name %s\n", importInfo.fileName.c_str());
	} else
		DEBUG_PRINTF(" aborted because imported module returned nil\n");

	LUA_DEBUG_END(L, 0); // function does not change the stack
}

bool lua_import_core(lua_State *L, const std::string &importName)
{
	LUA_DEBUG_START(L);

	if (!pi_lua_split_table_path(L, importName)) {
		lua_pushfstring(L, "import_core: %s: not found", importName.c_str());
		LUA_DEBUG_END(L, 1);
		return false;
	}

	lua_rawget(L, -2);
	if (!lua_isnil(L, -1)) {
		lua_remove(L, -2);
		LUA_DEBUG_END(L, 1);
		return true;
	}
	lua_pop(L, 2);

	lua_pushfstring(L, "import_core: %s: not found", importName.c_str());
	LUA_DEBUG_END(L, 1);
	return false;
}

/**
* Imports from file or loads from cache lua module.
* Also tries to import from core.
* Returns true if module imported, puts return of module to lua stack.
* Returns false if module not loaded and puts error string to the stack.
* If isFullName disabled tries to check with .lua extension and also
* relative by importDirectories. Else checks only by importName.
*/
bool lua_import(lua_State *L, const std::string &importName, bool isFullName)
{
	LUA_DEBUG_START(L);

	bool isImported = false;
	std::string fileName;

	const std::string &caller = get_caller(L);
	std::string callerDirectory;

	if (!caller.empty())
		callerDirectory = caller.substr(0, caller.find_last_of('/'));

	std::vector<std::string> importDirectories{
		"libs"
	};

	// main data for passing to functions
	ImportInfo importInfo = {
		importName,
		fileName,
		isFullName,
		importDirectories
	};

	if (!callerDirectory.empty()) DEBUG_INDENT_INCREASE();
	DEBUG_INDENTED_PRINTF("import [%s]: import started, caller: %s\n",
		importInfo.importName.c_str(), caller.empty() ? "core" : caller.c_str());

	// add caller directory to import directories if exists
	if (!isFullName && !callerDirectory.empty()) {
		bool directoryExists = false;

		// check if caller not exists in import directory
		for (auto it = importInfo.importDirectories.begin(); it != importInfo.importDirectories.end(); ++it) {
			if (*it == callerDirectory) {
				directoryExists = true;
				break;
			}
		}

		// if not exists already
		if (!directoryExists)
			importInfo.importDirectories.push_back(callerDirectory);
	}

	// load cache table
	lua_getfield(L, LUA_REGISTRYINDEX, "Imports");
	assert(lua_istable(L, -1));

	// import and save to cache
	{
		isImported = import_from_cache(L, importInfo);

		if (!isImported)
			// if imported, this also changes fileName in importInfo
			isImported = import_from_file(L, importInfo);

		if (!isImported)
			isImported = import_from_core(L, importInfo);

		// save to cache only when imported from file
		if (isImported && !importInfo.fileName.empty())
			save_to_cache(L, importInfo);
	}

	// cache table and optional value
	LUA_DEBUG_CHECK(L, (isImported ? 2 : 1));

	// remove Imports table from stack
	lua_remove(L, isImported ? -2 : -1);

	// generate error messages
	DEBUG_INDENTED_PRINTF("import [%s]: generating error messages...", importInfo.importName.c_str());
	if (isImported) {
		// usually happens when file not returns nothing or nil
		if (lua_isnil(L, -1)) {
			lua_pop(L, 1);
			DEBUG_PRINTF(" got error: %s did not return anything\n", importInfo.fileName.c_str());
			lua_pushfstring(L, "import [%s]: %s did not return anything", importInfo.importName.c_str(), importInfo.fileName.c_str());
			isImported = false;
		} else
			DEBUG_PRINTF(" no errors\n");
	} else {
		// if just not imported
		DEBUG_PRINTF(" got error: not found\n");
		lua_pushfstring(L, "import [%s]: not found", importInfo.importName.c_str());
	}

	DEBUG_INDENTED_PRINTF("import [%s]: import status: %s\n\n", importInfo.importName.c_str(), isImported ? "success" : "fail");
	if (!callerDirectory.empty()) DEBUG_INDENT_DECREASE();
	LUA_DEBUG_END(L, 1);

	return isImported;
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
		auto caller = get_caller(L);
		name = path_to_module(caller) + name;
	}

	// if it's still '.', then we don't have a source directory.
	if (name[0] == '.') {
		name.clear();
		luaL_error(L, "A fully-qualified module name cannot start with a '.' character. (%s)", name.c_str());
	}

	return name;
}

static int l_reimport_package(lua_State *L)
{
	std::string name = make_module_name(L, 1);

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
	return lua_error(L);
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
		Output("warning: the use of import(\"%s\") is deprecated; use require '%s' instead.\n\t(%s)\n", importPath.c_str(), moduleName.c_str(), get_caller(L).c_str());
	else
		Output("warning: the use of import() is deprecated; use require '%s' instead.\n\t(%s)\n", moduleName.c_str(), get_caller(L).c_str());

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

	LUA_DEBUG_END(L, 1);
	return 1;
}
