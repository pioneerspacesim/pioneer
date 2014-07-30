// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GAMECONFIG_H
#define _GAMECONFIG_H

#include "IniConfig.h"

class GameConfig : public IniConfig {
public:
	GameConfig(const std::map<std::string,std::string> &override_ = std::map<std::string,std::string>());

	void Load();
	bool Save();
};

#endif
