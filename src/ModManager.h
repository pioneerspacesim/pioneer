// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _MODMANAGER_H
#define _MODMANAGER_H

#include "FileSystem.h"
#include "core/IniConfig.h"

#include <string>
#include <vector>

// right now this is little more than a stub class to hook up zipfiles to the
// virtual filesystem

class ModManager {
public:
	struct ModInfo {
		std::string name;
		std::string path;
		std::unique_ptr<FileSystem::FileSource> fs;
		mutable int loadOrder;
		bool enabled;
	};

	static void Init();
	static void Uninit();

	static void LoadMods(IniConfig *config);
	static void ReorderMods(IniConfig *config);

	static const std::vector<ModInfo> &EnumerateMods() { return m_loadedMods; }

private:

	static std::vector<ModInfo> m_loadedMods;

};

#endif
