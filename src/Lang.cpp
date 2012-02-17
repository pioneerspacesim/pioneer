#include "libs.h"
#include "utils.h"
#include "TextSupport.h"
#include <map>

// XXX we're allocating a whole KB for each translatable string
// that's... not very nice (though I guess it "doesn't matter" with virtual memory and multi-GB of RAM)
#define MAX_STRING (1024)

namespace Lang {

// declaring value type as const char* so that we can give out a const reference to the real
// token map without allowing external code to modify token text
// unfortunately, this means we don't have write access internally either,
// so we have to const_cast<> to initialise the token values.
// this could be avoided by using a custom class for the value type
// (or std::string, but then we'd be changing the way translated text is stored)
typedef std::map<std::string, const char*> token_map;

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
	*outValue = 0;

	bool doing_token = true;

	char *line;

	errno = 0;
	while ((line = fgets(buf, sizeof(buf), f))) {
		(*lineno)++;

		int i = 0;
		while (buf[i]) {
			Uint32 chr;
			int n = conv_mb_to_wc(&chr, &buf[i]);
			if (!n) {
				fprintf(stderr, "invalid utf-8 character in line %d of '%s'\n", *lineno, filename.c_str());
				return false;
			}

			i += n;
		}

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
					fprintf(stderr, "unexpected token character '%c' in line %d of '%s'\n", *c, *lineno, filename.c_str());
					return false;
				}
			}

			*outIter = s_tokens.find(std::string(line));
            if ((*outIter) == s_tokens.end()) {
                fprintf(stderr, "unknown token '%s' at line %d of '%s'\n", line, *lineno, filename.c_str());
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

		if (doing_token)
			doing_token = false;
		else
			break;
	}

	if (errno) {
		fprintf(stderr, "error reading string file '%s': %s\n", filename.c_str(), strerror(errno));
		return false;
	}

	return true;
}

static void _copy_string(const char *src, char *dest)
{
	// copy in one char at a time
	int s = 0, d = 0;
	while (src[s] != '\0' && d < MAX_STRING-1) {

		// turn \n into a real newline
		if (src[s] == '\\' && src[s+1] == 'n') {
			dest[d++] = '\n';
			s += 2;
			continue;
		}

		dest[d++] = src[s++];
	}

	dest[d] = '\0';
}

bool LoadStrings(const std::string &lang)
{
	// XXX const_cast is ugly, but see note for declaration of tokens map
	for (token_map::iterator i = s_tokens.begin(); i != s_tokens.end(); i++)
		*const_cast<char*>((*i).second) = '\0';
	
	std::map<std::string,bool> seen, missing;

	token_map::iterator token_iter;
	char *value;

	std::string filename(PIONEER_DATA_DIR "/lang/English.txt");

	FILE *f = fopen(filename.c_str(), "r");
	if (!f) {
		fprintf(stderr, "couldn't open string file '%s': %s\n", filename.c_str(), strerror(errno));
		return false;
	}

	int lineno = 0;
	while (1) {
		bool success = _read_pair(f, filename, &lineno, &token_iter, &value);
		if (!success)
			return false;

		if (!value)
			break;

		// XXX const_cast is ugly, but see note for declaration of tokens map
		_copy_string(value, const_cast<char*>((*token_iter).second));
		seen.insert(std::make_pair((*token_iter).first, true));
	}

	fclose(f);

	if (seen.size() != s_tokens.size()) {
		fprintf(stderr, "string file '%s' has missing tokens:\n", filename.c_str());
		for (token_map::iterator i = s_tokens.begin(); i != s_tokens.end(); i++) {
			if (seen.find((*i).first) == seen.end()) {
				fprintf(stderr, "  %s\n", (*i).first.c_str());
				missing.insert(std::make_pair((*i).first, false));
			}
		}
	}

	if (lang == "English")
		return seen.size() == s_tokens.size();
	
	seen.clear();
	
	filename = std::string(PIONEER_DATA_DIR "/lang/" + lang + ".txt");

	f = fopen(filename.c_str(), "r");
	if (!f) {
		fprintf(stderr, "couldn't open string file '%s': %s\n", filename.c_str(), strerror(errno));
		// we failed to open/find the language file, but we've already successfully
		// read the default language above, so we still claim success here.
		return true;
	}

	lineno = 0;
	while (1) {
		bool success = _read_pair(f, filename, &lineno, &token_iter, &value);
		if (!success)
			return false;

		if (!value)
			break;

		// XXX const_cast is ugly, but see note for declaration of tokens map
		_copy_string(value, const_cast<char*>((*token_iter).second));
		seen.insert(std::make_pair((*token_iter).first, true));
		
		std::map<std::string,bool>::iterator i = missing.find((*token_iter).first);
		if (i != missing.end())
			missing.erase(i);
	}

	fclose(f);

	if (seen.size() != s_tokens.size()) {
		fprintf(stderr, "string file '%s' has missing tokens:\n", filename.c_str());
		for (token_map::iterator i = s_tokens.begin(); i != s_tokens.end(); i++) {
			if (seen.find((*i).first) == seen.end())
				fprintf(stderr, "  %s\n", (*i).first.c_str());
		}
	}

	if (missing.size() > 0) {
		fprintf(stderr, "no strings found for the following tokens:\n");
		for (std::map<std::string,bool>::iterator i = missing.begin(); i != missing.end(); i++)
			fprintf(stderr, "  %s\n", (*i).first.c_str());
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

const std::map<std::string, const char*> &GetDictionary()
{
    return s_tokens;
}

}
