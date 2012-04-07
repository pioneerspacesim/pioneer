#include "libs.h"
#include "Lang.h"
#include "FileSystem.h"
#include "StringRange.h"
#include "utils.h"
#include "text/TextSupport.h"
#include <map>
#include <set>

namespace Lang {

// XXX we're allocating half a KB for each translatable string
// that's... not very nice (though I guess it "doesn't matter" with virtual memory and multi-GB of RAM)
static const int STRING_RECORD_SIZE = 512;
#define DECLARE_STRING(x) char x[STRING_RECORD_SIZE];
#include "LangStrings.inc.h"
#undef DECLARE_STRING

}

namespace {

// declaring value type as const char* so that we can give out a const reference to the real
// token map without allowing external code to modify token text
// unfortunately, this means we don't have write access internally either,
// so we have to const_cast<> to initialise the token values.
// this could be avoided by using a custom class for the value type
// (or std::string, but then we'd be changing the way translated text is stored)
typedef std::map<std::string, const char*> token_map;
static token_map s_token_map;

static struct init_string_helper_class {
	init_string_helper_class() {
#define DECLARE_STRING(x)  s_token_map.insert(std::make_pair(#x, Lang::x)); Lang::x[0] = '\0';
#include "LangStrings.inc.h"
#undef DECLARE_STRING
	}
} init_string_helper;

static bool ident_head(char c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_');
}

static bool ident_tail(char c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || (c == '_');
}

static bool valid_token(StringRange tok)
{
	if (tok.Empty()) return false;
	if (!ident_head(tok[0])) return false;
	for (const char *c = tok.begin + 1; c != tok.end; ++c) {
		if (!ident_tail(*c)) return false;
	}
	return true;
}

// returns 0 on success, or a line number on error
static int valid_utf8(StringRange data)
{
	const char *c = data.begin;
	int line = 1;
	while (c != data.end) {
		Uint32 chr;
		int n = Text::conv_mb_to_wc(&chr, c);
		if (!n) return line;
		if (chr == '\n') ++line;
		c += n;
	}
	return 0;
}

struct StringFileParser {
public:
	StringFileParser(const std::string &filename, StringRange range);

	bool Finished() const { return m_data.Empty() && m_token.Empty(); }
	void Next();

	const std::string &GetFileName() const { return m_filename; }

	StringRange GetToken() const { return m_token; }
	int GetTokenLineNumber() const { return m_tokenLine; }
	StringRange GetText() const { return m_text; }
	int GetTextLineNumber() const { return m_textLine; }

	const std::string &GetAdjustedText() const { return m_adjustedText; }

private:
	StringRange NextLine();
	void SkipBlankLines();
	void ScanText();

	std::string m_filename;
	StringRange m_data;
	StringRange m_token;
	StringRange m_text;
	int m_lineNo;
	int m_tokenLine;
	int m_textLine;
	std::string m_adjustedText;
};

StringFileParser::StringFileParser(const std::string &filename, StringRange range):
	m_filename(filename), m_data(range), m_lineNo(0), m_tokenLine(-1), m_textLine(-1)
{
	assert(m_data.begin && m_data.end);
	SkipBlankLines();
	Next();
}

void StringFileParser::Next()
{
	if (!m_data.Empty()) {
		m_token = NextLine();
		m_tokenLine = m_lineNo;
		m_text = NextLine();
		m_textLine = m_lineNo;
		SkipBlankLines();

		if (!m_text.Empty() && (m_text[0] == '"') && (m_text[m_text.Size()-1] == '"')) {
			++m_text.begin;
			--m_text.end;
		}

		// adjust for escaped newlines
		{
			m_adjustedText.clear();
			m_adjustedText.reserve(m_text.Size());

			const char *end1 = (m_text.end - 1);
			const char *c;
			for (c = m_text.begin; c < end1; ++c) {
				if (c[0] == '\\' && c[1] == 'n') {
					++c;
					m_adjustedText += '\n';
				} else {
					m_adjustedText += *c;
				}
			}
			if (c != m_text.end) {
				m_adjustedText += *c++;
			}
			assert(c == m_text.end);
		}

		if (!valid_token(m_token)) {
			fprintf(stderr, "Invalid token '%.*s' at %s:%d\n", int(m_token.Size()), m_token.begin, m_filename.c_str(), m_tokenLine);
		}

		if (m_token.Empty()) {
			fprintf(stderr, "Blank token at %s:%d\n", m_filename.c_str(), m_tokenLine);
		}

		if (m_text.Empty()) {
			fprintf(stderr, "Blank string for token '%.*s' (at %s:%d)\n", int(m_token.Size()), m_token.begin, m_filename.c_str(), m_textLine);
		}
	} else {
		m_token.begin = m_token.end = 0;
		m_tokenLine = m_lineNo;
		m_text.begin = m_text.end = 0;
		m_textLine = m_lineNo;
		m_adjustedText.clear();
	}
}

StringRange StringFileParser::NextLine()
{
	if (!m_data.Empty()) {
		++m_lineNo;
		StringRange line = m_data.ReadLine();
		return line.StripSpace();
	} else
		return m_data;
}

void StringFileParser::SkipBlankLines()
{
	if (!m_data.Empty()) {
		StringRange line;
		// skip empty lines and comments
		while (!m_data.Empty() && (line.Empty() || line[0] == '#')) line = NextLine();
		--m_lineNo;
		m_data.begin = line.begin;
	}
}

static void ResetStringData()
{
	const char *badstring = "<badstring>";
	size_t sz = strlen(badstring) + 1;
#define DECLARE_STRING(x) memcpy(Lang::x, badstring, sz);
#include "LangStrings.inc.h"
#undef DECLARE_STRING
}

static std::vector<std::string> EnumAvailableLanguages()
{
	std::vector<std::string> languages;

	for (FileSystem::FileEnumerator files(FileSystem::gameDataFiles, "lang"); !files.Finished(); files.Next()) {
		assert(files.Current().IsFile());
		const std::string &path = files.Current().GetPath();
		if ((path.size() > 4) && (path.substr(path.size() - 4) == ".txt")) {
			const std::string name = files.Current().GetName();
			languages.push_back(name.substr(0, name.size() - 4));
		}
	}

	return languages;
}

static void copy_string(char *buf, const char *str, size_t strsize, size_t bufsize)
{
	size_t sz = std::min(strsize, bufsize-1);
	memcpy(buf, str, sz);
	buf[sz] = '\0';
}

} // anonymous namespace

