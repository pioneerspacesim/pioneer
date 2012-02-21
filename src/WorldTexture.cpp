#include "WorldTexture.h"
#include <cassert>

ModelTexture::ModelTexture(const std::string &filename, bool preload) :
	Texture(GL_TEXTURE_2D, Format(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE), REPEAT, LINEAR, true),
	m_filename(filename)
{
	if (preload)
		Load();
}

void ModelTexture::Load()
{
	assert(!IsCreated());
	CreateFromFile(m_filename, false);
}


BillboardTexture::BillboardTexture(const std::string &filename) :
	Texture(GL_TEXTURE_2D, Format(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE), CLAMP, LINEAR, true),
	m_filename(filename)
{
	CreateFromFile(filename, false);
}


TextureCache::~TextureCache()
{
	for (TextureCacheMap::iterator i = m_modelTextureCache.begin(); i != m_modelTextureCache.end(); ++i)
		delete (*i).second;

	for (TextureCacheMap::iterator i = m_billboardTextureCache.begin(); i != m_billboardTextureCache.end(); ++i)
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
