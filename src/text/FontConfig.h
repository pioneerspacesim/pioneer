#ifndef _TEXT_FONTCONFIG_H
#define _TEXT_FONTCONFIG_H

#include "IniConfig.h"

namespace Text {

class FontConfig : public IniConfig {
public:
	FontConfig();
	explicit FontConfig(const std::string &filename);
private:
	void SetDefaults();
};

}

#endif
