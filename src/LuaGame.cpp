// Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaGame.h"
#include "LuaObject.h"
#include "LuaUtils.h"
#include "FileSystem.h"
#include "Player.h"
#include "Pi.h"
#include "Game.h"
#include "Lang.h"
#include "StringF.h"
#include "WorldView.h"
#include "DeathView.h"
#include "galaxy/Galaxy.h"
#include "DateTime.h"
#include "SectorView.h"
#include "ShipCpanel.h"

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
 * > Game.StartGame(system_path, start_time)
 *
 * Parameters:
 *
 *   system_path - A SystemBody to start at. If this is a starport, the player
 *                 will begin docked here; otherwise the player will begin in
 *                 orbit around the specified body.
 *
 *   start_time - optional, default 0. Time to start at in seconds from the
 *                Pioneer epoch (i.e. from 3200-01-01 00:00 UTC).
 *
 * Availability:
 *
 *   alpha 28
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

	SystemPath *path = LuaObject<SystemPath>::CheckFromLua(1);
	const double start_time = luaL_optnumber(l, 2, 0.0);
	try {
		Pi::game = new Game(*path, start_time);
	}
	catch (InvalidGameStartLocation& e) {
		luaL_error(l, "invalid starting location for game: %s", e.error.c_str());
	}
	return 0;
}

/*
 * Function: LoadGame
 *
 * Load a saved game.
 *
 * > Game.LoadGame(filename)
 *
 * Parameters:
 *
 *   filename - Filename to load. It will be loaded from the 'savefiles'
 *              directory in the user's game directory.
 *
 * Availability:
 *
 *   alpha 28
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

	const std::string filename(luaL_checkstring(l, 1));

	try {
		Pi::game = Game::LoadGame(filename);
	}
	catch (SavedGameCorruptException) {
		luaL_error(l, Lang::GAME_LOAD_CORRUPT);
	}
	catch (SavedGameWrongVersionException) {
		luaL_error(l, Lang::GAME_LOAD_WRONG_VERSION);
	}
	catch (CouldNotOpenFileException) {
		const std::string msg = stringf(Lang::GAME_LOAD_CANNOT_OPEN,
			formatarg("filename", filename));
		luaL_error(l, msg.c_str());
	}

	return 0;
}

/*
 * Function: CanLoadGame
 *
 * Does file exist for loading.
 *
 * > Game.CanLoadGame(filename)
 *
 * Parameters:
 *
 *   filename - Filename to find. 
 *
 * Return:
 *
 *   bool - can the filename be found to load
 *
 * Availability:
 *
 *   YYYY - MM - DD
 *   2016 - 06 - 25
 *
 * Status:
 *
 *   experimental
 */
static int l_game_can_load_game(lua_State *l)
{
	const std::string filename(luaL_checkstring(l, 1));

	bool success = Game::CanLoadGame(filename);
	lua_pushboolean(l, success);

	return 1;
}

/*
 * Function: SaveGame
 *
 * Save the current game.
 *
 * > path = Game.SaveGame(filename)
 *
 * Parameters:
 *
 *   filename - Filename to save to. The file will be placed the 'savefiles'
 *              directory in the user's game directory.
 *
 * Return:
 *
 *   path - the full path to the saved file (so it can be displayed)
 *
 * Availability:
 *
 *   June 2013
 *
 * Status:
 *
 *   experimental
 */
static int l_game_save_game(lua_State *l)
{
	if (!Pi::game) {
		return luaL_error(l, "can't save when no game is running");
	}

	const std::string filename(luaL_checkstring(l, 1));
	const std::string path = FileSystem::JoinPathBelow(Pi::GetSaveDir(), filename);

	try {
		Game::SaveGame(filename, Pi::game);
		lua_pushlstring(l, path.c_str(), path.size());
		return 1;
	}
	catch (CannotSaveInHyperspace) {
		return luaL_error(l, "%s", Lang::CANT_SAVE_IN_HYPERSPACE);
	}
	catch (CannotSaveDeadPlayer) {
		return luaL_error(l, "%s", Lang::CANT_SAVE_DEAD_PLAYER);
	}
	catch (CouldNotOpenFileException) {
		const std::string message = stringf(Lang::COULD_NOT_OPEN_FILENAME, formatarg("path", path));
		lua_pushlstring(l, message.c_str(), message.size());
		return lua_error(l);
	}
	catch (CouldNotWriteToFileException) {
		return luaL_error(l, "%s", Lang::GAME_SAVE_CANNOT_WRITE);
	}
}

/*
 * Function: EndGame
 *
 * End the current game and return to the main menu.
 *
 * > Game.EndGame(filename)
 *
 * Availability:
 *
 *   June 2013
 *
 * Status:
 *
 *   experimental
 */
