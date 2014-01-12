// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "SystemPath.h"
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
