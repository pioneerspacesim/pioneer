#ifndef _TEXTURECACHE_H
#define _TEXTURECACHE_H

#include "Texture.h"
#include "libs.h"
#include <map>

class TextureCache {
public:
	~TextureCache();

	ModelTexture *GetModelTexture(const std::string &filename, bool preload = false);
	UITexture *GetUITexture(const std::string &filename);

private:
	typedef std::map<std::string,Texture*> TextureCacheMap;
	TextureCacheMap m_modelTextureCache;
	TextureCacheMap m_uiTextureCache;
};

#endif
