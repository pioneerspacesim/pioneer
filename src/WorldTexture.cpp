#include "WorldTexture.h"
#include <cassert>

ModelTexture::ModelTexture(const std::string &filename, bool preload) :
	Texture(Texture::TARGET_2D, Format(Format::INTERNAL_RGBA, Format::DATA_RGBA, Format::DATA_UNSIGNED_BYTE), Options(Options::REPEAT, Options::LINEAR, true)),
	m_filename(filename)
{
	if (preload)
		Load();
}

static inline Uint32 ceil_pow2(Uint32 v) {
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}

void ModelTexture::Load()
{
	assert(!IsCreated());
	CreateFromFile(m_filename, false);
	if (GetWidth() != ceil_pow2(GetWidth()) || GetHeight() != ceil_pow2(GetHeight()))
		fprintf(stderr, "WARNING: texture '%s' is not power-of-two and may not display correctly\n", m_filename.c_str());
}


BillboardTexture::BillboardTexture(const std::string &filename) :
	Texture(Texture::TARGET_2D, Format(Format::INTERNAL_RGBA, Format::DATA_RGBA, Format::DATA_UNSIGNED_BYTE), Options(Options::REPEAT, Options::LINEAR, true)),
	m_filename(filename)
{
	CreateFromFile(filename, true, false);
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
