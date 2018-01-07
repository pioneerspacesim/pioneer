// Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaUtils.h"
#include "libs.h"
#include "FileSystem.h"

extern "C" {
#include "jenkins/lookup3.h"
}

/*
 * Interface: util
 *
 * Utility functions available in all Lua contexts.
 */

/*
 * Function: hash_random
 *
 * > r = util.hash_random(seed, m, n)
 *
 * Pick a number deterministically according to some input value.
 *
 * > accent = util.hash_random(person_seed .. 'accent', #accents)
 *
 * Parameters:
 *
 *   seed - A string or a number. The output is deterministic based on this value.
 *   m, n - optional. If called as hash_random(seed), the result is in the range 0 <= x < 1.
 *          If called as hash_random(seed, m, n), the result is an integer in the range m <= x <= n.
 *          m must be less than n. (n - m) must be less than 2^32.
 *
 * Availability:
 *
 *   alpha 24
 *
 * Status:
 *
 *   experimental
 */
static int l_hash_random(lua_State *L)
{
	// This function is intended to:
	//  1- Produce a 64-bit hash of the input value (which is either a string
	//     or a double-precision float).
	//  2- Take those 64 bits and use them to pick a random number
	//     (as indicated in the doc comment above).
	// It's also intended to produce the same output for the same input, across
	// any platforms Pioneer runs on, but there may be bugs.

	int numargs = lua_gettop(L);
	// Note according to hashlittle2 comments, hashA is better mixed than hashB.
	Uint32 hashA = 0, hashB = 0;

	luaL_checkany(L, 1);
	switch (lua_type(L, 1)) {
		case LUA_TSTRING:
		{
			size_t sz;
			const char *str = lua_tolstring(L, 1, &sz);
			// jenkins/lookup3
			lookup3_hashlittle2(str, sz, &hashA, &hashB);
			break;
		}
		case LUA_TNUMBER:
		{
			double n = lua_tonumber(L, 1);
			assert(!is_nan(n));
			// jenkins/lookup3
			// There are assumptions here that 'double' has the same in-memory
			// representation on all platforms we care about. Also since we're
			// taking a number as input, the source of that number (Lua code)
			// needs to compute it in a way that gives the same result on all
			// platforms, which may be tricky in some cases.
			lookup3_hashlittle2(&n, sizeof(n), &hashA, &hashB);
			break;
		}
		default: return luaL_error(L, "expected a string or a number for argument 1");
	}

	if (numargs == 1) {
		// Generate a value in the range 0 <= x < 1.
		// We have 64 random bits (in hashA and hashB). We take 27 bits from
		// hashA and 26 bits from hashB to give a 53-bit integer
		// (0 to 2**53-1 inclusive)
		// (53 bits chosen because IEEE double precision floats can exactly
		// represent integers up to 2**53).
		// 67108864 = 2**26
		double x = (hashA >> 5) * 67108864.0 + double(hashB >> 6);
		// 9007199254740992 = 2**53
		// Divide by 2**53 to get a value from 0 to (less than) 1.
		x *= 1.0 / 9007199254740992.0;
		// return a value x: 0 <= x < 1
		lua_pushnumber(L, x);
		return 1;
	} else if (numargs == 3) {
		Sint64 m = Sint64(lua_tonumber(L, 2));
		Sint64 n = Sint64(lua_tonumber(L, 3));

		if (m > n) { return luaL_error(L, "arguments invalid (m > n not allowed)"); }

		// Restrict to 32-bit output. This is a bit weird because we allow both signed and unsigned.
		if (m < 0) {
			if (m < INT32_MIN || n > INT32_MAX) { return luaL_error(L, "arguments out of range for signed 32-bit int"); }
		} else {
			if (n > UINT32_MAX) { return luaL_error(L, "arguments out of range for unsigned 32-bit int"); }
		}

		Uint64 range = n - m + 1;
		Uint64 bits = (Uint64(hashB) << 32) | Uint64(hashA);
		// return a value x: m <= x <= n
		lua_pushnumber(L, double(Sint64(m) + Sint64(bits % range)));
		return 1;
	} else {
		return luaL_error(L, "wrong number of arguments");
	}
}

