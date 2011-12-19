#include "TextureManager.h"
#include "Texture.h"

TextureManager::~TextureManager()
{
	for (TextureCacheMap::iterator i = m_textureCache.begin(); i != m_textureCache.end(); ++i)
		delete (*i).second;
}

Texture *TextureManager::GetTexture(const std::string &filename, bool preload)
{
	TextureCacheMap::iterator i = m_textureCache.find(filename);
	if (i != m_textureCache.end())
		return (*i).second;
	
	Texture *t = new Texture(filename, preload);
	m_textureCache.insert(std::make_pair(filename, t));

	return t;

}

