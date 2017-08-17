#include "GameLog.h"
#include "StringF.h"
#include "graphics/Renderer.h"
#include "graphics/VertexArray.h"
#include "LuaObject.h"
#include "Game.h"
#include "Pi.h"

void GameLog::Add(const std::string &msg)
{

	// m_messages.push_back(Message(msg, 0));
	Add("", msg, GameLog::Priority::PRIORITY_NORMAL);
	// while (m_messages.size() > MAX_MESSAGES) m_messages.pop_front();
}

void GameLog::Add(const std::string &from, const std::string &msg, GameLog::Priority priority)
{
	// call lua Game.AddCommsLogLine(msg, from)
	lua_State *l = Lua::manager->GetLuaState();
	lua_getfield(l, LUA_REGISTRYINDEX, "CoreImports");
	lua_getfield(l, -1, "Game");
	lua_getfield(l, -1, "AddCommsLogLine");
	lua_remove(l, -2);
	lua_remove(l, -2);
	lua_pushstring(l, msg.c_str());
	lua_pushstring(l, from.c_str());
	lua_pushnumber(l, priority);
	lua_call(l, 3, 0);
}

