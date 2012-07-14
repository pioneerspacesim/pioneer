#ifndef _FONTCONFIG_H
#define _FONTCONFIG_H

#include "IniConfig.h"
#include "text/FontDescriptor.h"

class FontConfig : public IniConfig {
public:
	FontConfig();
	explicit FontConfig(const std::string &filename);

	Text::FontDescriptor GetDescriptor();

private:
	void SetDefaults();
};

#endif
