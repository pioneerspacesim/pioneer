#include "LuaUtils.h"
#include "libs.h"
#include <set>
#if defined(_WIN32) && !defined(__MINGW32__)
#include "win32-dirent.h"
#else
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

static int _ro_table_error(lua_State *l)
{
	luaL_error(l, "Attempt to modify read-only table");
	return 0;
}

void pi_lua_table_ro(lua_State *l)
{
	lua_newtable(l);
	lua_pushstring(l, "__index");
	lua_pushvalue(l, -3);
	lua_rawset(l, -3);
	lua_pushstring(l, "__newindex");
	lua_pushcfunction(l, _ro_table_error);
	lua_rawset(l, -3);
	lua_pushstring(l, "__metatable");
	lua_pushboolean(l, false);
	lua_rawset(l, -3);
	lua_setmetatable(l, -2);
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
	lua_getfield(L, LUA_REGISTRYINDEX, "PiDebug");
	lua_getfield(L, -1, "error_handler");
	lua_insert(L, handleridx);
	lua_pop(L, 1); // pop PiDebug table
	int ret = lua_pcall(L, nargs, nresults, handleridx);
	lua_remove(L, handleridx); // pop error_handler
	if (ret) {
		std::string errorMsg = lua_tostring(L, -1);
		lua_pop(L, 1);
		Error("%s", errorMsg.c_str());
	}
}

void pi_lua_dofile_recursive(lua_State *l, const std::string &basepath)
{
	DIR *dir;
	struct dirent *entry;
	std::string path;
	struct stat info;
	// putting directory contents into sorted order so order of
	// model definition is consistent
	std::set<std::string> entries;

	LUA_DEBUG_START(l);

	// XXX CurrentDirectory stuff has to go
	lua_getglobal(l, "CurrentDirectory");
	std::string save_dir = luaL_checkstring(l, -1);
	lua_pop(l, 1);

	lua_pushstring(l, basepath.c_str());
	lua_setglobal(l, "CurrentDirectory");

	if ((dir = opendir(basepath.c_str())) == NULL) {
		fprintf(stderr, "opendir: couldn't open directory '%s': %s\n", basepath.c_str(), strerror(errno));
		return;
	}

	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_name[0] != '.') {
			entries.insert(entry->d_name);
		}
	}
	closedir(dir);

	for (std::set<std::string>::iterator i = entries.begin(); i!=entries.end(); ++i) {
		const std::string &name = *i;
		path = basepath + "/" + name;

		if (stat(path.c_str(), &info) != 0) {
			fprintf(stderr, "stat: couldn't get info for '%s': %s\n", path.c_str(), strerror(errno));
			continue;
		}

		if (S_ISDIR(info.st_mode)) {
			pi_lua_dofile_recursive(l, path.c_str());
			continue;
		}

		if ( name.size() >= 4 && name.find(".lua") == name.size() - 4 ) {
			// XXX panic stuff can be removed once the global lua is used everywhere
			lua_pushcfunction(l, pi_lua_panic);
			if (luaL_loadfile(l, path.c_str())) {
				pi_lua_panic(l);
			} else {
				lua_pcall(l, 0, 0, -2);
			}
			lua_pop(l, 1);
		}
	}

	lua_pushstring(l, save_dir.c_str());
	lua_setglobal(l, "CurrentDirectory");

	LUA_DEBUG_END(l, 0);
}

// XXX compatibility
int pi_load_lua(lua_State *l) {
	const char *path = luaL_checkstring(l, 1);
	pi_lua_dofile_recursive(l, path);
	return 0;
}

void pi_lua_warn(lua_State *l, const char *format, ...)
{
	char buf[1024];
	va_list ap;
	va_start(ap, format);
	vsnprintf(buf, sizeof(buf), format, ap);
	va_end(ap);
	fprintf(stderr, "Lua Warning: %s\n", buf);

	lua_Debug info;
	int level = 0;
	while (lua_getstack(l, level, &info)) {
		lua_getinfo(l, "nSl", &info);
		fprintf(stderr, "  [%d] %s:%d -- %s [%s]\n",
			level, info.short_src, info.currentline,
			(info.name ? info.name : "<unknown>"), info.what);
		++level;
	}
}
