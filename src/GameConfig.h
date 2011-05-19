#ifndef _GAMECONFIG_H
#define _GAMECONFIG_H

#include "IniConfig.h"

class GameConfig : public IniConfig {
public:
	GameConfig(const std::string &filename);
};

#endif
