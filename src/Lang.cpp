#include "libs.h"
#include <map>

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

		// string line
		else {
			// eat quotes
			if (*line == '"' && *(end-1) == '"') {
				line++;
				end--;
				*end = '\0';
			}

			// skip empty lines
			if (line == end) continue;

			// target buffer
			char *str = (*token_iter).second;

			// copy in one char at a time
			int s = 0, d = 0;
			while (line[s] != '\0' && d < MAX_STRING) {

				// turn \n into a real newline
				if (line[s] == '\\' && line[s+1] == 'n') {
					str[d++] = '\n';
					s += 2;
					continue;
				}

				str[d++] = line[s++];
			}

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

std::list<std::string> s_availableLanguages;

void _found_language_file_callback(const std::string &name, const std::string &fullname) {
	size_t pos = name.find(".txt");
	if (pos == name.npos) return;
	if (name.length() - pos != 4) return;
	s_availableLanguages.push_back(name.substr(0, pos));
}

const std::list<std::string> &GetAvailableLanguages()
{
	s_availableLanguages.clear();

	foreach_file_in(PIONEER_DATA_DIR "/lang/", _found_language_file_callback);

	return s_availableLanguages;
}

}
