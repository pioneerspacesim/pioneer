// Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "SystemPath.h"
#include "GameSaveError.h"
#include "Json.h"
#include <cstdlib>

static int ParseInt(const char *&ss)
{
	assert(ss);
	char *end = 0;
	long n = strtol(ss, &end, 10);
	if (ss == end) {
		throw SystemPath::ParseFailure();
	} else {
		ss = end;
		return n;
	}
}

SystemPath SystemPath::Parse(const char * const str)
{
	assert(str);

	// syspath = '('? [+-]? [0-9]+ [, +-] [0-9]+ [, +-] [0-9]+ ')'?
	// with whitespace allowed between tokens

	const char *s = str;

	int x, y, z;

	while (isspace(*s)) { ++s; }
	if (*s == '(') { ++s; }

	x = ParseInt(s); // note: ParseInt (actually, strtol) skips leading whitespace itself

	while (isspace(*s)) { ++s; }
	if (*s == ',' || *s == '.') { ++s; }

	y = ParseInt(s);

	while (isspace(*s)) { ++s; }
	if (*s == ',' || *s == '.') { ++s; }

	z = ParseInt(s);

	while (isspace(*s)) { ++s; }
	if (*s == ')') { ++s; }
	while (isspace(*s)) { ++s; }

	if (*s) // extra unexpected text after the system path
		throw SystemPath::ParseFailure();
	else
		return SystemPath(x, y, z);
}

void SystemPath::ToJson(Json &jsonObj) const {
	Json systemPathObj({}); // Create JSON object to contain system path data.
	systemPathObj["sector_x"] = sectorX;
	systemPathObj["sector_y"] = sectorY;
	systemPathObj["sector_z"] = sectorZ;
	systemPathObj["system_index"] = systemIndex;
	systemPathObj["body_index"] = bodyIndex;
	jsonObj["system_path"] = systemPathObj; // Add system path object to supplied object.
}

SystemPath SystemPath::FromJson(const Json &jsonObj) {
	try {
		Json systemPathObj = jsonObj["system_path"];

		Sint32 x = systemPathObj["sector_x"];
		Sint32 y = systemPathObj["sector_y"];
		Sint32 z = systemPathObj["sector_z"];
		Uint32 si = systemPathObj["system_index"];
		Uint32 bi = systemPathObj["body_index"];

		return SystemPath(x, y, z, si, bi);
	} catch (Json::type_error &e) {
		throw SavedGameCorruptException();
	}
}
