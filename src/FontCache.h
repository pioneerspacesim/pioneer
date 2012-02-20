#ifndef _FONTCACHE_H
#define _FONTCACHE_H

#include <map>
#include <string>
#include "RefCounted.h"
#include "TextureFont.h"
#include "VectorFont.h"

class FontCache {
public:
	FontCache() {}

	RefCountedPtr<TextureFont> GetTextureFont(const std::string &name);
	RefCountedPtr<VectorFont> GetVectorFont(const std::string &name);

private:
	FontCache(const FontCache &);
	FontCache &operator=(const FontCache &);

	std::map< std::string,RefCountedPtr<TextureFont> > m_textureFonts;
	std::map< std::string,RefCountedPtr<VectorFont> > m_vectorFonts;
};

#endif

