// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "SaveGameManager.h"
#include "core/GZipFormat.h"
#include "core/Log.h"
#include "FileSystem.h"
#include "Game.h"
#include "GameSaveError.h"
#include "JsonUtils.h"
#include "Player.h"
#include "Pi.h"
#include "profiler/Profiler.h"


static const char s_saveDirName[] = "savefiles";

static const int s_saveVersion = 90;

// A simple job to load a savegame into a Json object
class LoadGameToJsonJob : public Job
{
public:
	LoadGameToJsonJob(std::string_view filename, void(*callback)(std::string_view, const Json &)) :
		m_filename(filename), m_callback(callback)
	{
	}

	virtual void OnRun() {
		m_rootNode = SaveGameManager::LoadGameToJson(m_filename);
	};
	virtual void OnFinish() {
		m_callback(m_filename, m_rootNode);
	};
	virtual void OnCancel() {};
private:
	std::string m_filename;
	Json m_rootNode;
	void(*m_callback)(std::string_view, const Json &);
};


void SaveGameManager::Init()
{
	if (!FileSystem::userFiles.MakeDirectory(s_saveDirName)) {
		throw CouldNotOpenFileException();
	}
}

void SaveGameManager::Uninit()
{
}

int SaveGameManager::CurrentSaveVersion()
{
	return s_saveVersion;
}

const std::string SaveGameManager::GetSaveGameDirectory()
{
	return FileSystem::JoinPath(FileSystem::userFiles.GetRoot(), s_saveDirName);
}

bool SaveGameManager::CanLoadGame(const std::string &name)
{
	FILE *f = NULL;
	try {
		f = FileSystem::userFiles.OpenReadStream(FileSystem::JoinPathBelow(s_saveDirName, name));
	} catch (const std::invalid_argument &) {
		return false;
	}
	if (!f) {
		return false;
	}

	fclose(f);
	return true;
}

Json SaveGameManager::LoadGameToJson(const std::string &filename)
{
	return JsonUtils::LoadJsonSaveFile(FileSystem::JoinPathBelow(s_saveDirName, filename), FileSystem::userFiles);
}

Job *SaveGameManager::LoadGameToJsonAsync(std::string_view filename, void(*callback)(std::string_view, const Json &))
{
	return new LoadGameToJsonJob(filename, callback);
}

Game *SaveGameManager::LoadGame(const std::string &name)
{
	Output("SaveGameManager::LoadGame('%s')\n", name.c_str());

	try {
		Json rootNode = LoadGameToJson(name);
		if (!rootNode.is_object()) {
			throw SavedGameCorruptException();
		}
		return new Game(LoadGameToJson(name));
	} catch (const Json::type_error &) {
		throw SavedGameCorruptException();
	} catch (const Json::out_of_range &) {
		throw SavedGameCorruptException();
	}
}

void SaveGameManager::SaveGame(const std::string &name, Game *game)
{
	PROFILE_SCOPED()
	assert(game);

	if (game->IsHyperspace()) {
		throw CannotSaveInHyperspace();
	}

	if (game->GetPlayer()->IsDead()) {
		throw CannotSaveDeadPlayer();
	}

	if (!FileSystem::IsValidFilename(name)) {
		throw std::invalid_argument(name);
	}

	FILE *f = NULL;
	try {
		f = FileSystem::userFiles.OpenWriteStream(FileSystem::JoinPathBelow(s_saveDirName, name));
	} catch (const std::invalid_argument &) {
		throw CouldNotOpenFileException();
	}
	if (!f) {
		throw CouldNotOpenFileException();
	}
	Json rootNode;
	game->ToJson(rootNode); // Encode the game data as JSON and give to the root value.
	std::vector<uint8_t> jsonData;
	{
		PROFILE_SCOPED_DESC("json.to_cbor");
		jsonData = Json::to_cbor(rootNode); // Convert the JSON data to CBOR.
	}

	try {
		// Compress the CBOR data.
		const std::string comressed_data = gzip::CompressGZip(
			std::string(reinterpret_cast<const char *>(jsonData.data()), jsonData.size()),
			name + ".json");
		size_t nwritten = fwrite(comressed_data.data(), comressed_data.size(), 1, f);
		fclose(f);
		if (nwritten != 1) {
			throw CouldNotWriteToFileException();
		}
	} catch (gzip::CompressionFailedException) {
		fclose(f);
		throw CouldNotWriteToFileException();
	}

	Pi::GetApp()->RequestProfileFrame("SaveGame");
}


bool SaveGameManager::DeleteSave(const std::string &name)
{
	std::string filePath;
	try {
		filePath = FileSystem::JoinPathBelow(s_saveDirName, name);
	} catch (const std::invalid_argument &) {
		return false;
	}
	return FileSystem::userFiles.RemoveFile(filePath);
}

std::vector<FileSystem::FileInfo> SaveGameManager::ListSaves()
{
	std::vector<FileSystem::FileInfo> saves;
	auto files = FileSystem::userFiles.Enumerate(s_saveDirName, 0);
	for (const FileSystem::FileInfo &fileInfo : files) {
		// MKW TODO : should probably check that this file is actually a valid
		// savegame file. But that would require actually loading the file,
		// parsing it into a JSON object, and at the very least extracting the
		// version number.
		saves.push_back(fileInfo);
	}
	return saves;
}