static int l_game_end_game(lua_State *l)
{
	if (Pi::game) {
		// Request to end the game as soon as possible.
		// Previously could be called from Lua UI and delete the object doing the calling causing a crash.
		Pi::RequestEndGame();
	}
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
		LuaObject<Player>::PushToLua(Pi::player);
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
		LuaObject<StarSystem>::PushToLua(Pi::game->GetSpace()->GetStarSystem().Get());
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

/*
 * Attribute: paused
 *
 * True if the game is paused.
 *
 * Availability:
 *
 *  September 2014
 *
 * Status:
 *
 *  experimental
 */
static int l_game_attr_paused(lua_State *l)
{
	if (!Pi::game)
		lua_pushboolean(l, 1);
	else
		lua_pushboolean(l, Pi::game->IsPaused() ? 1 : 0);
	return 1;
}

// XXX temporary to support StationView "Launch" button
// remove once WorldView has been converted to the new UI
static int l_game_switch_view(lua_State *l)
{
	if (!Pi::game)
		return luaL_error(l, "can't switch view when no game is running");
	if (Pi::player->IsDead())
		Pi::SetView(Pi::game->GetDeathView());
	else
		Pi::SetView(Pi::game->GetWorldView());
	return 0;
}

static int l_game_set_view(lua_State *l)
{
	if (!Pi::game)
		return luaL_error(l, "can't set view when no game is running");
	std::string target = luaL_checkstring(l, 1);
	if(!target.compare("world")) {
		Pi::SetView(Pi::game->GetWorldView());
	} else if(!target.compare("space_station")) {
		Pi::SetView(Pi::game->GetSpaceStationView());
	} else if(!target.compare("info")) {
		Pi::SetView(Pi::game->GetInfoView());
	} else if(!target.compare("sector")) {
		// Pi::SetView(Pi::game->GetSectorView());
		Pi::game->GetCpan()->OnChangeToMapView(nullptr);
	} // TODO else error
	return 0;
}

static int l_game_get_view(lua_State *l)
{
	if(Pi::GetView() == Pi::game->GetWorldView())
		lua_pushstring(l, "world");
	else if(Pi::GetView() == Pi::game->GetSpaceStationView())
		lua_pushstring(l, "space_station");
	else if(Pi::GetView() == Pi::game->GetInfoView())
		lua_pushstring(l, "info");
	else if(Pi::GetView() == Pi::game->GetSectorView())
		lua_pushstring(l, "sector");
	// TODO else error
	return 1;
}

static int l_game_set_world_cam_type(lua_State *l)
{
	std::string cam = luaL_checkstring(l, 1);
	if(!cam.compare("internal"))
		Pi::game->GetWorldView()->SetCamType(WorldView::CAM_INTERNAL);
	else if(!cam.compare("external"))
		Pi::game->GetWorldView()->SetCamType(WorldView::CAM_EXTERNAL);
	else if(!cam.compare("sidereal"))
		Pi::game->GetWorldView()->SetCamType(WorldView::CAM_SIDEREAL);
	// TODO else error
	return 0;
}

static int l_game_get_date_time(lua_State *l)
{
	Time::DateTime t(Pi::game->GetTime());
	int year, month, day, hour, minute, second;
	t.GetDateParts(&year, &month, &day);
	t.GetTimeParts(&hour, &minute, &second);
	lua_pushinteger(l, year);
	lua_pushinteger(l, month);
	lua_pushinteger(l, day);
	lua_pushinteger(l, hour);
	lua_pushinteger(l, minute);
	lua_pushinteger(l, second);
	return 6;
}

void LuaGame::Register()
{
	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);

	static const luaL_Reg l_methods[] = {
		{ "StartGame",      l_game_start_game       },
		{ "LoadGame",       l_game_load_game        },
		{ "CanLoadGame",    l_game_can_load_game    },
		{ "SaveGame",       l_game_save_game        },
		{ "EndGame",        l_game_end_game         },

		{ "SwitchView", l_game_switch_view },
		{ "SetView",    l_game_set_view },
		{ "GetView",    l_game_get_view },
		{ "GetDateTime", l_game_get_date_time },
		{ "SetWorldCamType", l_game_set_world_cam_type },
		{ 0, 0 }
	};

	static const luaL_Reg l_attrs[] = {
		{ "player", l_game_attr_player },
		{ "system", l_game_attr_system },
		{ "time",   l_game_attr_time   },
		{ "paused", l_game_attr_paused },
		{ 0, 0 }
	};

	lua_getfield(l, LUA_REGISTRYINDEX, "CoreImports");
	LuaObjectBase::CreateObject(l_methods, l_attrs, 0);
	lua_setfield(l, -2, "Game");
	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);
}
