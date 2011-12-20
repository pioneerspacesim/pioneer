#include "TextureCache.h"
#include "Texture.h"

TextureCache::~TextureCache()
{
	for (TextureCacheMap::iterator i = m_textureCache.begin(); i != m_textureCache.end(); ++i)
		delete (*i).second;
}

ModelTexture *TextureCache::GetTexture(const std::string &filename, bool preload)
{
	TextureCacheMap::iterator i = m_textureCache.find(filename);
	if (i != m_textureCache.end())
		return (*i).second;
	
	ModelTexture *t = new ModelTexture(filename, preload);
	m_textureCache.insert(std::make_pair(filename, t));

	return t;

}

