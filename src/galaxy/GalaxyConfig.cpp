// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "GalaxyConfig.h"
#include "FileSystem.h"

GalaxyConfig::GalaxyConfig()
{
	Read(FileSystem::userFiles, "galaxy.ini");

	// set defaults
	SetInt("GalaxyExploredMax", 90);
	SetInt("GalaxyExploredMin", 65);
	SetInt("GalaxyExploredMix", 40);

	if (HasUnsavedChanges()) {
		Save();
	}
}
