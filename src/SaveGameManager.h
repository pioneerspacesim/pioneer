// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SAVEGAMEMANAGER_H
#define _SAVEGAMEMANAGER_H

#include "FileSystem.h"
#include "JsonFwd.h"

#include <string>
#include <vector>

class Game;

class SaveGameManager {
public:
	static void Init();
	static void Uninit();

	/** Return currently supported version of the save game format. */
	static int CurrentSaveVersion();

	/** Return the savegame direcotry name.
	 * NOTE: This should only be used for logging purposes. The game nor any
	 * mods should ever directly access the savegame directory.
	 */
	static const std::string GetSaveGameDirectory();

	/** Check whether the named save exists and can be opened.
	 * NOTE: This does not check whether the named save actually contains a
	 *       compatible savegame file.
	 */
	static bool CanLoadGame(const std::string &name);

	/** Load a game and return a new Game object.
	 * This function will throw an exception if there is an error during the
	 * loading process.
	*/
	static Game *LoadGame(const std::string &name);

	/** Loads a game and returns it as a Json object.
	 * NOTE: this function will not perform any sanity checks that the loaded
	 * file actually contains a Pioneer savegame. It will return a null Json
	 * object if the contents of the file could not be decoded as a Json object.
	 *
	 * This is only required to support the LUA game upgrade/recovery
	 * implementation.
	 */
	static Json LoadGameToJson(const std::string &name);

	/** Save the game.
	 * NOTE: This function will throw an exception if an error occurs while
	 * saving the game.
	*/
	static void SaveGame(const std::string &name, Game *game);

	/** Delete a savegame file. */
	static bool DeleteSave(const std::string &name);

	/** Return a  list of saved games. */
	static std::vector<FileSystem::FileInfo> ListSaves();
};

#endif
