#include "LuaUI.h"
#include "LuaObject.h"
#include "LuaUtils.h"
#include "Pi.h"
#include "ShipCpanel.h"

/*
 * Interface: UI
 *
 * User interface functions.
 */

/*
 * Function: Message
 *
 * Post a message to the player's control panel.
 *
 * > UI.Message(message, from)
 *
 * Parameters:
 *
 *   message - the message text to post
 *
 *   from - optional; who the message is from (person, ship, etc)
 *
 * Example:
 *
 * > UI.Message("Please repair my ship.", "Gary Jones")
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   experimental
 */
static int l_ui_message(lua_State *l)
{
	std::string msg = luaL_checkstring(l, 1);

	std::string from;
	if (lua_gettop(l) >= 2)
		from = luaL_checkstring(l, 2);

	Pi::cpan->MsgLog()->Message(from, msg);
	return 0;
}

/*
 * Function: ImportantMessage
 *
 * Post an important message to the player's control panel.
 *
 * > UI.ImportantMessage(message, from)
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
 * > UI.ImportantMessage("Prepare to die!", "AB-1234")
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   experimental
 */
static int l_ui_important_message(lua_State *l)
{
	std::string msg = luaL_checkstring(l, 1);

	std::string from;
	if (lua_gettop(l) >= 2)
		from = luaL_checkstring(l, 2);

	Pi::cpan->MsgLog()->ImportantMessage(from, msg);
	return 0;
}

void LuaUI::Register()
{
	lua_State *l = Pi::luaManager->GetLuaState();

	LUA_DEBUG_START(l);

	static const luaL_reg methods[] = {
		{ "Message",          l_ui_message           },
		{ "ImportantMessage", l_ui_important_message },
		{ 0, 0 }
	};

	luaL_register(l, "UI", methods);
	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);
}
