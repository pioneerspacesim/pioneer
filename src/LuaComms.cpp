// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaComms.h"
#include "LuaObject.h"
#include "LuaUtils.h"
#include "Pi.h"
#include "ShipCpanel.h"
#include "GameLog.h"

/*
 * Interface: Comms
 *
 * Player communication functions.
 */

/*
 * Function: Message
 *
 * Post a message to the player's control panel.
 *
 * > Comms.Message(message, from)
 *
 * Parameters:
 *
 *   message - the message text to post
 *
 *   from - optional; who the message is from (person, ship, etc)
 *
 * Example:
 *
 * > Comms.Message("Please repair my ship.", "Gary Jones")
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   experimental
 */
static int l_comms_message(lua_State *l)
{
	if (!Pi::game || !Pi::game->GetCpan())
		luaL_error(l, "Control panel does not exist.");

	std::string msg = luaL_checkstring(l, 1);

	std::string from;
	if (lua_gettop(l) >= 2)
		from = luaL_checkstring(l, 2);

	Pi::game->log->Add(from, msg, GameLog::Priority::PRIORITY_NORMAL);
	return 0;
}

/*
 * Function: ImportantMessage
 *
 * Post an important message to the player's control panel.
 *
 * > Comms.ImportantMessage(message, from)
 *
 * The only difference between this and <Message> is that if multiple messages
 * arrive at the same time, the important ones will be shown first.
 *
 * Parameters:
 *
 *   message - the message text to post
 *
 *   from - optional; who the message is from (person, ship, etc)
 *
 * Example:
 *
 * > Comms.ImportantMessage("Prepare to die!", "AB-1234")
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   experimental
 */
static int l_comms_important_message(lua_State *l)
{
	if (!Pi::game || !Pi::game->GetCpan())
		luaL_error(l, "Control panel does not exist.");

	std::string msg = luaL_checkstring(l, 1);

	std::string from;
	if (lua_gettop(l) >= 2)
		from = luaL_checkstring(l, 2);

	Pi::game->log->Add(from, msg, GameLog::Priority::PRIORITY_ALERT);
	return 0;
}

void LuaComms::Register()
{
	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);

	static const luaL_Reg l_methods[] = {
		{ "Message",          l_comms_message           },
		{ "ImportantMessage", l_comms_important_message },
		{ 0, 0 }
	};

	lua_getfield(l, LUA_REGISTRYINDEX, "CoreImports");
	LuaObjectBase::CreateObject(l_methods, 0, 0);
	lua_setfield(l, -2, "Comms");
	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);
}
