// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "enum_table.h"
#include <stdio.h>
#include <map>
#include <string>

namespace EnumStrings {

	static std::map<std::string, std::map<int, std::string>> enumStrings;
	static std::map<std::string, std::map<std::string, int>> enumValues;

	void Init()
	{
		for (const EnumTable *table = ENUM_TABLES; table->name; table++) {
			std::map<int, std::string> &stringMap = enumStrings[table->name];
			std::map<std::string, int> &valueMap = enumValues[table->name];

			for (const EnumItem *item = table->first; item->name; item++) {
				stringMap.insert(std::make_pair(item->value, item->name));
				valueMap.insert(std::make_pair(item->name, item->value));
			}
		}
	}

	const char *GetString(const char *ns, int value)
	{
		std::map<std::string, std::map<int, std::string>>::const_iterator tableIter = enumStrings.find(ns);
		if (tableIter == enumStrings.end())
			return 0;

		const std::map<int, std::string> &table = tableIter->second;
		std::map<int, std::string>::const_iterator e = table.find(value);
		if (e == table.end())
			return 0;

		return e->second.c_str();
	}

	int GetValue(const char *ns, const char *name)
	{
		std::map<std::string, std::map<std::string, int>>::const_iterator tableIter = enumValues.find(ns);
		if (tableIter == enumValues.end())
			return -1;

		const std::map<std::string, int> &table = tableIter->second;
		std::map<std::string, int>::const_iterator e = table.find(name);
		if (e == table.end())
			return -1;

		return e->second;
	}

} // namespace EnumStrings
