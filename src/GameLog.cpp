// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "GameLog.h"
#include "lua/Lua.h"

void GameLog::Add(const std::string &msg)
{
	Add("", msg, GameLog::Priority::PRIORITY_NORMAL);
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
