#ifndef _WORLDTEXTURE_H
#define _WORLDTEXTURE_H

#include "libs.h"
#include <map>
#include "graphics/Texture.h"

// various class for textures used in world drawing

// subclass for model textures. primarily allows lazy-loaded textures, where
// they aren't pulled from disk until the first call to Bind().
class ModelTexture : public Graphics::Texture {
public:
	ModelTexture(const std::string &filename, bool preload = false);

	virtual void Bind() {
		if (!IsCreated())
			Load();
		Texture::Bind();
	}

	const std::string &GetFilename() const { return m_filename; }

private:
	void Load();

	std::string m_filename;
};

// a lot like model texture, but meant for billboards, particle effects, sprites
// they are clamped and cannot be delay-loaded (they are expected to be rather small anyway)
class BillboardTexture : public Graphics::Texture {
public:
	BillboardTexture(const std::string &filename);

	//needed for LMR caching
	const std::string &GetFilename() const { return m_filename; }
private:
	std::string m_filename;
};

// cache for named world textures
class TextureCache {
public:
	~TextureCache();

	ModelTexture *GetModelTexture(const std::string &filename, bool preload = false);
	BillboardTexture *GetBillboardTexture(const std::string &filename);

private:
	typedef std::map<std::string,Graphics::Texture*> TextureCacheMap;
	TextureCacheMap m_modelTextureCache;
	TextureCacheMap m_billboardTextureCache;
};



#endif
