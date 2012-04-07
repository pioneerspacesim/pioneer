#ifndef _FONTCONFIG_H
#define _FONTCONFIG_H

#include "IniConfig.h"

class FontConfig : public IniConfig {
public:
	FontConfig();
	explicit FontConfig(const std::string &filename);
private:
	void SetDefaults();
};

#endif
