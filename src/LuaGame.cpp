// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
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
		luaL_error(l, Lang::GAME_LOAD_CANNOT_OPEN);
	}

	return 0;
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

	if (Pi::game->IsHyperspace()) {
		return luaL_error(l, "%s", Lang::CANT_SAVE_IN_HYPERSPACE);
	}

	const std::string filename(luaL_checkstring(l, 1));
	const std::string path = FileSystem::JoinPathBelow(Pi::GetSaveDir(), filename);

	try {
		Game::SaveGame(filename, Pi::game);
		lua_pushlstring(l, path.c_str(), path.size());
		return 1;
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
		Pi::EndGame();
	}
	return 0;
}

/*
 * Function: SetHyperspaceAllowInAtmosphere(flag)
 *
 * Sets whether hyperjumps are allowed in atmosphere.
 *
 * > Game.SetHyperspaceAllowInAtmosphere(true)
 *
 * Parameters:
 *
 *   flag - true to allow jump in atmosphere, false/nil to disallow
 *
 * Availability:
 *
 *   TBD
 *
 * Status:
 *
 *   experimental
 */
static int l_game_set_hyperspace_allow_in_atmosphere(lua_State *l)
{
	if (Pi::game) {
		bool allowInAtmosphere = false;
		if (!lua_isnil(l, 1)) {
			luaL_checktype(l, 1, LUA_TBOOLEAN);
			allowInAtmosphere = lua_toboolean(l, 1);
		}
		Pi::game->SetHyperspaceAllowInAtmosphere(allowInAtmosphere);
	}
	return 0;
}

/*
 * Function: SetHyperspaceMinAltitude(altitude)
 *
 * Sets the minimal distance to planet surfaces to initiate hyperjump.
 *
 * > Game.SetHyperspaceMinAltitude(altitude)
 *
 * Parameters:
 *
 *   altitude - Minimum altitude for hyperjumps in km.
 *
 * Availability:
 *
 *   TBD
 *
 * Status:
 *
 *   experimental
 */
static int l_game_set_hyperspace_min_altitude(lua_State *l)
{
	if (Pi::game) {
		double minAltitude = luaL_checknumber(l, 1) * 1000.0;
		Pi::game->SetHyperspaceMinTerrainDistance(minAltitude);
	}
	return 0;
}

/*
 * Function: SetHyperspaceMinStationDistance(distance)
 *
 * Sets the minimal distance to stations to initiate hyperjump.
 *
 * > Game.SetHyperspaceMinStationDistance(distance)
 *
 * Parameters:
 *
 *   distance - Minimum distance from space stations for hyperjumps in km.
 *
 * Availability:
 *
 *   TBD
 *
 * Status:
 *
 *   experimental
 */
static int l_game_set_hyperspace_min_station_distance(lua_State *l)
{
	if (Pi::game) {
		double minDistance = luaL_checknumber(l, 1) * 1000.0;
		Pi::game->SetHyperspaceMinStationDistance(minDistance);
	}
	return 0;
}

/*
 * Function: IsHyperspaceAllowedInAtmosphere()
 *
 * Checks whether hyperjumps are allowed in atmosphere.
 *
 * > if Game.IsHyperspaceAllowedInAtmosphere() then ... end
 *
 * Availability:
 *
 *   TBD
 *
 * Status:
 *
 *   experimental
 */
static int l_game_is_hyperspace_allowed_in_atmosphere(lua_State *l)
{
	if (Pi::game) {
		lua_pushboolean(l, Pi::game->IsHyperspaceAllowedInAtmosphere());
		return 1;
	}
	return 0;
}

/*
 * Function: GetHyperspaceMinAltitude()
 *
 * Gets the minimal distance to planet surfaces to initiate hyperjump.
 *
 * > altitude = Game.GetHyperspaceMinAltitude()
 *
 * Return:
 *
 *   altitude - Minimum altitude for hyperjumps in km.
 *
 * Availability:
 *
 *   TBD
 *
 * Status:
 *
 *   experimental
 */
static int l_game_get_hyperspace_min_altitude(lua_State *l)
{
	if (Pi::game) {
		lua_pushnumber(l, Pi::game->GetHyperspaceMinTerrainDistance() / 1000.0);
		return 1;
	}
	return 0;
}

/*
 * Function: GetHyperspaceMinStationDistance(distance)
 *
 * Gets the minimal distance to stations to initiate hyperjump.
 *
 * > distance = Game.GetHyperspaceMinStationDistance()
 *
 * Return:
 *
 *   distance - Minimum distance from space stations for hyperjumps in km.
 *
 * Availability:
 *
 *   TBD
 *
 * Status:
 *
 *   experimental
 */
static int l_game_get_hyperspace_min_station_distance(lua_State *l)
{
	if (Pi::game) {
		lua_pushnumber(l, Pi::game->GetHyperspaceMinStationDistance() / 1000.0);
		return 1;
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

void LuaGame::Register()
{
	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);

	static const luaL_Reg l_methods[] = {
		{ "StartGame",                       l_game_start_game                          },
		{ "LoadGame",                        l_game_load_game                           },
		{ "SaveGame",                        l_game_save_game                           },
		{ "EndGame",                         l_game_end_game                            },
		{ "SetHyperspaceAllowInAtmosphere",  l_game_set_hyperspace_allow_in_atmosphere  },
		{ "SetHyperspaceMinAltitude",        l_game_set_hyperspace_min_altitude         },
		{ "SetHyperspaceMinStationDistance", l_game_set_hyperspace_min_station_distance },
		{ "IsHyperspaceAllowedInAtmosphere", l_game_is_hyperspace_allowed_in_atmosphere },
		{ "GetHyperspaceMinAltitude",        l_game_get_hyperspace_min_altitude         },
		{ "GetHyperspaceMinStationDistance", l_game_get_hyperspace_min_station_distance },
		{ 0, 0 }
	};

	static const luaL_Reg l_attrs[] = {
		{ "player",              l_game_attr_player      },
		{ "system",              l_game_attr_system      },
		{ "time",                l_game_attr_time        },
		{ 0, 0 }
	};

	lua_getfield(l, LUA_REGISTRYINDEX, "CoreImports");
	LuaObjectBase::CreateObject(l_methods, l_attrs, 0);
	lua_setfield(l, -2, "Game");
	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);
}
