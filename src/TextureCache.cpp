#include "TextureCache.h"
#include "Texture.h"

TextureCache::~TextureCache()
{
	for (TextureCacheMap::iterator i = m_modelTextureCache.begin(); i != m_modelTextureCache.end(); ++i)
		delete (*i).second;

	for (TextureCacheMap::iterator i = m_uiTextureCache.begin(); i != m_uiTextureCache.end(); ++i)
		delete (*i).second;
}

ModelTexture *TextureCache::GetModelTexture(const std::string &filename, bool preload)
{
	TextureCacheMap::iterator i = m_modelTextureCache.find(filename);
	if (i != m_modelTextureCache.end())
		return static_cast<ModelTexture*>((*i).second);
	
	ModelTexture *t = new ModelTexture(filename, preload);
	m_modelTextureCache.insert(std::make_pair(filename, t));

	return t;
}

BillboardTexture *TextureCache::GetBillboardTexture(const std::string &filename)
{
	TextureCacheMap::iterator i = m_billboardTextureCache.find(filename);
	if (i != m_billboardTextureCache.end())
		return static_cast<BillboardTexture*>((*i).second);

	BillboardTexture *t = new BillboardTexture(filename);
	m_billboardTextureCache.insert(std::make_pair(filename, t));

	return t;
}

UITexture *TextureCache::GetUITexture(const std::string &filename)
{
	TextureCacheMap::iterator i = m_uiTextureCache.find(filename);
	if (i != m_uiTextureCache.end())
		return static_cast<UITexture*>((*i).second);
	
	UITexture *t = new UITexture(filename);
	m_uiTextureCache.insert(std::make_pair(filename, t));

	return t;

}
