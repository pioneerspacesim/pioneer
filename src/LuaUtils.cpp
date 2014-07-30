// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaUtils.h"
#include "libs.h"
#include "FileSystem.h"

extern "C" {
#ifdef ENABLE_LDB
#include <ldbcore.h>
#endif //ENABLE_LDB
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
 *          If called as hash_random(seed, n), the result is an integer in the range 1 <= x <= n.
 *          If called as hash_random(seed, m, n), the result is an integer in the range m <= x <= n.
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
	int numargs = lua_gettop(L);
	Uint32 hashA = 0, hashB = 0;

	luaL_checkany(L, 1);
	switch (lua_type(L, 1)) {
		case LUA_TNIL:
			// random numbers!
			hashA = 0xBF42B131u;
			hashB = 0x2A40F7F2u;
			break;
		case LUA_TSTRING:
		{
			size_t sz;
			const char *str = lua_tolstring(L, 1, &sz);
			lookup3_hashlittle2(str, sz, &hashA, &hashB);
			break;
		}
		case LUA_TNUMBER:
		{
			lua_Number n = lua_tonumber(L, 1);
			assert(!is_nan(n));
			lookup3_hashlittle2(&n, sizeof(n), &hashA, &hashB);
			break;
		}
		default: return luaL_error(L, "expected a string or a number for argument 1");
	}

	// generate a value in the range 0 <= x < 1
	double x = (hashA >> 5) * 67108864.0 + double(hashB >> 6);
	x *= 1.0 / 9007199254740992.0;
	if (numargs == 1) {
		// return a value x: 0 <= x < 1
		lua_pushnumber(L, x);
		return 1;
	} else {
		int m, n;
		if (numargs == 3) {
			m = lua_tointeger(L, 2);
			n = lua_tointeger(L, 3);
		} else if (numargs == 2) {
			m = 1;
			n = lua_tointeger(L, 2);
		} else {
			assert(numargs > 3);
			return luaL_error(L, "wrong number of arguments");
		}
		// return a value x: m <= x <= n
		lua_pushinteger(L, m + int(x * (n - m + 1)));
		return 1;
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
#else
#define DEBUG_PRINTF(...)
#endif

static bool _import(lua_State *L, const std::string &importName)
{
	LUA_DEBUG_START(L);

	DEBUG_PRINTF("import: request for %s\n", importName.c_str());

	// get the cache of previously imported things
	lua_getfield(L, LUA_REGISTRYINDEX, "Imports");
	assert(lua_istable(L, -1));

	// first check the cache for the explicit name (its there if it was
	// previously imported from core)
	std::string cacheName = importName;
	lua_getfield(L, -1, cacheName.c_str());
	if (!lua_isnil(L, -1)) {
		// found it, use it
		DEBUG_PRINTF("import: found cached %s\n", cacheName.c_str());
		lua_remove(L, -2);
		return 1;
	}
	lua_pop(L, 1);
	DEBUG_PRINTF("import: not cached %s\n", cacheName.c_str());

	// next we're going to look in libs/
	cacheName = FileSystem::JoinPath("libs", importName+".lua");
	lua_getfield(L, -1, cacheName.c_str());
	if (!lua_isnil(L, -1)) {
		// found it, use it
		DEBUG_PRINTF("import: found cached %s\n", cacheName.c_str());
		lua_remove(L, -2);
		return 1;
	}
	lua_pop(L, 1);
	DEBUG_PRINTF("import: not cached %s\n", cacheName.c_str());

	RefCountedPtr<FileSystem::FileData> code = FileSystem::gameDataFiles.ReadFile(cacheName);

	// if its not found then we try to find the lib relative to the calling file
	if (!code) {
		DEBUG_PRINTF("import: not found %s, trying relative\n", cacheName.c_str());

		lua_Debug ar;
		lua_getstack(L, 1, &ar);
		lua_getinfo(L, "S", &ar);

		if (ar.source) {
			int start = 0;
			int end = strlen(ar.source);

			if (ar.source[start] != '@' && end > 5)
				start = 4;

			if (ar.source[start] == '@') {
				start++;

				static char source[1024];
				strncpy(source, &ar.source[start], end-start+1);
				DEBUG_PRINTF("import: called from %s\n", source);

				for (end = end-start-1; end >= 0; end--)
					if (source[end] == '/') {
						source[end] = '\0';
						break;
					}

				DEBUG_PRINTF("import: relative path %s\n", source);

				cacheName = FileSystem::JoinPath(source, importName+".lua");
				lua_getfield(L, -1, cacheName.c_str());
				if (!lua_isnil(L, -1)) {
					// found it, use it
					DEBUG_PRINTF("import: found cached %s\n", cacheName.c_str());
					lua_remove(L, -2);
					return 1;
				}
				lua_pop(L, 1);
				DEBUG_PRINTF("import: not cached %s\n", cacheName.c_str());

				code = FileSystem::gameDataFiles.ReadFile(cacheName);
			}
		}
	}

	if (code) {
		DEBUG_PRINTF("import: found %s, running\n", cacheName.c_str());
		pi_lua_dofile(L, *code, 1);
	}

	else {
		// file not foudn, try for a core import
		DEBUG_PRINTF("import: not found %s, trying core\n", cacheName.c_str());
		if (!_import_core(L, importName)) {
			lua_pop(L, 1); // remove import_core error
			lua_pushfstring(L, "import: %s: not found", importName.c_str());
			return false;
		}
		cacheName = importName;
	}

	if (lua_isnil(L, -1)) {
		lua_pushfstring(L, "import: %s: did not return anything", cacheName.c_str());
		return false;
	}

	DEBUG_PRINTF("import: entering %s into cache\n", cacheName.c_str());

	lua_pushvalue(L, -1);
	lua_setfield(L, -3, cacheName.c_str());

	lua_remove(L, -2);

	LUA_DEBUG_END(L, 1);

	return true;
}

static int l_base_import(lua_State *L)
{
	if (!_import(L, luaL_checkstring(L, 1)))
		return lua_error(L);
	return 1;
}

bool pi_lua_import(lua_State *L, const std::string &importName)
{
	if (!_import(L, importName)) {
		Output("%s\n", lua_tostring(L, -1));
		lua_pop(L, 1);
		return false;
	}
	return true;
}

static int l_base_import_core(lua_State *L)
{
	if (!_import_core(L, luaL_checkstring(L, 1)))
		return lua_error(L);
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
#ifdef ENABLE_LDB
	{ LUA_LDBCORELIBNAME, luaopen_ldbcore},
#endif //ENABLE_LDB
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
