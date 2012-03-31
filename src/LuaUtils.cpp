#include "LuaUtils.h"
#include "libs.h"
#include "FileSystem.h"

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

static void pi_lua_dofile(lua_State *l, const FileSystem::FileData &code)
{
	// XXX make this a proper protected call (after working out the implications -- *sigh*)
	lua_pushcfunction(l, &pi_lua_panic);
	if (luaL_loadbuffer(l, code.GetData(), code.GetSize(), code.GetInfo().GetPath().c_str())) {
		pi_lua_panic(l);
	} else {
		lua_pcall(l, 0, 0, -2);
	}
	lua_pop(l, 1);

}

void pi_lua_dofile(lua_State *l, const std::string &path)
{
	assert(l);

	RefCountedPtr<FileSystem::FileData> code = FileSystem::gameDataFiles.ReadFile(path);
	if (!code) {
		fprintf(stderr, "could not read Lua file '%s'\n", path.c_str());
	}

	// XXX kill CurrentDirectory
	std::string dir = code->GetInfo().GetDir();
	if (dir.empty()) { dir = "."; }
	lua_pushstring(l, dir.c_str());
	lua_setglobal(l, "CurrentDirectory");

	pi_lua_dofile(l, *code);

	// XXX kill CurrentDirectory
	lua_pushnil(l);
	lua_setglobal(l, "CurrentDirectory");
}

void pi_lua_dofile_recursive(lua_State *l, const std::string &basepath)
{
	LUA_DEBUG_START(l);

	for (FileSystem::FileEnumerator files(FileSystem::gameDataFiles, basepath, FileSystem::FileEnumerator::IncludeDirectories); !files.Finished(); files.Next())
	{
		const FileSystem::FileInfo &info = files.Current();
		const std::string &fpath = info.GetPath();
		if (info.IsDir()) {
			pi_lua_dofile_recursive(l, fpath);
		} else if (info.IsFile()) {
			if ((fpath.size() > 4) && (fpath.substr(fpath.size() - 4) == ".lua")) {
				// XXX kill CurrentDirectory
				lua_pushstring(l, basepath.empty() ? "." : basepath.c_str());
				lua_setglobal(l, "CurrentDirectory");

				RefCountedPtr<FileSystem::FileData> code = info.Read();
				pi_lua_dofile(l, *code);
			}
		}
	}

	LUA_DEBUG_END(l, 0);
}

// XXX compatibility
int pi_load_lua(lua_State *l) {
	const std::string path = luaL_checkstring(l, 1);
	FileSystem::FileInfo info = FileSystem::gameDataFiles.Lookup(path);

	lua_getglobal(l, "CurrentDirectory");
	std::string currentDir = luaL_optstring(l, -1, "");
	lua_pop(l, 1);

	if (info.IsDir()) {
		pi_lua_dofile_recursive(l, path);
	} else if (info.IsFile() && (path.size() > 4) && (path.substr(path.size() - 4) == ".lua")) {
		pi_lua_dofile(l, path);
	} else if (info.IsFile()) {
		return luaL_error(l, "load_lua('%s') called on a file without a .lua extension", path.c_str());
	} else if (!info.Exists()) {
		return luaL_error(l, "load_lua('%s') called on a path that doesn't exist", path.c_str());
	} else {
		return luaL_error(l, "load_lua('%s') called on a path that doesn't refer to a valid file", path.c_str());
	}

	if (currentDir.empty())
		lua_pushnil(l);
	else
		lua_pushlstring(l, currentDir.c_str(), currentDir.size());
	lua_setglobal(l, "CurrentDirectory");

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
