// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "ModManager.h"
#include "FileSourceZip.h"
#include "FileSystem.h"
#include "utils.h"

std::vector<ModManager::ModInfo> ModManager::m_loadedMods;

void ModManager::Init()
{
	FileSystem::userFiles.MakeDirectory("mods");
}

void ModManager::Uninit()
{
	m_loadedMods.clear();
}

void ModManager::LoadMods(IniConfig *config)
{
	auto files = FileSystem::userFiles.Enumerate("mods", FileSystem::FileEnumerator::IncludeDirs);
	for (const FileSystem::FileInfo &fileInfo : files) {
		ModInfo newMod = {};
		newMod.name = fileInfo.GetName();
		newMod.path = FileSystem::JoinPath(FileSystem::userFiles.GetRoot(), fileInfo.GetPath());

		if (ends_with_ci(fileInfo.GetPath(), ".zip")) {
			newMod.name = newMod.name.substr(0, newMod.name.size() - 4);
			newMod.fs = std::make_unique<FileSystem::FileSourceZip>(FileSystem::userFiles, fileInfo.GetPath());
		} else if (fileInfo.IsDir()) {
			newMod.fs = std::make_unique<FileSystem::FileSourceFS>(newMod.path);
		}

		// Not a valid mod if we don't have a modfs for it
		if (!newMod.fs)
			continue;

		newMod.enabled = !compare_ci(config->String("ModLoader", newMod.name, ""), "disabled");
		newMod.loadOrder = config->Int("ModLoader", newMod.name, m_loadedMods.size());

		Log::Info("Found mod {}, enabled: {}", newMod.name, newMod.enabled);
		Log::Info("\tsource: {}", newMod.path);

		// Don't load the mod if it's been disabled
		m_loadedMods.emplace_back(std::move(newMod));
	}

	ReorderMods(config);

	Log::Info("Loaded mods:");
	for (const auto &modInfo : m_loadedMods) {
		if (modInfo.enabled) {
			Log::Info("\t{}", modInfo.name);
			FileSystem::gameDataFiles.PrependSource(modInfo.fs.get());
		}
	}
}

void ModManager::ReorderMods(IniConfig *config)
{
	std::sort(m_loadedMods.begin(), m_loadedMods.end(), [](const auto &a, const auto &b){ return a.loadOrder < b.loadOrder; });

	for (const auto &modInfo : m_loadedMods) {
		// write back the mod load order to the config file
		if (modInfo.enabled)
			config->SetInt("ModLoader", modInfo.name, modInfo.loadOrder);
		else
			config->SetString("ModLoader", modInfo.name, "disabled");
	}
}
