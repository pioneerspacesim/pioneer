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
