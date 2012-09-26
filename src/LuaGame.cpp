// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaGame.h"
#include "LuaObject.h"
#include "LuaUtils.h"
#include "LuaPlayer.h"
#include "LuaStarSystem.h"
#include "LuaSystemPath.h"
#include "Pi.h"
#include "Game.h"

/*
 * Interface: Game
 *
 * A global table that exposes a number of essential values relevant to the
 * current game.
 *
 */

static int l_game_start_game(lua_State *l)
{
	if (Pi::game) {
		luaL_error(l, "can't start a new game while a game is already running");
		return 0;
	}

	SystemPath *path = LuaSystemPath::GetFromLua(1);

	RefCountedPtr<StarSystem> system(StarSystem::GetCached(*path));
	SystemBody *sbody = system->GetBodyByPath(path);

	if (sbody->GetSuperType() == SystemBody::SUPERTYPE_STARPORT)
		Pi::game = new Game(*path);
	else
		Pi::game = new Game(*path, vector3d(0, 1.5*sbody->GetRadius(), 0));

	return 0;
}

static int l_game_end_game(lua_State *l)
{
	if (!Pi::game)
		return 0;
	
	// XXX stuff
	
	return 0;
}

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
static int l_game_attr_player(lua_State *l)
{
	if (!Pi::game)
		lua_pushnil(l);
	else
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
static int l_game_attr_system(lua_State *l)
{
	if (!Pi::game)
		lua_pushnil(l);
	else
		LuaStarSystem::PushToLua(Pi::game->GetSpace()->GetStarSystem().Get());
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
static int l_game_attr_time(lua_State *l)
{
	if (!Pi::game)
		lua_pushnil(l);
	else
		lua_pushnumber(l, Pi::game->GetTime());
	return 1;
}

void LuaGame::Register()
{
	static const luaL_Reg l_methods[] = {
		{ "StartGame", l_game_start_game },
		{ "EndGame",   l_game_end_game   },
		{ 0, 0 }
	};

	static const luaL_Reg l_attrs[] = {
		{ "player", l_game_attr_player },
		{ "system", l_game_attr_system },
		{ "time",   l_game_attr_time   },
		{ 0, 0 }
	};

	LuaObjectBase::CreateObject(l_methods, l_attrs, 0);
	lua_setglobal(Lua::manager->GetLuaState(), "Game");
}
