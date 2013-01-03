// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _INICONFIG_H
#define _INICONFIG_H

#include "libs.h"
#include "StringF.h"
#include <map>
#include <string>

namespace FileSystem {
	class FileData;
	class FileSource;
	class FileSourceFS;
}

class IniConfig {
public:
	IniConfig() {}

	void Read(FileSystem::FileSource &fs, const std::string &path);
	void Read(const FileSystem::FileData &data);
	bool Write(FileSystem::FileSourceFS &fs, const std::string &path);

	void SetInt(const char *section, const char *key, int val) {
		m_map[section][key] = stringf("%0{d}", val);
	}
	void SetFloat(const char *section, const char *key, float val) {
		m_map[section][key] = stringf("%0{f}", val);
	}
	void SetString(const char *section, const char *key, const char *val) {
		m_map[section][key] = val;
	}
	int Int(const char *section, const char *key) {
		return atoi(m_map[section][key].c_str());
	}
	float Float(const char *section, const char *key) {
		float val;
		if (sscanf(m_map[section][key].c_str(), "%f", &val)==1) return val;
		else return 0;
	}
	std::string String(const char *section, const char *key) {
		return m_map[section][key];
	}

	void SetInt(const char *key, int val) { SetInt("", key, val); }
	void SetFloat(const char *key, float val) { SetFloat("", key, val); }
	void SetString(const char *key, const char *val) { SetString("", key, val); }

	int Int(const char *key) { return Int("", key); }
	float Float(const char *key) { return Float("", key); }
	std::string String(const char *key) { return String("", key); }

	bool HasSection(const char *section) {
		SectionMapType::const_iterator it = m_map.find(section);
		return (it != m_map.end()) && (!it->second.empty());
	}

protected:
	typedef std::map<std::string, std::string> MapType;
	typedef std::map<std::string, MapType> SectionMapType;
	SectionMapType m_map;
};

#endif /* _INICONFIG_H */
