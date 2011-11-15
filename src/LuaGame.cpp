#include "LuaGame.h"
#include "LuaObject.h"
#include "LuaUtils.h"
#include "LuaPlayer.h"
#include "LuaStarSystem.h"
#include "Pi.h"

/*
 * Interface: Game
 *
 * A global table that exposes a number of essential values relevant to the
 * current game.
 *
 */

static int l_game_meta_index(lua_State *l)
{
	if (!Pi::IsGameStarted())
		luaL_error(l, "Can't query game state when game is not started");

	const char *key = luaL_checkstring(l, 2);

	/*
	 * Attribute: player
	 *
	 * The <Player> object for the current player.
	 *
	 * Availability:
	 *
	 *  alpha 10
	 *
	 * Status:
	 *
	 *  stable
	 */
	if (strcmp(key, "player") == 0) {
		LuaPlayer::PushToLua(Pi::player);
		return 1;
	}

	/*
	 * Attribute: system
	 *
	 * The <StarSystem> object for the system the player is currently in.
	 *
	 * Availability:
	 *
	 *  alpha 10
	 *
	 * Status:
	 *
	 *  stable
	 */
	if (strcmp(key, "system") == 0) {
		LuaStarSystem::PushToLua(Pi::currentSystem.Get());
		return 1;
	}

	/*
	 * Attribute: time
	 *
	 * The current game time, in seconds since 12:00 01-01-3200
	 *
	 * Availability:
	 *
	 *  alpha 10
	 *
	 * Status:
	 *
	 *  stable
	 */
	if (strcmp(key, "time") == 0) {
		lua_pushnumber(l, Pi::GetGameTime());
		return 1;
	}

	lua_pushnil(l);
	return 1;
}

void LuaGame::Register()
{
	lua_State *l = Pi::luaManager->GetLuaState();

	LUA_DEBUG_START(l);

	lua_newtable(l);

	luaL_newmetatable(l, "Game");
	
	lua_pushstring(l, "__index");
	lua_pushcfunction(l, l_game_meta_index);
	lua_rawset(l, -3);

	lua_setmetatable(l, -2);

	lua_setfield(l, LUA_GLOBALSINDEX, "Game");
	
	LUA_DEBUG_END(l, 0);
}