/*
 * Function: trim
 *
 * > s = util.trim(str)
 *
 * Trim leading and/or trailing whitespace from a string.
 *
 * Parameters:
 *
 *   str - A string.
 *
 * Availability:
 *
 *   November 2013
 *
 * Status:
 *
 *   experimental
 */
static int l_trim(lua_State *l)
{
	size_t len;
	const char *str = luaL_checklstring(l, 1, &len);

	if (len == 0 || (!isspace(str[0]) && !isspace(str[len-1]))) {
		// empty string, or the string beings & ends with non-whitespace
		// just return the same value
		lua_pushvalue(l, 1);
		return 1;
	} else {
		const char *first = str;
		const char *last = str + (len - 1);
		while (len && isspace(*first)) { ++first; --len; }
		while (len && isspace(*last)) { --last; --len; }
		lua_pushlstring(l, first, len);
		return 1;
	}
}

static const luaL_Reg UTIL_FUNCTIONS[] = {
	{ "trim", l_trim },
	{ "hash_random", l_hash_random },
	{ 0, 0 }
};

static int luaopen_utils(lua_State *L)
{
	luaL_newlib(L, UTIL_FUNCTIONS);
	return 1;
}

static void pi_lua_dofile(lua_State *l, const FileSystem::FileData &code, int nret = 0);

