#include "ModManager.h"
#include "FileSystem.h"
#include "FileSourceZip.h"

void ModManager::Init() {
	FileSystem::rawFileSystem.MakeDirectory(FileSystem::GetUserDir("mods"));

	FileSystem::FileSourceFS modFiles(FileSystem::GetUserDir("mods"));
	for (FileSystem::FileEnumerator files(modFiles, "", 0); !files.Finished(); files.Next()) {
		const FileSystem::FileInfo &info = files.Current();
		const std::string &zipPath = info.GetPath();
		if (zipPath.size() > 4 && zipPath.substr(zipPath.size()-4) == ".zip") {
			printf("adding mod: %s\n", zipPath.c_str());
			FileSystem::gameDataFiles.AppendSource(new FileSystem::FileSourceZip(FileSystem::JoinPathBelow(modFiles.GetRoot(), zipPath)));
		}
	}
}
