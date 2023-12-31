// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "CoreFwdDecl.h"
#include "FileSystem.h"
#include "LuaUtils.h"
#include "core/Log.h"
#include "utils.h"
#include "FileSystem.h"
#include "LuaFileSystem.h"

static int l_d_null_userdata(lua_State *L)
{
	lua_pushlightuserdata(L, nullptr);
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

static int l_d_traceback(lua_State *L)
{
	const char *str = luaL_optstring(L, 1, "");
	int level = luaL_optinteger(L, 2, 1);

	if (str[0] != '\0') {
		lua_pushstring(L, fmt::format("{}\n{}", str, pi_lua_traceback(L, level)).c_str());
	} else {
		lua_pushstring(L, pi_lua_traceback(L, level).c_str());
	}

	return 1;
}

static int l_d_dumpstack(lua_State *L)
{
	int level = luaL_optinteger(L, 1, 1);

	lua_pushstring(L, pi_lua_dumpstack(L, level).c_str());
	return 1;
}

// Copy of luaB_print tailored to use Pioneer logging facilities
static int l_print(lua_State *L)
{
	std::string accum = "";
	int n = lua_gettop(L);  /* number of arguments */
	int i;

	lua_getglobal(L, "tostring");
	for (i=1; i<=n; i++) {
		const char *s;
		size_t l;
		lua_pushvalue(L, -1);  /* function to be called */
		lua_pushvalue(L, i);   /* value to print */
		lua_call(L, 1, 1);
		s = lua_tolstring(L, -1, &l);  /* get result */

		if (s == NULL)
			return luaL_error(L, LUA_QL("tostring") " must return a string to " LUA_QL("print"));

		accum += s;
		if (n > i)
			accum.append(1, '\t');

		lua_pop(L, 1);  /* pop result */
	}

	accum.append("\n");
	Log::GetLog()->LogLevel(Log::Severity::Info, accum);
	return 0;
}

static int l_log_verbose(lua_State *L)
{
	const char *str = lua_tostring(L, 1);
	if (lua_gettop(L) > 0 && str)
		Log::GetLog()->LogLevel(Log::Severity::Verbose, str);

	return 0;
}

static int l_log_warning(lua_State *L)
{
	const char *str = lua_tostring(L, 1);
	if (lua_gettop(L) > 0 && str)
		Log::GetLog()->LogLevel(Log::Severity::Warning, str);

	return 0;
}

static const luaL_Reg STANDARD_LIBS[] = {
	{ "_G", luaopen_base },
	{ LUA_COLIBNAME, luaopen_coroutine },
	{ LUA_TABLIBNAME, luaopen_table },
	{ LUA_STRLIBNAME, luaopen_string },
	{ LUA_BITLIBNAME, luaopen_bit32 },
	{ LUA_MATHLIBNAME, luaopen_math },
	{ LUA_DBLIBNAME, luaopen_debug },
	{ LUA_IOLIBNAME, luaopen_io },
	{ "util", luaopen_utils },
	{ "package", luaopen_import },
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

	lua_pushcfunction(L, l_print);
	lua_setglobal(L, "print");
	lua_pushcfunction(L, l_log_warning);
	lua_setglobal(L, "logWarning");
	lua_pushcfunction(L, l_log_verbose);
	lua_setglobal(L, "logVerbose");

	// IO library adjustments
	lua_getglobal(L, LUA_IOLIBNAME);

	lua_getfield(L, -1, "open");
	assert(lua_iscfunction(L, -1));
	LuaFileSystem::register_raw_io_open_function(lua_tocfunction(L, -1));

	lua_pop(L, 1); // pop the io table
	lua_getglobal(L, LUA_IOLIBNAME);

	// patch io.open so we can check the path
	lua_pushcfunction(L, LuaFileSystem::l_patched_io_open);
	lua_setfield(L, -2, "open");

	// remove io.popen as we don't want people running apps
	lua_pushnil(L);
	lua_setfield(L, -2, "popen");
	lua_pushnil(L);
	// remove other fields that we don't allow as we only want
	// specific file IO and no console IO (as it makes little sense)
	lua_setfield(L, -2, "input");
	lua_pushnil(L);
	lua_setfield(L, -2, "output");
	lua_pushnil(L);
	lua_setfield(L, -2, "tmpfile");
	lua_pushnil(L);
	lua_setfield(L, -2, "stdout");
	lua_pushnil(L);
	lua_setfield(L, -2, "stderr");

	lua_pop(L, 1); // pop the io table

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

	lua_pushcfunction(L, l_d_traceback);
	lua_setfield(L, -2, "traceback");

	lua_pushcfunction(L, l_d_dumpstack);
	lua_setfield(L, -2, "dumpstack");

	lua_pop(L, 1); // pop the debug table
}

static int l_handle_error(lua_State *L)
{
	const char *msg = lua_tostring(L, 1);

	Log::Debug("{}\n", msg);
	Log::Debug("{}", pi_lua_dumpstack(L, 1));

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

	Log::Debug("{}\n", errorMsg);
	Log::Debug("{}", pi_lua_dumpstack(L, 0));

	errorMsg += "\n" + pi_lua_traceback(L, 0) + "\n";

	Error("%s", errorMsg.c_str());
	// Error() is noreturn

	// XXX when Lua management is good enough, we can probably remove panic
	//     entirely in favour of pcall and a nicer error handling system
	RETURN_ZERO_NONGNU_ONLY;
}

void pi_lua_protected_call(lua_State *L, int nargs, int nresults)
{
	int handleridx = lua_gettop(L) - nargs;
	lua_pushcfunction(L, &l_handle_error);
	lua_insert(L, handleridx);
	int ret = lua_pcall(L, nargs, nresults, handleridx);
	lua_remove(L, handleridx); // pop error_handler
	if (ret) {
		std::string errorMsg = lua_tostring(L, -1);
		lua_pop(L, 1);
		Error("%s\n", errorMsg.c_str());
	}
}

int pi_lua_loadfile(lua_State *l, const FileSystem::FileData &code)
{
	assert(l);

	const StringRange source = code.AsStringRange().StripUTF8BOM();
	const std::string &path(code.GetInfo().GetPath());
	bool trusted = code.GetInfo().GetSource().IsTrusted();
	const std::string chunkName = (trusted ? "@[T] " : "@") + path;

	return luaL_loadbuffer(l, source.begin, source.Size(), chunkName.c_str());
}

void pi_lua_dofile(lua_State *l, const FileSystem::FileData &code, int nret)
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
		if (emsg) {
			Output("lua error: %s\n", emsg);
		}
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

void pi_lua_dofile(lua_State *l, const std::string &path, int nret)
{
	assert(l);
	LUA_DEBUG_START(l);

	RefCountedPtr<FileSystem::FileData> code = FileSystem::gameDataFiles.ReadFile(path);
	if (!code) {
		Output("could not read Lua file '%s'\n", path.c_str());
		return;
	}

	pi_lua_dofile(l, *code, nret);

	LUA_DEBUG_END(l, 0);
}

void pi_lua_dofile_recursive(lua_State *l, const std::string &basepath)
{
	LUA_DEBUG_START(l);

	for (FileSystem::FileEnumerator files(FileSystem::gameDataFiles, basepath, FileSystem::FileEnumerator::IncludeDirs); !files.Finished(); files.Next()) {
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
			trusted = (strncmp(ar.source, "@[T]", 4) == 0);
			break;
		}
		++stack_pos;
	}

	if (!trusted)
		luaL_error(l, "attempt to access protected method or attribute from untrusted script blocked");

	lua_CFunction fn = lua_tocfunction(l, lua_upvalueindex(1));
	return fn(l);
}