static bool _import_core(lua_State *L, const std::string &importName)
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
struct ImportInfo
{
	const std::string& importName; // original name argument from "import(importName)"
	std::string& fileName; // name of the existing file
	const bool& isFullName; // if true import will no try to load relative to the importDirectories
	std::vector<std::string>& importDirectories; // contans list of paths where file can be imported
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
	if (lua_getstack(L, 1, &ar) && lua_getinfo(L, "S", &ar) && ar.source)
	{
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
static bool get_cached(lua_State *L, const std::string& name)
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
* Finds field in the cache.
* Require a existing cache table in the stack at index -1.
* Returns true if field not nil (exists), puts value to lua stack.
*/
static bool import_from_cache(lua_State *L, const ImportInfo& importInfo)
{
	LUA_DEBUG_START(L);
	assert(lua_istable(L, -1));
	DEBUG_INDENTED_PRINTF("import [%s]: trying to load from cache...", importInfo.importName.c_str());

	bool isImported = false;
	std::string cacheName;

	size_t dirsToCheck = 1; // check by original name
	if (!importInfo.isFullName)
		dirsToCheck += importInfo.importDirectories.size(); // check also relative names

	// check original name, with lua extension and relative by dirs by same rule
	// if isFullName is true, loop pass once and check only original importName
	for (size_t i = 0; i < dirsToCheck; i++)
	{
		if (i == 0) cacheName = importInfo.importName;
		else cacheName = FileSystem::NormalisePath( // normalize for relative paths as "../target"
			FileSystem::JoinPath(importInfo.importDirectories[i - 1], importInfo.importName));

		// check original name
		isImported = get_cached(L, cacheName);

		// check name with extension
		if (!isImported && !importInfo.isFullName)
		{
			cacheName += ".lua";
			isImported = get_cached(L, cacheName);
		}

		if (isImported)
		{
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
static bool load_file(lua_State *L, const std::string& name)
{
	LUA_DEBUG_START(L);

	RefCountedPtr<FileSystem::FileData> fileData = FileSystem::gameDataFiles.ReadFile(name);

	// if file exists
	if (fileData)
	{
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
static bool import_from_file(lua_State *L, ImportInfo& importInfo)
{
	LUA_DEBUG_START(L);
	DEBUG_INDENTED_PRINTF("import [%s]: trying to load a file...", importInfo.importName.c_str());

	bool isImported = false;
	std::string fileName;

	size_t dirsToCheck = 1; // check by original name
	if (!importInfo.isFullName)
		dirsToCheck += importInfo.importDirectories.size(); // check also relative names

	// load file with original name, with lua extension and relative by dirs by same rule
	// if isFullName is true, loop pass once and check only original importName
	for (size_t i = 0; i < dirsToCheck; i++)
	{
		if (i == 0) fileName = importInfo.importName;
		else fileName = FileSystem::NormalisePath( // normalize for relative paths as "../target"
			FileSystem::JoinPath(importInfo.importDirectories[i - 1], importInfo.importName));

		// check original name
		isImported = load_file(L, fileName);

		// check name with extension
		if (!isImported && !importInfo.isFullName)
		{
			fileName += ".lua";
			isImported = load_file(L, fileName);
		}

		if (isImported)
		{
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
static bool import_from_core(lua_State *L, const ImportInfo& importInfo)
{
	LUA_DEBUG_START(L);
	DEBUG_INDENTED_PRINTF("import [%s]: trying core import...", importInfo.importName.c_str());

	bool isImported = _import_core(L, importInfo.importName);

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
static void save_to_cache(lua_State *L, const ImportInfo& importInfo)
{
	LUA_DEBUG_START(L);

	DEBUG_INDENTED_PRINTF("import [%s]: entering into cache...", importInfo.importName.c_str());

	// cache if got not nil
	if (!lua_isnil(L, -1))
	{
		lua_pushvalue(L, -1);
		lua_setfield(L, -3, importInfo.fileName.c_str());
		DEBUG_PRINTF(" saved with name %s\n", importInfo.fileName.c_str());
	}
	else
		DEBUG_PRINTF(" aborted because imported module returned nil\n");

	LUA_DEBUG_END(L, 0); // function does not change the stack
}


/**
* Imports from file or loads from cache lua module.
* Also tries to import from core.
* Returns true if module imported, puts return of module to lua stack.
* Returns false if module not loaded and puts error string to the stack.
* If isFullName disabled tries to check with .lua extension and also
* relative by importDirectories. Else checks only by importName.
*/
static bool _import(lua_State *L, const std::string& importName, bool isFullName = false)
{
	LUA_DEBUG_START(L);

	bool isImported = false;
	std::string fileName;

	const std::string& caller = get_caller(L);
	std::string callerDirectory;

	if (!caller.empty())
		callerDirectory = caller.substr(0, caller.find_last_of('/'));

	std::vector<std::string> importDirectories
	{
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
	if (!isFullName && !callerDirectory.empty())
	{
		bool directoryExists = false;

		// check if caller not exists in import directory
		for (auto it = importInfo.importDirectories.begin(); it != importInfo.importDirectories.end(); ++it)
		{
			if (*it == callerDirectory)
			{
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
	if (isImported)
	{
		// usually happens when file not returns nothing or nil
		if (lua_isnil(L, -1))
		{
			lua_pop(L, 1);
			DEBUG_PRINTF(" got error: %s did not return anything\n", importInfo.fileName.c_str());
			lua_pushfstring(L, "import [%s]: %s did not return anything", importInfo.importName.c_str(), importInfo.fileName.c_str());
			isImported = false;
		}
		else
			DEBUG_PRINTF(" no errors\n");
	}
	else
	{
		// if just not imported
		DEBUG_PRINTF(" got error: not found\n");
		lua_pushfstring(L, "import [%s]: not found", importInfo.importName.c_str());
	}

	DEBUG_INDENTED_PRINTF("import [%s]: import status: %s\n\n", importInfo.importName.c_str(), isImported ? "success" : "fail");
	if (!callerDirectory.empty()) DEBUG_INDENT_DECREASE();
	LUA_DEBUG_END(L, 1);

	return isImported;
}


static int l_d_null_userdata(lua_State *L)
{
	lua_pushlightuserdata(L, nullptr);
	return 1;
}

static int l_base_import(lua_State *L)
{
	if (!_import(L, luaL_checkstring(L, 1)))
		return lua_error(L);
	return 1;
}

bool pi_lua_import(lua_State *L, const std::string &importName, bool isFullName)
{
	if (!_import(L, importName, isFullName)) {
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

	for (FileSystem::FileEnumerator files(FileSystem::gameDataFiles, basepath, FileSystem::FileEnumerator::IncludeDirs); !files.Finished(); files.Next())
	{
		const FileSystem::FileInfo &info = files.Current();
		const std::string &fpath = info.GetPath();
		if (info.IsDir()) {
			pi_lua_import_recursive(L, fpath);
		}
		else {
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

static int l_base_import_core(lua_State *L)
{
	if (!_import_core(L, luaL_checkstring(L, 1)))
		return lua_error(L);
	return 1;
}

static int l_enable_d_mode(lua_State *L)
{
	lua_pushboolean(L, true);
	lua_setfield(L, LUA_REGISTRYINDEX, "debug_enabled");
	return 0;
}

static int l_disable_d_mode(lua_State *L)
{
	lua_pushboolean(L, false);
	lua_setfield(L, LUA_REGISTRYINDEX, "debug_enabled");
	return 0;
}

static int l_d_mode_enabled(lua_State *L)
{
	lua_getfield(L, LUA_REGISTRYINDEX, "debug_enabled");
	return 1;
}

static const luaL_Reg STANDARD_LIBS[] = {
	{ "_G", luaopen_base },
	{ LUA_COLIBNAME, luaopen_coroutine },
	{ LUA_TABLIBNAME, luaopen_table },
	{ LUA_STRLIBNAME, luaopen_string },
	{ LUA_BITLIBNAME, luaopen_bit32 },
	{ LUA_MATHLIBNAME, luaopen_math },
	{ LUA_DBLIBNAME, luaopen_debug },
	{ "util", luaopen_utils },
	{ 0, 0 }
};

// excluded standard libraries:
//  - package library: because we don't want scripts to load Lua code,
//    or (worse) native dynamic libraries from arbitrary places on the system
//    We want to be able to restrict library loading to use our own systems
//    (for safety, and so that the FileSystem abstraction isn't bypassed, and
//    so that installable mods continue to work)
//  - io library: we definitely don't want Lua scripts to be able to read and
//    (worse) write to arbitrary files on the host system
//  - os library: we definitely definitely don't want Lua scripts to be able
//    to run arbitrary shell commands, or rename or remove files on the host
//    system
//  - math.random/math.randomseed: these use the C library RNG, which is not
//    guaranteed to be the same across platforms and is often low quality.
//    Also, global RNGs are almost never a good idea because they make it
//    almost impossible to produce robustly repeatable results
//  - dofile(), loadfile(), load(): same reason as the package library

// extra/custom functionality:
//  - import(): library/dependency loader
//  - import_core(): lowlevel library/dependency loader
//  - math.rad is aliased as math.deg2rad: I prefer the explicit name
//  - util.hash_random(): a repeatable, safe, hash function based source of
//    variation

void pi_lua_open_standard_base(lua_State *L)
{
	for (const luaL_Reg *lib = STANDARD_LIBS; lib->func; ++lib) {
		luaL_requiref(L, lib->name, lib->func, 1);
		lua_pop(L, 1);
	}

	// remove stuff that can load untrusted code
	lua_pushnil(L);
	lua_setglobal(L, "dofile");
	lua_pushnil(L);
	lua_setglobal(L, "loadfile");
	lua_pushnil(L);
	lua_setglobal(L, "load");
	lua_pushnil(L);
	lua_setglobal(L, "loadstring");


	// import table and function
	lua_newtable(L);
	lua_setfield(L, LUA_REGISTRYINDEX, "Imports");
	lua_pushcfunction(L, l_base_import);
	lua_setglobal(L, "import");

	// same for core imports
	lua_newtable(L);
	lua_setfield(L, LUA_REGISTRYINDEX, "CoreImports");
	lua_pushcfunction(L, l_base_import_core);
	lua_setglobal(L, "import_core");

	// standard library adjustments (math library)
	lua_getglobal(L, LUA_MATHLIBNAME);

	// remove math.random and math.randomseed
	lua_pushnil(L);
	lua_setfield(L, -2, "random");
	lua_pushnil(L);
	lua_setfield(L, -2, "randomseed");

	// alias math.deg2rad = math.rad
	lua_getfield(L, -1, "rad");
	assert(lua_isfunction(L, -1));
	lua_setfield(L, -2, "deg2rad");

	// alias math.rad2deg = math.deg
	lua_getfield(L, -1, "deg");
	assert(lua_isfunction(L, -1));
	lua_setfield(L, -2, "rad2deg");

	lua_pop(L, 1); // pop the math table

	// debug library adjustments (adding our own debug mode)
	lua_getglobal(L, LUA_DBLIBNAME);
	lua_pushcfunction(L, l_enable_d_mode);
	lua_pushcclosure(L, secure_trampoline, 1);
	lua_setfield(L, -2, "enabledmode");

	lua_pushcfunction(L, l_disable_d_mode);
	lua_pushcclosure(L, secure_trampoline, 1);
	lua_setfield(L, -2, "disabledmode");

	// That one should NOT be trampolined, since it is intended to be used all over.
	lua_pushcfunction(L, l_d_mode_enabled);
	lua_setfield(L, -2, "dmodeenabled");

	lua_pushcfunction(L, l_d_null_userdata);
	lua_setfield(L, -2, "makenull");

	lua_pop(L, 1); // pop the debug table
}

static int l_readonly_table_newindex(lua_State *l)
{
	return luaL_error(l, "attempting to write to a read-only table");
}

static int l_readonly_table_len(lua_State *l)
{
	lua_getuservalue(l, 1);
	lua_pushunsigned(l, lua_rawlen(l, -1));
	return 1;
}

static int l_readonly_table_pairs(lua_State *l)
{
	lua_getglobal(l, "pairs");
	lua_getuservalue(l, 1);
	lua_call(l, 1, 3);
	return 3;
}

static int l_readonly_table_ipairs(lua_State *l)
{
	lua_getglobal(l, "ipairs");
	lua_getuservalue(l, 1);
	lua_call(l, 1, 3);
	return 3;
}

void pi_lua_readonly_table_proxy(lua_State *l, int table_idx)
{
	table_idx = lua_absindex(l, table_idx);

	LUA_DEBUG_START(l);
	lua_newuserdata(l, 0); // proxy
	lua_pushvalue(l, table_idx);
	lua_setuservalue(l, -2);

	lua_createtable(l, 0, 5); // metatable
	lua_pushliteral(l, "__index");
	lua_pushvalue(l, table_idx);
	lua_rawset(l, -3);
	lua_pushliteral(l, "__len");
	lua_pushcfunction(l, &l_readonly_table_len);
	lua_rawset(l, -3);
	lua_pushliteral(l, "__pairs");
	lua_pushcfunction(l, &l_readonly_table_pairs);
	lua_rawset(l, -3);
	lua_pushliteral(l, "__ipairs");
	lua_pushcfunction(l, &l_readonly_table_ipairs);
	lua_rawset(l, -3);
	lua_pushliteral(l, "__newindex");
	lua_pushcfunction(l, &l_readonly_table_newindex);
	lua_rawset(l, -3);
	lua_pushliteral(l, "__metatable");
	lua_pushboolean(l, false);
	lua_rawset(l, -3);

	lua_setmetatable(l, -2);
	LUA_DEBUG_END(l, 1);
}

void pi_lua_readonly_table_original(lua_State *l, int index)
{
	lua_getuservalue(l, index);
}

static int l_handle_error(lua_State *L)
{
	const char *msg = lua_tostring(L, 1);
	luaL_traceback(L, L, msg, 1);
	return 1;
}

int pi_lua_panic(lua_State *L)
{
	luaL_where(L, 0);
	std::string errorMsg = lua_tostring(L, -1);
	lua_pop(L, 1);

	errorMsg += lua_tostring(L, -1);
	lua_pop(L, 1);

	lua_getglobal(L, "debug");
	lua_getfield(L, -1, "traceback");
	lua_call(L, 0, 1);
	errorMsg += "\n";
	errorMsg += lua_tostring(L, -1);
	errorMsg += "\n";
	Error("%s", errorMsg.c_str());
	// Error() is noreturn

	// XXX when Lua management is good enough, we can probably remove panic
	//     entirely in favour of pcall and a nicer error handling system
	RETURN_ZERO_NONGNU_ONLY;
}

void pi_lua_protected_call(lua_State* L, int nargs, int nresults) {
	int handleridx = lua_gettop(L) - nargs;
	lua_pushcfunction(L, &l_handle_error);
	lua_insert(L, handleridx);
	int ret = lua_pcall(L, nargs, nresults, handleridx);
	lua_remove(L, handleridx); // pop error_handler
	if (ret) {
		std::string errorMsg = lua_tostring(L, -1);
		lua_pop(L, 1);
		Error("%s", errorMsg.c_str());
	}
}

int pi_lua_loadfile(lua_State *l, const FileSystem::FileData &code)
{
	assert(l);

	const StringRange source = code.AsStringRange().StripUTF8BOM();
	const std::string &path(code.GetInfo().GetPath());
	bool trusted = code.GetInfo().GetSource().IsTrusted();
	const std::string chunkName = (trusted ? "[T] @" : "@") + path;

	return luaL_loadbuffer(l, source.begin, source.Size(), chunkName.c_str());
}

static void pi_lua_dofile(lua_State *l, const FileSystem::FileData &code, int nret)
{
	assert(l);
	LUA_DEBUG_START(l);

	if (pi_lua_loadfile(l, code) != LUA_OK) {
		const char *msg = lua_tostring(l, -1);
		Error("%s", msg);
	}

	// XXX make this a proper protected call (after working out the implications -- *sigh*)
	lua_pushcfunction(l, &pi_lua_panic);
	lua_insert(l, -2);
	int panicidx = lua_absindex(l, -2);

	// give the file its own global table, backed by the "real" one
	lua_newtable(l);
	lua_newtable(l);
	lua_pushliteral(l, "__index");
	lua_getglobal(l, "_G");
	lua_rawset(l, -3);
	lua_setmetatable(l, -2);
	lua_setupvalue(l, -2, 1);

	int ret = lua_pcall(l, 0, nret, panicidx);
	if (ret) {
		const char *emsg = lua_tostring(l, -1);
		if (emsg) { Output("lua error: %s\n", emsg); }
		switch (ret) {
			case LUA_ERRRUN:
				Output("Lua runtime error in pi_lua_dofile('%s')\n",
						code.GetInfo().GetAbsolutePath().c_str());
				break;
			case LUA_ERRMEM:
				Output("Memory allocation error in Lua pi_lua_dofile('%s')\n",
						code.GetInfo().GetAbsolutePath().c_str());
				break;
			case LUA_ERRERR:
				Output("Error running error handler in pi_lua_dofile('%s')\n",
						code.GetInfo().GetAbsolutePath().c_str());
				break;
			default: abort();
		}
		lua_pop(l, 1);
	}

	lua_remove(l, panicidx);
	LUA_DEBUG_END(l, nret);
}

void pi_lua_dofile(lua_State *l, const std::string &path)
{
	assert(l);
	LUA_DEBUG_START(l);

	RefCountedPtr<FileSystem::FileData> code = FileSystem::gameDataFiles.ReadFile(path);
	if (!code) {
		Output("could not read Lua file '%s'\n", path.c_str());
		return;
	}

	pi_lua_dofile(l, *code);

	LUA_DEBUG_END(l, 0);
}

void pi_lua_dofile_recursive(lua_State *l, const std::string &basepath)
{
	LUA_DEBUG_START(l);

	for (FileSystem::FileEnumerator files(FileSystem::gameDataFiles, basepath, FileSystem::FileEnumerator::IncludeDirs); !files.Finished(); files.Next())
	{
		const FileSystem::FileInfo &info = files.Current();
		const std::string &fpath = info.GetPath();
		if (info.IsDir()) {
			pi_lua_dofile_recursive(l, fpath);
		} else {
			assert(info.IsFile());
			if (ends_with_ci(fpath, ".lua")) {
				RefCountedPtr<FileSystem::FileData> code = info.Read();
				pi_lua_dofile(l, *code);
			}
		}
		LUA_DEBUG_CHECK(l, 0);
	}

	LUA_DEBUG_END(l, 0);
}

void pi_lua_warn(lua_State *l, const char *format, ...)
{
	char buf[1024];
	va_list ap;
	va_start(ap, format);
	vsnprintf(buf, sizeof(buf), format, ap);
	va_end(ap);
	Output("Lua Warning: %s\n", buf);

	lua_Debug info;
	int level = 0;
	while (lua_getstack(l, level, &info)) {
		lua_getinfo(l, "nSl", &info);
		Output("  [%d] %s:%d -- %s [%s]\n",
			level, info.short_src, info.currentline,
			(info.name ? info.name : "<unknown>"), info.what);
		++level;
	}
}

// drill down from global looking for the appropriate table for the given
// path. returns with the table and the last fragment on the stack, ready for
// get/set a value in the table with that key.
// eg foo.bar.baz results in something like _G.foo = { bar = {} }, with the
// "bar" table left at -2 and "baz" at -1.
// returns true if "bar" table found, false otherwise
bool pi_lua_split_table_path(lua_State *l, const std::string &path)
{
	LUA_DEBUG_START(l);

	const char delim = '.';

	std::string last;

	lua_getfield(l, LUA_REGISTRYINDEX, "CoreImports");

	size_t start = 0, end = 0;
	while (end != std::string::npos) {
		// get to the first non-delim char
		start = path.find_first_not_of(delim, end);

		// read the end, no more to do
		if (start == std::string::npos)
			break;

		// have a fragment from last time, get the next table
		if (!last.empty()) {
			luaL_getsubtable(l, -1, last.c_str());
			if (lua_isnil(l, -1)) {
				LUA_DEBUG_END(l, 0);
				return false;
			}
			assert(lua_istable(l, -1));
			lua_remove(l, -2);
		}

		// find the end - next delim or end of string
		end = path.find_first_of(delim, start);

		// extract the fragment and remember it
		last = path.substr(start, (end == std::string::npos) ? std::string::npos : end - start);
	}

	if (last.empty()) {
		LUA_DEBUG_END(l, 0);
		return false;
	}

	assert(!last.empty());

	lua_pushlstring(l, last.c_str(), last.size());

	LUA_DEBUG_END(l, 2);

	return true;
}

int secure_trampoline(lua_State *l)
{
	// walk the stack
	// pass through any C functions
	// if we reach a non-C function, then check whether it's trusted and we're done
	// (note: trusted defaults to true because if the loop bottoms out then we've only gone through C functions)

	bool trusted = true;

	lua_Debug ar;
	int stack_pos = 1;
	while (lua_getstack(l, stack_pos, &ar) && lua_getinfo(l, "S", &ar)) {
		if (strcmp(ar.what, "C") != 0) {
			trusted = (strncmp(ar.source, "[T]", 3) == 0);
			break;
		}
		++stack_pos;
	}

	if (!trusted)
		luaL_error(l, "attempt to access protected method or attribute from untrusted script blocked");

	lua_CFunction fn = lua_tocfunction(l, lua_upvalueindex(1));
	return fn(l);
}

