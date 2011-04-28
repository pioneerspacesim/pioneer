#include "LuaManager.h"
#include "oolua/oolua.h"

bool instantiated = false;

LuaManager::LuaManager() : m_lua(NULL) {
	if (instantiated) {
		fprintf(stderr, "Can't instantiate more than one LuaManager");
		abort();
	}

	m_lua = lua_open();

	luaL_openlibs(m_lua);

	lua_atpanic(m_lua, pi_lua_panic);

	// XXX remove once oolua is gone
	OOLUA::setup_user_lua_state(m_lua);

	instantiated = true;
}

LuaManager::~LuaManager() {
	lua_close(m_lua);

	instantiated = false;
}
