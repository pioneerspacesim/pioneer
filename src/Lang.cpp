// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "Lang.h"
#include "FileSystem.h"
#include "StringRange.h"
#include "utils.h"
#include "text/TextSupport.h"
#include "json/json.h"
#include <map>
#include <set>

namespace Lang {

// XXX we're allocating a KB for each translatable string
// that's... not very nice (though I guess it "doesn't matter" with virtual memory and multi-GB of RAM)
static const int STRING_RECORD_SIZE = 1024;
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
static std::string s_current_language("English");

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

static bool valid_token(const std::string &token)
{
	if (token.empty()) return false;
	if (!ident_head(token[0])) return false;
	for (unsigned int i = 1; i < token.size(); i++)
		if (!ident_tail(token[i])) return false;
	return true;
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

	for (FileSystem::FileEnumerator files(FileSystem::gameDataFiles, "lang/core"); !files.Finished(); files.Next()) {
		assert(files.Current().IsFile());
		const std::string &path = files.Current().GetPath();
		if (ends_with(path, ".json")) {
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
	std::set<std::string> seen;

	ResetStringData();

	Json::Reader reader;
	Json::Value data;

	std::string filename = "lang/core/" + lang + ".json";
	RefCountedPtr<FileSystem::FileData> fd = FileSystem::gameDataFiles.ReadFile(filename);
	if (!fd) {
		if (lang != "en") {
			fprintf(stderr, "couldn't open string file '%s', falling back to 'en'\n", filename.c_str());
			filename = "lang/core/en.json";
			fd = FileSystem::gameDataFiles.ReadFile(filename);
		}
		if (!fd) {
			fprintf(stderr, "couldn't open string file '%s'\n", filename.c_str());
			return false;
		}
	}

	if (!reader.parse(fd->GetData(), fd->GetData()+fd->GetSize(), data)) {
		fprintf(stderr, "couldn't read language file '%s': %s\n", filename.c_str(), reader.getFormattedErrorMessages().c_str());
		return false;
	}

	fd.Reset();

	for (Json::Value::iterator i = data.begin(); i != data.end(); ++i) {
		const std::string token(i.key().asString());
		if (token.empty()) {
			fprintf(stderr, "%s: found empty token, skipping it\n", filename.c_str());
			continue;
		}
		if (!valid_token(token)) {
			fprintf(stderr, "%s: invalid token '%s', skipping it\n", filename.c_str(), token.c_str());
			continue;
		}

		token_map::iterator it = s_token_map.find(token);
		if (it == s_token_map.end()) {
			fprintf(stderr, "%s: unknown token '%s', skipping it\n", filename.c_str(), token.c_str());
			continue;
		}

		std::string text((*i).asString()); // XXX handle quotes, escapes
		if (text.empty()) {
			fprintf(stderr, "%s: empty value for token '%s', skipping it\n", filename.c_str(), token.c_str());
			continue;
		}

		// extracted quoted string
		if (text[0] == '"' && text[text.size()-1] == '"')
			text = text.substr(1, text.size()-2);

		// adjust for escaped newlines
		{
			std::string adjustedText;
			adjustedText.reserve(text.size());

			unsigned int ii;
			for (ii = 0; ii < text.size()-1; ii++) {
				const char *c = &text[ii];
				if (c[0] == '\\' && c[1] == 'n') {
					ii++;
					adjustedText += '\n';
				}
				else
					adjustedText += *c;
			}
			if (ii != text.size())
				adjustedText += text[ii++];
			assert(ii == text.size());
			text = adjustedText;
		}

		if (text.size() >= size_t(STRING_RECORD_SIZE))
			fprintf(stderr, "%s: text for token '%s' is too long and will be truncated\n", filename.c_str(), token.c_str());

		char *record = const_cast<char*>(it->second);
		copy_string(record, text.c_str(), text.size(), STRING_RECORD_SIZE);

		seen.insert(token);
	}

	if (seen.size() != s_token_map.size()) {
		fprintf(stderr, "string file '%s' has missing tokens:\n", filename.c_str());
		for (token_map::iterator it = s_token_map.begin(); it != s_token_map.end(); ++it)
			if (!seen.count(it->first))
				fprintf(stderr, "  %s\n", it->first.c_str());
		return false;
	}

	s_current_language = lang;

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

const std::string &GetCurrentLanguage()
{
	return s_current_language;
}

} // namespace Lang
