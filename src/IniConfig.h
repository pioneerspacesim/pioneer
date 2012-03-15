#ifndef _INICONFIG_H
#define _INICONFIG_H

#include "libs.h"
#include "StringF.h"
#include <map>
#include <string>

namespace FileSystem { class FileData; }

class IniConfig {
public:
	void Load();
	void Load(const FileSystem::FileData &data);
	bool Save();

	void SetInt(const char *key, int val) {
		m_map[key] = stringf("%0{d}", val);
	}
	void SetFloat(const char *key, float val) {
		m_map[key] = stringf("%0{f}", val);
	}
	void SetString(const char *key, const char *val) {
		m_map[key] = val;
	}
	int Int(const char *key) {
		return atoi(m_map[key].c_str());
	}
	float Float(const char *key) {
		float val;
		if (sscanf(m_map[key].c_str(), "%f", &val)==1) return val;
		else return 0;
	}
	std::string String(const char *key) {
		return m_map[key];
	}

	const std::string &GetFilename() const { return m_filename; }

protected:
	IniConfig() {}
	explicit IniConfig(const std::string &filename): m_filename(filename) {}

	std::map<std::string, std::string> m_map;

private:
	std::string m_filename;
};

#endif /* _INICONFIG_H */
