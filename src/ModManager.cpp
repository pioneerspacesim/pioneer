// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "ModManager.h"
#include "FileSystem.h"
#include "FileSourceZip.h"
#include "utils.h"

void ModManager::Init() {
	FileSystem::userFiles.MakeDirectory("mods");

	for (FileSystem::FileEnumerator files(FileSystem::userFiles, "mods", 0); !files.Finished(); files.Next()) {
		const FileSystem::FileInfo &info = files.Current();
		const std::string &zipPath = info.GetPath();
		if (ends_with_ci(zipPath, ".zip")) {
			Output("adding mod: %s\n", zipPath.c_str());
			FileSystem::gameDataFiles.PrependSource(new FileSystem::FileSourceZip(FileSystem::userFiles, zipPath));
		}
	}
}
