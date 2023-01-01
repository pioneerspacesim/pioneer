// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LANG_H
#define _LANG_H

#include "IterationProxy.h"
#include <SDL_stdinc.h>
#include <map>
#include <string>
#include <vector>

namespace Lang {

	class Resource {
	public:
		Resource(const std::string &name, const std::string &langCode) :
			m_name(name),
			m_langCode(langCode),
			m_loaded(false) {}

		const std::string &GetName() const { return m_name; }
		const std::string &GetLangCode() const { return m_langCode; }

		bool Load();

		Uint32 GetNumStrings() const { return static_cast<Uint32>(m_strings.size()); }

		const std::string &Get(const std::string &token) const;

		static std::vector<std::string> GetAvailableLanguages(const std::string &resourceName);

		IterationProxy<std::map<std::string, std::string>> GetStrings() { return MakeIterationProxy(m_strings); }
		const IterationProxy<const std::map<std::string, std::string>> GetStrings() const { return MakeIterationProxy(m_strings); }

	private:
		std::string m_name;
		std::string m_langCode;

		bool m_loaded;

		std::map<std::string, std::string> m_strings;
	};

// declare all strings
#define DECLARE_STRING(x) extern char x[];
#include "LangStrings.inc.h"
#undef DECLARE_STRING

	void MakeCore(Resource &res);
	const Resource &GetCore();

	Resource GetResource(const std::string &name, const std::string &langCode);

} // namespace Lang

#endif
