// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

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
