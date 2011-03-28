#include "LuaManager.h"
#include "LuaObject.h"
#include "LuaEventQueue.h"
#include "PiLuaConstants.h"
#include "LuaUtils.h"
#include "oolua/oolua.h"

class LuaChatForm;

std::auto_ptr<LuaManager> LuaManager::s_instance;

LuaManager::LuaManager() : m_lua(NULL) {
	m_lua = lua_open();

	luaL_openlibs(m_lua);

	lua_atpanic(m_lua, pi_lua_panic);

	// XXX remove once oolua is gone
	OOLUA::setup_user_lua_state(m_lua);
}

void LuaManager::Init()
{
	// initialise things that require the singleton
    PiLuaConstants::RegisterConstants(m_lua);

	LuaObject<LuaEventQueueBase>::RegisterClass();

	LuaBody::RegisterClass();
	LuaShip::RegisterClass();
	LuaSpaceStation::RegisterClass();
	LuaPlanet::RegisterClass();
	LuaStar::RegisterClass();
	LuaPlayer::RegisterClass();
	LuaStarSystem::RegisterClass();
	LuaSBodyPath::RegisterClass();

	LuaObject<LuaChatForm>::RegisterClass();
}

LuaManager::~LuaManager() {
	lua_close(m_lua);
}
