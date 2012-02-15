#ifndef _INICONFIG_H
#define _INICONFIG_H

#include "libs.h"
#include "StringF.h"
#include <map>
#include <string>

class IniConfig: protected std::map<std::string, std::string> {
public:
	void Load();
	bool Save();

	void SetInt(const char *key, int val) {
		(*this)[key] = stringf("%0{d}", val);
	}
	void SetFloat(const char *key, float val) {
		(*this)[key] = stringf("%0{f}", val);
	}
	void SetString(const char *key, const char *val) {
		(*this)[key] = val;
	}
	int Int(const char *key) {
		return atoi((*this)[key].c_str());
	}
	float Float(const char *key) {
		float val;
		if (sscanf((*this)[key].c_str(), "%f", &val)==1) return val;
		else return 0;
	}
	std::string String(const char *key) {
		return (*this)[key];
	}

	const std::string &GetFilename() const { return m_filename; }

protected:
	IniConfig(const std::string &filename) : m_filename(filename) {}

private:
	std::string m_filename;
};

#endif /* _INICONFIG_H */
