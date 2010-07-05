#include "libs.h"
#include "IniConfig.h"

void IniConfig::Load(const std::string &filename)
{
	this->clear();
	this->filename = filename;
	FILE *f = fopen(filename.c_str(), "r");
	if (f) {
		char buf[1024];
		while (fgets(buf, sizeof(buf), f)) {
			if (buf[0] == '#') continue;
			char *sep = strchr(buf, '=');
			char *kend = sep;
			if (!sep) continue;
			*sep = 0;
			// strip whitespace
			while (isspace(*(--kend))) *kend = 0;
			while (isspace(*(++sep))) *sep = 0;
			// snip \r, \n
			char *vend = sep;
			while (*(++vend)) if ((*vend == '\r') || (*vend == '\n')) { *vend = 0; break; }
			std::string key = std::string(buf);
			std::string val = std::string(sep);
			(*this)[key] = val;
		}
		fclose(f);
	} else {
		// set defaults
		(*this)["DisableShaders"] = "0";
		(*this)["DisableSound"] = "0";
		(*this)["StartFullscreen"] = "1";
		(*this)["ScrWidth"] = "0";
		(*this)["ScrHeight"] = "0";
		(*this)["DetailCities"] = "1";
		(*this)["DetailPlanets"] = "1";
	}
}

bool IniConfig::Save()
{
	FILE *f = fopen(filename.c_str(), "w");
	if (!f) {
		return false;
	} else {
		for (std::map<std::string, std::string>::const_iterator i = begin(); i!=end(); ++i) {
			fprintf(f, "%s=%s\n", (*i).first.c_str(), (*i).second.c_str());
		}
		fclose(f);
		return true;
	}
}