namespace Lang {

bool LoadStrings(const std::string &lang)
{
	int errline;
	std::set<std::string> seen, missing;

	ResetStringData();

	std::string filename = "lang/English.txt";
	RefCountedPtr<FileSystem::FileData> english_data = FileSystem::gameDataFiles.ReadFile(filename);
	if (!english_data) {
		fprintf(stderr, "couldn't open string file '%s'\n", filename.c_str());
		return false;
	}

	errline = valid_utf8(english_data->AsStringRange());
	if (errline) {
		fprintf(stderr, "invalid UTF-8 code in line %d of '%s'\n", errline, filename.c_str());
		return false;
	}

	seen.clear();
	for (StringFileParser parser(filename, english_data->AsStringRange()); !parser.Finished(); parser.Next()) {
		const std::string token = parser.GetToken().ToString();
		token_map::iterator it = s_token_map.find(token);
		if (it != s_token_map.end()) {
			seen.insert(token);
			const std::string &text = parser.GetAdjustedText();
			// XXX const_cast is ugly, but see note for declaration of tokens map
			char *record = const_cast<char*>(it->second);
			copy_string(record, text.c_str(), text.size(), STRING_RECORD_SIZE);
		} else {
			fprintf(stderr, "unknown language token '%s' at %s:%d\n", token.c_str(), parser.GetFileName().c_str(), parser.GetTokenLineNumber());
		}
	}

	english_data.Reset();

	if (seen.size() != s_token_map.size()) {
		fprintf(stderr, "string file '%s' has missing tokens:\n", filename.c_str());
		for (token_map::iterator it = s_token_map.begin(); it != s_token_map.end(); ++it) {
			if (!seen.count(it->first)) {
				fprintf(stderr, "  %s\n", it->first.c_str());
				missing.insert(it->first);
			}
		}
	}

	if (lang == "English")
		return (seen.size() == s_token_map.size());

	filename = "lang/" + lang + ".txt";
	RefCountedPtr<FileSystem::FileData> lang_data = FileSystem::gameDataFiles.ReadFile(filename);
	if (!lang_data) {
		fprintf(stderr, "couldn't open string file '%s'\n", filename.c_str());
		return false;
	}

	errline = valid_utf8(lang_data->AsStringRange());
	if (errline) {
		fprintf(stderr, "invalid UTF-8 code in line %d of '%s'\n", errline, filename.c_str());
		return false;
	}

	seen.clear();
	for (StringFileParser parser(filename, lang_data->AsStringRange()); !parser.Finished(); parser.Next()) {
		const std::string token = parser.GetToken().ToString();
		token_map::iterator it = s_token_map.find(token);
		if (it != s_token_map.end()) {
			seen.insert(token);
			const std::string &text = parser.GetAdjustedText();
			// XXX const_cast is ugly, but see note for declaration of tokens map
			char *record = const_cast<char*>(it->second);
			copy_string(record, text.c_str(), text.size(), STRING_RECORD_SIZE);
		} else {
			fprintf(stderr, "unknown language token '%s' at %s:%d\n", token.c_str(), parser.GetFileName().c_str(), parser.GetTokenLineNumber());
		}
	}

	if (seen.size() != s_token_map.size()) {
		fprintf(stderr, "string file '%s' has missing tokens:\n", filename.c_str());
		for (token_map::iterator it = s_token_map.begin(); it != s_token_map.end(); ++it) {
			if (!seen.count(it->first)) {
				fprintf(stderr, "  %s\n", it->first.c_str());
			} else {
				missing.erase(it->first);
			}
		}
	}

	if (!missing.empty()) {
		fprintf(stderr, "no strings found for the following tokens:\n");
		for (std::set<std::string>::iterator it = missing.begin(); it != missing.end(); ++it) {
			fprintf(stderr, "  %s\n", it->c_str());
		}
		return false;
	}

	return true;
}

const std::vector<std::string> &GetAvailableLanguages()
{
	static std::vector<std::string> languages = EnumAvailableLanguages();
	return languages;
}

const std::map<std::string, const char*> &GetDictionary()
{
    return s_token_map;
}

} // namespace Lang
