// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GALAXYCONFIG_H
#define _GALAXYCONFIG_H

#include "core/IniConfig.h"

class GalaxyConfig : public IniConfig {
public:
	typedef std::map<std::string, std::string> map_string;
	// GalaxyConfig(const map_string &override_ = map_string());
	GalaxyConfig();
};

#endif
