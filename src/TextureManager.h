#ifndef _TEXTUREMANAGER_H
#define _TEXTUREMANAGER_H

#include "Texture.h"
#include "libs.h"
#include <map>

class TextureManager {
public:
	~TextureManager();

	Texture *GetTexture(const std::string &filename, bool preload = false);

private:
	typedef std::map<std::string,Texture*> TextureCacheMap;
	TextureCacheMap m_textureCache;
};

#endif
