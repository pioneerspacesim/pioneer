#include "libs.h"
#include "IniConfig.h"

void IniConfig::Load()
{
	FILE *f = fopen(m_filename.c_str(), "r");
	if (!f) return;
	
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
}

bool IniConfig::Save()
{
	FILE *f = fopen(m_filename.c_str(), "w");
	if (!f)
		// XXX do something useful here
		return false;

	for (std::map<std::string, std::string>::const_iterator i = begin(); i!=end(); ++i) {
		if ((*i).second != "") fprintf(f, "%s=%s\n", (*i).first.c_str(), (*i).second.c_str());
	}

	fclose(f);

	return true;
}
