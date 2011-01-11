#ifndef INICONFIG_H
#define INICONFIG_H

#include <map>
#include <string>

class IniConfig: private std::map<std::string, std::string> {
	public:
	void Load(const std::string &filename);
	void SetInt(const char *key, int val) {
		(*this)[key] = stringf(64, "%d", val);
	}
	void SetFloat(const char *key, float val) {
		(*this)[key] = stringf(64, "%f", val);
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
	bool Save();
	private:
	std::string filename;
};

#endif /* INICONFIG_H */
