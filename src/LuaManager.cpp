#include "LuaManager.h"
#include "LuaObject.h"
#include "oolua/oolua.h"

std::auto_ptr<LuaManager> LuaManager::s_instance;

LuaManager::LuaManager() : m_lua(NULL) {
    m_lua = lua_open();

    luaL_openlibs(m_lua);

	// XXX remove once oolua is gone
	OOLUA::setup_user_lua_state(m_lua);
}

void LuaManager::Init()
{
	// initialise things that require the singleton
	LuaBody::RegisterClass();
	LuaShip::RegisterClass();
	LuaSpaceStation::RegisterClass();
	LuaPlanet::RegisterClass();
	LuaStar::RegisterClass();
	LuaPlayer::RegisterClass();
	LuaStarSystem::RegisterClass();
	LuaSBodyPath::RegisterClass();
}

LuaManager::~LuaManager() {
    lua_close(m_lua);
}
