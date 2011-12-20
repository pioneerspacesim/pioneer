#ifndef _TEXTURECACHE_H
#define _TEXTURECACHE_H

#include "Texture.h"
#include "libs.h"
#include <map>

class TextureCache {
public:
	~TextureCache();

	ModelTexture *GetTexture(const std::string &filename, bool preload = false);

private:
	typedef std::map<std::string,ModelTexture*> TextureCacheMap;
	TextureCacheMap m_textureCache;
};

#endif
