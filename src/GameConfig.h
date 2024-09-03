// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GAMECONFIG_H
#define _GAMECONFIG_H

#include "core/IniConfig.h"

class GameConfig : public IniConfig {
public:
	typedef std::map<std::string, std::string> map_string;
	GameConfig(const map_string &override_ = map_string());
};

#endif
