// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LANG_H
#define _LANG_H

#include <vector>
#include <string>
#include <map>

namespace Lang {

bool LoadStrings(const std::string &lang);
const std::vector<std::string> &GetAvailableLanguages();
const std::map<std::string, const char*> &GetDictionary();

// declare all strings
#define DECLARE_STRING(x) extern char x[];
#include "LangStrings.inc.h"
#undef DECLARE_STRING

}

#endif
