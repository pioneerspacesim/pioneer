// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaGame.h"
#include "DateTime.h"
#include "DeathView.h"
#include "FileSystem.h"
#include "Game.h"
#include "GameSaveError.h"
#include "Json.h"
#include "Lang.h"
#include "LuaObject.h"
#include "LuaTable.h"
#include "LuaUtils.h"
#include "Pi.h"
#include "Player.h"
#include "SectorView.h"
#include "Space.h"
#include "StringF.h"
#include "SystemView.h"
#include "WorldView.h"
#include "core/GZipFormat.h"
#include "galaxy/Galaxy.h"
#include "pigui/LuaPiGui.h"

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
 * > Game.StartGame(system_path, start_time, ship_type)
 *
 * Parameters:
 *
 *   system_path - A SystemBody to start at. If this is a starport, the player
 *                 will begin docked here; otherwise the player will begin in
 *                 orbit around the specified body.
 *
 *   start_time - Time to start at in seconds from the Pioneer epoch
 *                (i.e. from 3200-01-01 00:00 UTC).
 *
 *   ship_type - string, ship id, i.e. 'sinonatrix'
 *
 *
 * Availability:
 *
 *   2023
 *
 * Status:
 *
 *   experimental
 */
static int l_game_start_game(lua_State *l)
{
	if (Pi::game) {
		return luaL_error(l, "can't start a new game while a game is already running");
	}

	const auto path = LuaPull<SystemPath>(l, 1);
	const auto time = LuaPull<double>(l, 2);
	const auto shipType = LuaPull<const char *>(l, 3);

	Pi::StartGame(new Game(path, time, shipType));
	return 0;
}

/*
 * Function: SaveGameStats
 *
 * Return stats about a game.
 *
 * > Game.SaveGameStats(filename)
 *
 * Parameters:
 *
 *   filename - The filename of the save game to retrieve stats for.
 *              Stats will be loaded from the 'savefiles' directory in the user's game directory.
 *
 * Availability:
 *
 *   2018-02-10
 *
 * Status:
 *
 *   experimental
 */
static int l_game_savegame_stats(lua_State *l)
{
	const std::string filename = LuaPull<std::string>(l, 1);

	try {
		Json rootNode = Game::LoadGameToJson(filename);

		LuaTable t(l, 0, 3);

		t.Set("time", rootNode["time"].get<double>());

		// if this is a newer saved game, show the embedded info
		if (rootNode["game_info"].is_object()) {
			Json gameInfo = rootNode["game_info"];
			t.Set("system", gameInfo["system"].get<std::string>());
			t.Set("ship", gameInfo["ship"].get<std::string>());
			t.Set("credits", gameInfo["credits"].get<float>());
			t.Set("flight_state", gameInfo["flight_state"].get<std::string>());
			if (gameInfo["docked_at"].is_string())
				t.Set("docked_at", gameInfo["docked_at"].get<std::string>());
		} else {
			// this is an older saved game...try to show something useful
			Json shipNode = rootNode["space"]["bodies"][rootNode["player"].get<int>() - 1];
			t.Set("frame", rootNode["space"]["bodies"][shipNode["body"]["index_for_frame"].get<int>() - 1]["body"]["label"].get<std::string>());
			t.Set("ship", shipNode["model_body"]["model_name"].get<std::string>());
		}

		return 1;
	} catch (const CouldNotOpenFileException &e) {
		const std::string message = stringf(Lang::COULD_NOT_OPEN_FILENAME, formatarg("path", filename));
		lua_pushlstring(l, message.c_str(), message.size());
		return lua_error(l);
	} catch (const Json::type_error &) {
		return luaL_error(l, Lang::GAME_LOAD_CORRUPT);
	} catch (const Json::out_of_range &) {
		return luaL_error(l, Lang::GAME_LOAD_CORRUPT);
	} catch (const SavedGameCorruptException &) {
		return luaL_error(l, Lang::GAME_LOAD_CORRUPT);
	}
}


/*
 * Function: CurrentSaveVersion
 *
 * Returns the current version of the game save file format.
 *
 * > Game.CurrentSaveVersion()
 *
 * Return:
 *
 *   number
 *
 */
