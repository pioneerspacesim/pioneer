#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <map>
#include <string>

namespace Lang {

class token_data {
public:
	token_data(const char **target) : m_target(target) {
		*m_target = m_string.c_str();
	}
	void set_string(const char *str) {
		m_string = std::string(str);
		*m_target = m_string.c_str();
	}
private:
	std::string  m_string;
	const char **m_target;
};

typedef std::map<std::string,token_data> token_map;

static token_map s_tokens;

}

#define DECLARE_STRING(x)                       \
	const char *x;                              \
	static class _init_class_##x {              \
	public:                                     \
		_init_class_##x() {                     \
			s_tokens.insert(                    \
				std::make_pair(                 \
					std::string(#x),            \
					token_data(&x)));           \
		}                                       \
	} _init_##x

#include "Lang.h"

namespace Lang {

bool LoadStrings(char *lang)
{
	for (token_map::iterator i = s_tokens.begin(); i != s_tokens.end(); i++)
		(*i).second.set_string("");

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
			
			(*token_iter).second.set_string(line);
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
