// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _INICONFIG_H
#define _INICONFIG_H

#include "libs.h"
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

	void SetInt(const std::string &section, const std::string &key, int val);
	void SetFloat(const std::string &section, const std::string &key, float val);
	void SetString(const std::string &section, const std::string &key, const std::string &val);

	int Int(const std::string &section, const std::string &key, int defval) const;
	float Float(const std::string &section, const std::string &key, float defval) const;
	std::string String(const std::string &section, const std::string &key, const std::string &defval) const;

	void SetInt(const std::string &key, int val) { SetInt("", key, val); }
	void SetFloat(const std::string &key, float val) { SetFloat("", key, val); }
	void SetString(const std::string &key, const std::string &val) { SetString("", key, val); }

	int Int(const std::string &key, int defval = 0) const { return Int("", key, defval); }
	float Float(const std::string &key, float defval = 0.0f) const { return Float("", key, defval); }
	std::string String(const std::string &key, const std::string &defval = std::string()) const { return String("", key, defval); }

	bool HasSection(const std::string &section) const {
		SectionMapType::const_iterator it = m_map.find(section);
		return (it != m_map.end()) && (!it->second.empty());
	}

	bool HasEntry(const std::string &section, const std::string &key) const {
		SectionMapType::const_iterator it = m_map.find(section);
		return (it != m_map.end()) && it->second.count(key);
	}
	bool HasEntry(const std::string &key) const { return HasEntry("", key); }

protected:
	typedef std::map<std::string, std::string> MapType;
	typedef std::map<std::string, MapType> SectionMapType;
	SectionMapType m_map;
};

#endif /* _INICONFIG_H */
