// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaGame.h"
#include "LuaObject.h"
#include "LuaUtils.h"
#include "LuaPlayer.h"
#include "LuaStarSystem.h"
#include "LuaSystemPath.h"
#include "FileSystem.h"
#include "Pi.h"
#include "Game.h"
#include "Lang.h"

/*
 * Interface: Game
 *
 * A global table that exposes a number of essential values relevant to the
 * current game.
 *
 */

/*
 * Function: StartGame
 *
 * Start a new game.
 *
 * > Game.StartGame(system_path)
 *
 * Parameters:
 *
 *   system_path - A SystemBody to start at. If this is a starport, the player
 *                 will begin docked here; otherwise the player will begin in
 *                 orbit around the specified body.
 *
 * Availability:
 *
 *   not yet
 *
 * Status:
 *
 *   experimental
 */
static int l_game_start_game(lua_State *l)
{
	if (Pi::game) {
		luaL_error(l, "can't start a new game while a game is already running");
		return 0;
	}

	SystemPath *path = LuaSystemPath::CheckFromLua(1);

	RefCountedPtr<StarSystem> system(StarSystem::GetCached(*path));
	SystemBody *sbody = system->GetBodyByPath(path);

	if (sbody->GetSuperType() == SystemBody::SUPERTYPE_STARPORT)
		Pi::game = new Game(*path);
	else
		Pi::game = new Game(*path, vector3d(0, 1.5*sbody->GetRadius(), 0));

	return 0;
}

/*
 * Function: LoadGame
 *
 * Load a saved game.
 *
 * > loaded = Game.LoadGame(filename)
 *
 * Parameters:
 *
 *   filename - Filename to load. It will be loaded from the 'savefiles'
 *              directory in the user's game directory.
 *
 * Availability:
 *
 *   not yet
 *
 * Status:
 *
 *   experimental
 */
static int l_game_load_game(lua_State *l)
{
	if (Pi::game) {
		luaL_error(l, "can't load a game while a game is already running");
		return 0;
	}

	const std::string filename(FileSystem::JoinPathBelow(Pi::SAVE_DIR_NAME, luaL_checkstring(l, 1)));

	Game *newGame = 0;

	// XXX use FileSystem stuff
	try {
		FILE *f = FileSystem::userFiles.OpenReadStream(filename);
		if (!f) throw CouldNotOpenFileException();

		Serializer::Reader rd(f);
		fclose(f);

		newGame = new Game(rd);
	}
	catch (SavedGameCorruptException) {
		luaL_error(l, Lang::GAME_LOAD_CORRUPT);
	}
	catch (CouldNotOpenFileException) {
		luaL_error(l, Lang::GAME_LOAD_CANNOT_OPEN);
	}

	// XXX deal with this gracefully
	if (!newGame)
		abort();

	Pi::game = newGame;

	return 0;
}

static int l_game_end_game(lua_State *l)
{
	if (!Pi::game)
		return 0;
	
	// XXX stuff
	return luaL_error(l, "Game.EndGame() is not yet implemented");
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
		{ "LoadGame",  l_game_load_game  },
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
