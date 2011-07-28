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

static bool _read_pair(FILE *f, const std::string &filename, int *lineno, token_map::iterator *outIter, char **outValue)
{
	static char buf[1024];
	*outValue = buf;

	bool doing_token = true;

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
					fprintf(stderr, "unexpected token character '%c' in line %d of '%s'", *c, *lineno, filename.c_str());
					return false;
				}
			}

			*outIter = s_tokens.find(std::string(line));
            if ((*outIter) == s_tokens.end()) {
                fprintf(stderr, "unknown token '%s' at line %d of '%s'", line, *lineno, filename.c_str());
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

			*outValue = line;
		}

		(*lineno)++;

		if (doing_token)
			doing_token = false;
		else
			break;
	}

	if (errno) {
		fprintf(stderr, "error reading string file '%s': %s", filename.c_str(), strerror(errno));
		return false;
	}

	return true;
}

static void _copy_string(const char *src, char *dest)
{
	// copy in one char at a time
	int s = 0, d = 0;
	while (src[s] != '\0' && d < MAX_STRING) {

		// turn \n into a real newline
		if (src[s] == '\\' && src[s+1] == 'n') {
			dest[d++] = '\n';
			s += 2;
			continue;
		}

		dest[d++] = src[s++];
	}

	dest[MAX_STRING-1] = '\0';
}

bool LoadStrings(const std::string &lang)
{
	for (token_map::iterator i = s_tokens.begin(); i != s_tokens.end(); i++)
		*((*i).second) = '\0';

	std::string filename(PIONEER_DATA_DIR "/lang/" + lang + ".txt");

	FILE *f = fopen(filename.c_str(), "r");
	if (!f) {
		fprintf(stderr, "couldn't open string file '%s': %s\n", filename.c_str(), strerror(errno));
		return false;
	}

	token_map::iterator token_iter;
	char *value;

	int lineno = 1;
	while (!feof(f)) {
		bool success = _read_pair(f, filename, &lineno, &token_iter, &value);
		if (!success)
			return false;

		_copy_string(value, (*token_iter).second);
	}

	fclose(f);

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
