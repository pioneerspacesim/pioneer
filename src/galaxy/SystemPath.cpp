#include "SystemPath.h"
#include <cstdlib>

static const char *SkipSpace(const char *s)
{ if (s) { while (isspace(*s)) { ++s; } } return s; }

static const char *TryParseInt(int &out, const char * const str)
{
	if (str) {
		char *end = 0;
		long n = strtol(str, &end, 10);
		if (str != end) {
			out = n;
			return end;
		} else {
			return 0;
		}
	} else {
		return 0;
	}
}

bool SystemPath::TryParse(SystemPath &out, const char * const str)
{
	assert(str);

	// syspath = '('? [+-]? [0-9]+ [, +-] [0-9]+ [, +-] [0-9]+ ')'?
	// with whitespace allowed between tokens

	const char *s = str;

	int x, y, z;

	// TryParseInt returns a null pointer on failure
	// SkipSpace and TryParseInt passes null pointers through

	printf("@ '%s'\n", s);
	s = SkipSpace(s);
	if (s && (*s == '(')) { ++s; }
	printf("@ '%s'\n", s);
	s = SkipSpace(s);
	s = TryParseInt(x, s);
	printf("@ '%s'\n", s);
	s = SkipSpace(s);
	if (s && (*s == ',' || *s == '.')) { ++s; s = SkipSpace(s); }
	s = TryParseInt(y, s);
	printf("@ '%s'\n", s);
	s = SkipSpace(s);
	if (s && (*s == ',' || *s == '.')) { ++s; s = SkipSpace(s); }
	s = TryParseInt(z, s);
	printf("@ '%s'\n", s);
	s = SkipSpace(s);
	if (s && (*s == ')')) { ++s; }
	s = SkipSpace(s);
	printf("@ '%s'\n", s);
	if (s && !*s) {
		out = SystemPath(x, y, z);
		return true;
	} else {
		return false;
	}
}
