#ifndef _TEXTURECACHE_H
#define _TEXTURECACHE_H

#include "Texture.h"
#include "libs.h"
#include <map>

class TextureCache {
public:
	~TextureCache();

	Texture *GetTexture(const std::string &filename, bool preload = false);

private:
	typedef std::map<std::string,Texture*> TextureCacheMap;
	TextureCacheMap m_textureCache;
};

#endif
