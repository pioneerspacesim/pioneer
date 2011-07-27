#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <map>
#include <string>

#define MAX_STRING (1024)

namespace Lang {

typedef std::map<std::string,char*> token_map;

static token_map s_tokens;

}

#define DECLARE_STRING(x)                           \
	char x[MAX_STRING];                             \
	static class _init_class_##x {                  \
	public:                                         \
		_init_class_##x() {                         \
			s_tokens.insert(std::make_pair(#x,x)); \
			*x = '\0';                              \
		}                                           \
	} _init_##x

#include "Lang.h"

namespace Lang {

bool LoadStrings(char *lang)
{
	for (token_map::iterator i = s_tokens.begin(); i != s_tokens.end(); i++)
		*((*i).second) = '\0';

	std::string filename(PIONEER_DATA_DIR "/lang/" + std::string(lang) + ".txt");

	FILE *f = fopen(filename.c_str(), "r");
	if (!f) {
		fprintf(stderr, "couldn't open string file '%s': %s", filename.c_str(), strerror(errno));
		return false;
	}

	token_map::iterator token_iter;

	bool doing_token = true;
	int lineno = 1;
	char buf[1024];

	while (fgets(buf, sizeof(buf), f)) {
		char *line = buf;

		// eat leading whitespace
		while (*line == ' ' || *line == '\t' || *line == '\r' || *line == '\n') line++;

		// skip empty lines
		if (!*line) continue;

		// skip comment lines
		if (*line == '#') continue;

		// eat trailing whitespace
		char *end = strchr(line, '\0');
		while (*(end-1) == ' ' || *(end-1) == '\t' || *(end-1) == '\r' || *(end-1) == '\n') end--;
		*end = '\0';

		// skip empty lines
		if (line == end) continue;

		// token line
		if (doing_token) {
			for (char *c = line; c < end; c++) {
				// valid token chars are A-Z0-9_
				if (!((*c >= 'A' && *c <= 'Z') || (*c >= '0' && *c <= '9') || (*c == '_'))) {
					fprintf(stderr, "unexpected token character '%c' in line %d of '%s'", *c, lineno, filename.c_str());
					return false;
				}
			}

			token_iter = s_tokens.find(std::string(line));
            if (token_iter == s_tokens.end()) {
                fprintf(stderr, "unknown token '%s' at line %d of '%s'", line, lineno, filename.c_str());
                return false;
			}
		}

		else {
			// eat quotes
			if (*line == '"' && *(end-1) == '"') {
				line++;
				end--;
				*end = '\0';
			}

			// skip empty lines
			if (line == end) continue;

			char *str = (*token_iter).second;
			strncpy(str, line, MAX_STRING);
			str[MAX_STRING-1] = '\0';
		}

		doing_token = !doing_token;
		lineno++;
	}

	if (errno) {
		fprintf(stderr, "error reading string file for language '%s': %s", lang, strerror(errno));
		return false;
	}

	return true;
}

}
