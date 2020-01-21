// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "CoreFwdDecl.h"
#include "FileSystem.h"

#undef DEBUG_IMPORT
#ifdef DEBUG_IMPORT
#define DEBUG_PRINTF Output
#define DEBUG_INDENTED_PRINTF IndentedOutput
#define DEBUG_INDENT_INCREASE IndentIncrease
#define DEBUG_INDENT_DECREASE IndentDecrease
#else
#define DEBUG_PRINTF(...)
#define DEBUG_INDENTED_PRINTF(...)
#define DEBUG_INDENT_INCREASE(...)
#define DEBUG_INDENT_DECREASE(...)
#endif

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

			static char source[1024];
			strncpy(source, &ar.source[start], end - start + 1);

			caller = std::string(source);
		}
	}

	LUA_DEBUG_END(L, 0);
	return caller;
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
#include <algorithm>
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

bool pi_lua_import(lua_State *L, const std::string &importName, bool isFullName)
{
	if (!lua_import(L, importName, isFullName)) {
#ifndef DEBUG_IMPORT // already have extended info
		Output("%s\n", lua_tostring(L, -1));
#endif
		lua_pop(L, 1);
		return false;
	}
	return true;
}

void pi_lua_import_recursive(lua_State *L, const std::string &basepath)
{
	LUA_DEBUG_START(L);
	DEBUG_INDENTED_PRINTF("import recursive [%s]: started\n", basepath.c_str());
	DEBUG_INDENT_INCREASE();

	for (FileSystem::FileEnumerator files(FileSystem::gameDataFiles, basepath, FileSystem::FileEnumerator::IncludeDirs); !files.Finished(); files.Next()) {
		const FileSystem::FileInfo &info = files.Current();
		const std::string &fpath = info.GetPath();
		if (info.IsDir()) {
			pi_lua_import_recursive(L, fpath);
		} else {
			assert(info.IsFile());
			if (ends_with_ci(fpath, ".lua")) {
				if (pi_lua_import(L, fpath, true))
					lua_pop(L, 1); // pop imported value
			}
		}
		LUA_DEBUG_CHECK(L, 0);
	}

	DEBUG_INDENT_DECREASE();
	DEBUG_INDENTED_PRINTF("import recursive [%s]: ended\n\n", basepath.c_str());
	LUA_DEBUG_END(L, 0);
}