static int l_game_current_save_version(lua_State *l)
{
	LuaPush(l, Game::CurrentSaveVersion());
	return 1;
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
		return luaL_error(l, "can't load a game while a game is already running");
	}

	const std::string filename(luaL_checkstring(l, 1));

	try {
		Pi::StartGame(Game::LoadGame(filename));
	} catch (const SavedGameCorruptException &) {
		return luaL_error(l, Lang::GAME_LOAD_CORRUPT);
	} catch (const SavedGameWrongVersionException &) {
		return luaL_error(l, Lang::GAME_LOAD_WRONG_VERSION);
	} catch (const CouldNotOpenFileException &) {
		const std::string msg = stringf(Lang::GAME_LOAD_CANNOT_OPEN, formatarg("filename", filename));
		return luaL_error(l, msg.c_str());
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

	const bool success = Game::CanLoadGame(filename);
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
	std::string path;

	try {
		path = FileSystem::JoinPathBelow(Pi::GetSaveDir(), filename);
		Game::SaveGame(filename, Pi::game);
		lua_pushlstring(l, path.c_str(), path.size());
		return 1;
	} catch (const CannotSaveInHyperspace &) {
		return luaL_error(l, "%s", Lang::CANT_SAVE_IN_HYPERSPACE);
	} catch (const CannotSaveDeadPlayer &) {
		return luaL_error(l, "%s", Lang::CANT_SAVE_DEAD_PLAYER);
	} catch (const CouldNotOpenFileException &) {
		const std::string message = stringf(Lang::COULD_NOT_OPEN_FILENAME, formatarg("path", path));
		lua_pushlstring(l, message.c_str(), message.size());
		return lua_error(l);
	} catch (const CouldNotWriteToFileException &) {
		return luaL_error(l, "%s", Lang::GAME_SAVE_CANNOT_WRITE);
	} catch (const std::invalid_argument &) {
		return luaL_error(l, "%s", Lang::GAME_SAVE_INVALID_NAME);
	}
}

/*
 * Function: DeleteSave
 *
 * Deletes save with specified file name.
 *
 * > Game.DeleteSave(filename)
 *
 * Parameters:
 *
 *   filename - Filename to delete. The file will be checked at 'savefiles'
 *              directory in the user's game directory.
 * Return:
 *
 *   bool - is save deleted successfully
 *
 * Availability:
 *
 *   November 2023
 *
 * Status:
 *
 *   experimental
 */
