#ifndef _TEXTURES_H
#define _TEXTURES_H

#include <string>
#include <map>

class Textures {
public:
	static unsigned int Load(const std::string&);
	static void Init();
	static void FreeAll();
private:
	static std::map<std::string, unsigned int> m_textures;
};

#endif
