#include "LuaManager.h"
#include "FileSystem.h"
#include <cstdlib>

bool instantiated = false;

const std::string luaDebugFilename("pidebug.lua");

LuaManager::LuaManager() : m_lua(NULL) {
	if (instantiated) {
		fprintf(stderr, "Can't instantiate more than one LuaManager");
		abort();
	}

	m_lua = lua_open();

	luaL_openlibs(m_lua);

	lua_atpanic(m_lua, pi_lua_panic);

	RefCountedPtr<FileSystem::FileData> code = FileSystem::gameDataFiles.ReadFile(luaDebugFilename);
	if (!code) {
		fprintf(stderr, "could not read Lua file '%s'\n", luaDebugFilename.c_str());
		abort();
	}

	int ret = luaL_loadbuffer(m_lua, code->GetData(), code->GetSize(), code->GetInfo().GetPath().c_str());
	if (ret) {
		if (ret == LUA_ERRSYNTAX) {
			const char* message = lua_tostring(m_lua, -1);
			fprintf(stderr, "Syntax error in '%s:\n%s\n", code->GetInfo().GetAbsolutePath().c_str(), message);
		}
		else
			fprintf(stderr, "Error while loading '%s'\n", code->GetInfo().GetAbsolutePath().c_str());
		abort();
	}
	if (lua_pcall(m_lua, 0, 1, 0)) {
		fprintf(stderr, "Fatal Lua error: pidebug.lua failed to initialise.");
		abort();
	}
	if (lua_type(m_lua, -1) != LUA_TTABLE) {
		fprintf(stderr, "Fatal Lua error: pidebug.lua did not return module table.");
		abort();
	}
	lua_getfield(m_lua, -1, "error_handler");
	if (lua_type(m_lua, -1) != LUA_TFUNCTION) {
		fprintf(stderr, "Fatal Lua error: pidebug.lua did not define error_handler function.");
		abort();
	}
	lua_pop(m_lua, 1);
	lua_setfield(m_lua, LUA_REGISTRYINDEX, "PiDebug");

	instantiated = true;
}

LuaManager::~LuaManager() {
	lua_close(m_lua);

	instantiated = false;
}

size_t LuaManager::GetMemoryUsage() const {
	int kb = lua_gc(m_lua, LUA_GCCOUNT, 0);
	int b = lua_gc(m_lua, LUA_GCCOUNTB, 0);
	return (size_t(kb) * 1024) + b;
}

void LuaManager::CollectGarbage() {
	lua_gc(m_lua, LUA_GCCOLLECT, 0);
}