static int l_game_delete_save(lua_State *l)
{
	const std::string filename(luaL_checkstring(l, 1));
	lua_pushboolean(l, Game::DeleteSave(filename));
	return 1;
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
 * Attribute: systemView
 *
 * The <SystemView> object for the system map view class
 *
 * Availability:
 *
 *  February 2020
 *
 * Status:
 *
 *  experiment
 */
static int l_game_attr_systemview(lua_State *l)
{
	if (!Pi::game)
		lua_pushnil(l);
	else
		LuaObject<SystemView>::PushToLua(Pi::game->GetSystemView());
	return 1;
}

/*
 * Attribute: sectorView
 *
 * The <SectorView> object for the sector map view class
 *
 * Availability:
 *
 *  April 2020
 *
 * Status:
 *
 *  experiment
 */
static int l_game_attr_sectorview(lua_State *l)
{
	if (!Pi::game)
		lua_pushnil(l);
	else
		LuaObject<SectorView>::PushToLua(Pi::game->GetSectorView());
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
		lua_pushboolean(l, Pi::game->IsPaused());
	return 1;
}

/*
 * Function: InHyperspace
 *
 * Return true if the game is in hyperspace mode
 *
 * > hyperspace = Game.InHyperspace()
 *
 * Return:
 *
 *   hyperspace - true if the game is currently in hyperspace
 *
 * Availability:
 *
 *   2017-04
 *
 * Status:
 *
 *   stable
 */

static int l_game_in_hyperspace(lua_State *l)
{
	LuaPush(l, Pi::game->IsHyperspace() || Pi::player->GetFlightState() == Ship::HYPERSPACE);
	return 1;
}

/*
 * Function: CurrentView
 *
 * Return the currently active game view
 *
 * > current_view = Game.CurrentView()
 *
 * Return:
 *
 *   view - a string describing the game view: "world", "space_station", "info", "sector", "system", "death", "settings"
 *
 * Availability:
 *
 *   2017-04
 *
 * Status:
 *
 *   stable
 */

static int l_game_current_view(lua_State *l)
{
	const View *view = Pi::GetView();
	if (view == Pi::game->GetWorldView())
		LuaPush(l, "world");
	else if (view == Pi::game->GetSpaceStationView())
		LuaPush(l, "space_station");
	else if (view == Pi::game->GetInfoView())
		LuaPush(l, "info");
	else if (view == Pi::game->GetSectorView())
		LuaPush(l, "sector");
	else if (view == Pi::game->GetSystemView())
		LuaPush(l, "system");
	else if (view == Pi::game->GetDeathView())
		LuaPush(l, "death");
	else
		lua_pushnil(l);
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

static int pushTimeAccel(lua_State *l, const Game::TimeAccel accel)
{
	switch (accel) {
	case Game::TIMEACCEL_PAUSED: lua_pushstring(l, "paused"); break;
	case Game::TIMEACCEL_1X: lua_pushstring(l, "1x"); break;
	case Game::TIMEACCEL_10X: lua_pushstring(l, "10x"); break;
	case Game::TIMEACCEL_100X: lua_pushstring(l, "100x"); break;
	case Game::TIMEACCEL_1000X: lua_pushstring(l, "1000x"); break;
	case Game::TIMEACCEL_10000X: lua_pushstring(l, "10000x"); break;
	case Game::TIMEACCEL_HYPERSPACE: lua_pushstring(l, "hyperspace"); break;
	default:
		return luaL_error(l, "TimeAccel value of \"%d\" is outside of Game::TimeAccel enum", accel);
	}
	return 1;
}

static int l_game_get_time_acceleration(lua_State *l)
{
	const Game::TimeAccel accel = Pi::game->GetTimeAccel();
	return pushTimeAccel(l, accel);
}

static int l_game_get_requested_time_acceleration(lua_State *l)
{
	const Game::TimeAccel accel = Pi::game->GetRequestedTimeAccel();
	return pushTimeAccel(l, accel);
}

static int l_game_set_time_acceleration(lua_State *l)
{
	const std::string accel = LuaPull<std::string>(l, 1);
	const bool force = LuaPull<bool>(l, 2);
	Game::TimeAccel a = Game::TIMEACCEL_PAUSED;
	if (!accel.compare("paused"))
		a = Game::TIMEACCEL_PAUSED;
	else if (!accel.compare("1x"))
		a = Game::TIMEACCEL_1X;
	else if (!accel.compare("10x"))
		a = Game::TIMEACCEL_10X;
	else if (!accel.compare("100x"))
		a = Game::TIMEACCEL_100X;
	else if (!accel.compare("1000x"))
		a = Game::TIMEACCEL_1000X;
	else if (!accel.compare("10000x"))
		a = Game::TIMEACCEL_10000X;
	else if (!accel.compare("hyperspace"))
		a = Game::TIMEACCEL_HYPERSPACE;
	else
		return luaL_error(l, "Unknown time acceleration %s", accel.c_str());
	Pi::game->RequestTimeAccel(a, force);
	return 0;
}

static int l_game_get_date_time(lua_State *l)
{
	const Time::DateTime t(Pi::game->GetTime());
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

static int l_game_set_view(lua_State *l)
{
	if (!Pi::game)
		return luaL_error(l, "can't set view when no game is running");
	const std::string target = luaL_checkstring(l, 1);
	if (!target.compare("world")) {
		Pi::SetView(Pi::game->GetWorldView());
	} else if (!target.compare("space_station")) {
		Pi::SetView(Pi::game->GetSpaceStationView());
	} else if (!target.compare("info")) {
		Pi::SetView(Pi::game->GetInfoView());
	} else if (!target.compare("death")) {
		Pi::SetView(Pi::game->GetDeathView());
	} else if (!target.compare("sector")) {
		Pi::SetView(Pi::game->GetSectorView());
	} else if (!target.compare("system")) {
		Pi::SetView(Pi::game->GetSystemView());
	} else {
		return luaL_error(l, "Unknown view %s", target.c_str());
	}
	return 0;
}

static int l_game_get_world_cam_type(lua_State *l)
{
	switch (Pi::game->GetWorldView()->shipView->GetCamType()) {
	case ShipViewController::CAM_INTERNAL: lua_pushstring(l, "internal"); break;
	case ShipViewController::CAM_EXTERNAL: lua_pushstring(l, "external"); break;
	case ShipViewController::CAM_SIDEREAL: lua_pushstring(l, "sidereal"); break;
	case ShipViewController::CAM_FLYBY: lua_pushstring(l, "flyby"); break;
	default: Output("Unknown world view cam type\n"); break;
	}
	return 1;
}

static int l_game_set_world_cam_type(lua_State *l)
{
	const std::string cam = luaL_checkstring(l, 1);
	if (!cam.compare("internal"))
		Pi::game->GetWorldView()->shipView->SetCamType(ShipViewController::CAM_INTERNAL);
	else if (!cam.compare("external"))
		Pi::game->GetWorldView()->shipView->SetCamType(ShipViewController::CAM_EXTERNAL);
	else if (!cam.compare("sidereal"))
		Pi::game->GetWorldView()->shipView->SetCamType(ShipViewController::CAM_SIDEREAL);
	else if (!cam.compare("flyby"))
		Pi::game->GetWorldView()->shipView->SetCamType(ShipViewController::CAM_FLYBY);
	else
		return luaL_error(l, "Unknown world cam type %s", cam.c_str());
	return 0;
}

static int l_game_get_hyperspace_travelled_percentage(lua_State *l)
{
	LuaPush(l, Pi::game->GetHyperspaceArrivalProbability());
	return 1;
}

static int l_game_get_parts_from_date_time(lua_State *l)
{
	const double time = LuaPull<double>(l, 1);
	const Time::DateTime t(time);
	int year, month, day, hour, minute, second;
	t.GetDateParts(&year, &month, &day);
	t.GetTimeParts(&hour, &minute, &second);
	LuaPush(l, second);
	LuaPush(l, minute);
	LuaPush(l, hour);
	LuaPush(l, day);
	LuaPush(l, month);
	LuaPush(l, year);
	return 6;
}

void LuaGame::Register()
{
	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);

	static const luaL_Reg l_methods[] = {
		{ "StartGame", l_game_start_game },
		{ "LoadGame", l_game_load_game },
		{ "CanLoadGame", l_game_can_load_game },
		{ "SaveGame", l_game_save_game },
		{ "DeleteSave", l_game_delete_save },
		{ "EndGame", l_game_end_game },
		{ "InHyperspace", l_game_in_hyperspace },
		{ "SaveGameStats", l_game_savegame_stats },
		{ "CurrentSaveVersion", l_game_current_save_version },

		{ "SwitchView", l_game_switch_view },
		{ "CurrentView", l_game_current_view },
		{ "SetView", l_game_set_view },
		{ "GetDateTime", l_game_get_date_time },
		{ "GetPartsFromDateTime", l_game_get_parts_from_date_time },
		{ "SetTimeAcceleration", l_game_set_time_acceleration },
		{ "GetTimeAcceleration", l_game_get_time_acceleration },
		{ "GetRequestedTimeAcceleration", l_game_get_requested_time_acceleration },
		{ "GetHyperspaceTravelledPercentage", l_game_get_hyperspace_travelled_percentage },

		{ "SetWorldCamType", l_game_set_world_cam_type },
		{ "GetWorldCamType", l_game_get_world_cam_type },

		{ 0, 0 }
	};

	static const luaL_Reg l_attrs[] = {
		{ "player", l_game_attr_player },
		{ "system", l_game_attr_system },
		{ "systemView", l_game_attr_systemview },
		{ "sectorView", l_game_attr_sectorview },
		{ "time", l_game_attr_time },
		{ "paused", l_game_attr_paused },
		{ 0, 0 }
	};

	lua_getfield(l, LUA_REGISTRYINDEX, "CoreImports");
	LuaObjectBase::CreateObject(l_methods, l_attrs, 0);
	lua_setfield(l, -2, "Game");
	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);
}
