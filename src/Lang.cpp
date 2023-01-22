// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Lang.h"
#include "FileSystem.h"
#include "JsonUtils.h"
#include "StringRange.h"
#include "core/Log.h"
#include "libs.h"
#include "text/TextSupport.h"
#include "utils.h"
#include <map>
#include <set>

namespace Lang {

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

	bool Resource::Load()
	{
		if (m_loaded)
			return true;

		std::string filename = "lang/" + m_name + "/" + m_langCode + ".json";
		Json data = JsonUtils::LoadJsonDataFile(filename);
		if (data.is_null()) {
			Log::Warning("couldn't read language file '{}'\n", filename.c_str());
			return false;
		}

		for (Json::iterator i = data.begin(); i != data.end(); ++i) {
			const std::string token = i.key();
			if (token.empty()) {
				Log::Info("{}: found empty token, skipping it\n", filename.c_str());
				continue;
			}
			if (!valid_token(token)) {
				Log::Info("{}: invalid token '{}', skipping it\n", filename.c_str(), token.c_str());
				continue;
			}

			Json message = i.value()["message"];
			if (message.is_null()) {
				Log::Info("{}: no 'message' key for token '{}', skipping it\n", filename.c_str(), token.c_str());
				continue;
			}

			if (!message.is_string()) {
				Log::Info("{}: value for token '{}' is not a string, skipping it\n", filename.c_str(), token.c_str());
				continue;
			}

			std::string text = message;
			if (text.empty()) {
				Log::Info("{}: empty value for token '{}', skipping it\n", filename.c_str(), token.c_str());
				continue;
			}

			// extracted quoted string
			if (text[0] == '"' && text[text.size() - 1] == '"')
				text = text.substr(1, text.size() - 2);

			// adjust for escaped newlines
			{
				std::string adjustedText;
				adjustedText.reserve(text.size());

				unsigned int ii;
				for (ii = 0; ii < text.size() - 1; ii++) {
					const char *c = &text[ii];
					if (c[0] == '\\' && c[1] == 'n') {
						ii++;
						adjustedText += '\n';
					} else
						adjustedText += *c;
				}
				if (ii != text.size())
					adjustedText += text[ii++];
				assert(ii == text.size());
				text = adjustedText;
			}

			m_strings[token] = text;
		}

		m_loaded = true;
		return true;
	}

	const std::string &Resource::Get(const std::string &token) const
	{
		std::map<std::string, std::string>::const_iterator i = m_strings.find(token);
		if (i == m_strings.end()) {
			static const std::string empty;
			return empty;
		}
		return (*i).second;
	}

	std::vector<std::string> Resource::GetAvailableLanguages(const std::string &resourceName)
	{
		std::vector<std::string> languages;

		for (FileSystem::FileEnumerator files(FileSystem::gameDataFiles, "lang/" + resourceName); !files.Finished(); files.Next()) {
			assert(files.Current().IsFile());
			const std::string &path = files.Current().GetPath();
			if (ends_with_ci(path, ".json")) {
				const std::string name = files.Current().GetName();
				languages.push_back(name.substr(0, name.size() - 5));
			}
		}

		return languages;
	}

	// XXX we're allocating a KB for each translatable string
	// that's... not very nice (though I guess it "doesn't matter" with virtual memory and multi-GB of RAM)
	static const int STRING_RECORD_SIZE = 1024;
#define DECLARE_STRING(x) char x[STRING_RECORD_SIZE];
#include "LangStrings.inc.h"
#undef DECLARE_STRING

	//
	// declaring value type as const char* so that we can give out a const reference to the real
	// token map without allowing external code to modify token text
	// unfortunately, this means we don't have write access internally either,
	// so we have to const_cast<> to initialise the token values.
	// this could be avoided by using a custom class for the value type
	// (or std::string, but then we'd be changing the way translated text is stored)
	typedef std::map<std::string, const char *> token_map;
	static token_map s_token_map;

	static struct init_string_helper_class {
		init_string_helper_class()
		{
#define DECLARE_STRING(x)                            \
	s_token_map.insert(std::make_pair(#x, Lang::x)); \
	Lang::x[0] = '\0';
#include "LangStrings.inc.h"
#undef DECLARE_STRING
		}
	} init_string_helper;

	static void copy_string(char *buf, const char *str, size_t strsize, size_t bufsize)
	{
		size_t sz = std::min(strsize, bufsize - 1);
		memcpy(buf, str, sz);
		buf[sz] = '\0';
	}

	static Resource s_coreResource("core", "<unknown>");

	void MakeCore(Resource &res)
	{
		assert(res.GetName() == "core");

		res.Load();

		for (token_map::iterator i = s_token_map.begin(); i != s_token_map.end(); ++i) {
			const std::string &token = i->first;
			std::string text = res.Get(token);

			if (text.empty()) {
				Log::Info("{}/{}: token '{}' not found\n", res.GetName().c_str(), res.GetLangCode().c_str(), token.c_str());
				text = token;
			}

			if (text.size() > size_t(STRING_RECORD_SIZE)) {
				Log::Info("{}/{}: text for token '{}' is too long and will be truncated\n", res.GetName().c_str(), res.GetLangCode().c_str(), token.c_str());
				text.resize(STRING_RECORD_SIZE);
			}

			// const_cast so we can set the string, see above
			char *record = const_cast<char *>(i->second);
			copy_string(record, text.c_str(), text.size(), STRING_RECORD_SIZE);
		}

		s_coreResource = res;
	}

	const Resource &GetCore()
	{
		return s_coreResource;
	}

	static std::map<std::string, Resource> m_cachedResources;

	Resource GetResource(const std::string &name, const std::string &langCode)
	{
		auto key = name + ":" + langCode;

		auto i = m_cachedResources.find(key);
		if (i != m_cachedResources.end())
			return i->second;

		Lang::Resource res = Lang::Resource(name, langCode);
		bool loaded = res.Load();
		if (!loaded) {
			if (langCode != "en") {
				Log::Warning("couldn't load language resource {}/{}, trying {}/en\n", name.c_str(), langCode.c_str(), name.c_str());
				res = Lang::Resource(name, "en");
				loaded = res.Load();
				key = name + ":" + "en";
			}
			if (!loaded)
				Log::Warning("couldn't load language resource {}/en\n", name.c_str());
		}

		if (loaded)
			m_cachedResources.insert(std::make_pair(key, res));

		return res;
	}

} // namespace Lang
