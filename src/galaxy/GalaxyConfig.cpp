// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "GalaxyConfig.h"
#include "FileSystem.h"

GalaxyConfig::GalaxyConfig()
{
	// set defaults
	std::map<std::string, std::string> &map = m_map[""];
	map["GalaxyExploredMax"] = "90";
	map["GalaxyExploredMin"] = "65";
	map["GalaxyExploredMix"] = "40";

	Read(FileSystem::userFiles, "galaxy.ini");

	Save();
}
