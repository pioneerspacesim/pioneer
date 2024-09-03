// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "SystemPath.h"
#include "GameSaveError.h"
#include "Json.h"
#include "fmt/format.h"
#include <cstdlib>

// https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
static std::vector<std::string> split(const std::string &str, const std::string &delim)
{
	std::vector<std::string> tokens;
	size_t prev = 0, pos = 0;
	do {
		pos = str.find(delim, prev);
		if (pos == std::string::npos) pos = str.length();
		std::string token = str.substr(prev, pos - prev);
		if (!token.empty()) tokens.push_back(token);
		prev = pos + delim.length();
	} while (pos < str.length() && prev < str.length());
	return tokens;
}

static int ParseInt(const std::string &str)
{
	int i = 0;
	try {
		i = std::stoi(str);
	} catch (const std::invalid_argument &e) {
		throw SystemPath::ParseFailure();
	}
	return i;
}

SystemPath SystemPath::Parse(const char *const str)
{
	// Parse a system path, three to five integers separated by commas (,), optionally starting with ( and ending with )
	// 0,0,0  or  (0, 0, 0)  or  1,3,-3,5  or  (0,0,0,0,18)
	assert(str);
	std::string s = str;

	assert(s.length() > 0);

	if (s[0] == '(')
		s = s.substr(1, s.length());

	if (s[s.length() - 1] == ')')
		s = s.substr(0, s.length() - 1);

	std::vector<std::string> parts = split(s, ",");
	int x = 0, y = 0, z = 0, si = 0, bi = 0;
	if (parts.size() < 3 || parts.size() > 5)
		throw SystemPath::ParseFailure();
	if (parts.size() >= 3) {
		x = ParseInt(parts[0].c_str());
		y = ParseInt(parts[1].c_str());
		z = ParseInt(parts[2].c_str());
	}
	if (parts.size() >= 4) {
		si = ParseInt(parts[3].c_str());
	}
	if (parts.size() == 5) {
		bi = ParseInt(parts[4].c_str());
	}
	return SystemPath(x, y, z, si, bi);
}

void SystemPath::ToJson(Json &jsonObj) const
{
	Json systemPathObj({}); // Create JSON object to contain system path data.
	systemPathObj["sector_x"] = sectorX;
	systemPathObj["sector_y"] = sectorY;
	systemPathObj["sector_z"] = sectorZ;
	systemPathObj["system_index"] = systemIndex;
	systemPathObj["body_index"] = bodyIndex;
	jsonObj["system_path"] = systemPathObj; // Add system path object to supplied object.
}

SystemPath SystemPath::FromJson(const Json &jsonObj)
{
	try {
		Json systemPathObj = jsonObj["system_path"];

		Sint32 x = systemPathObj["sector_x"];
		Sint32 y = systemPathObj["sector_y"];
		Sint32 z = systemPathObj["sector_z"];
		Uint32 si = systemPathObj["system_index"];
		Uint32 bi = systemPathObj["body_index"];

		return SystemPath(x, y, z, si, bi);
	} catch (Json::type_error &) {
		throw SavedGameCorruptException();
	}
}

std::string to_string(const SystemPath &path)
{
	return fmt::format("({},{},{},{},{})",
		path.sectorX, path.sectorY, path.sectorZ, path.systemIndex, path.bodyIndex);
}
