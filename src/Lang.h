// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LANG_H
#define _LANG_H

#include "IterationProxy.h"
#include <SDL_stdinc.h>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace Lang {

	class Resource {
	public:
		using StringMap = std::map<std::string, std::string, std::less<>>;

		Resource(std::string_view name, std::string_view langCode) :
			m_name(name),
			m_langCode(langCode),
			m_loaded(false) {}

		std::string_view GetName() const { return m_name; }
		std::string_view GetLangCode() const { return m_langCode; }

		bool Load();

		Uint32 GetNumStrings() const { return static_cast<Uint32>(m_strings.size()); }

		const std::string &Get(std::string_view token) const;

		static std::vector<std::string> GetAvailableLanguages(std::string_view resourceName);

		IterationProxy<StringMap> GetStrings() { return MakeIterationProxy(m_strings); }
		const IterationProxy<const StringMap> GetStrings() const { return MakeIterationProxy(m_strings); }

	private:
		std::string m_name;
		std::string m_langCode;

		bool m_loaded;

		StringMap m_strings;
	};

// declare all strings
#define DECLARE_STRING(x) extern char x[];
#include "LangStrings.inc.h"
#undef DECLARE_STRING

	void MakeCore(Resource &res);
	const Resource &GetCore();

	Resource &GetResource(std::string_view name, std::string_view langCode);

} // namespace Lang

#endif
