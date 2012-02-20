#include "FontCache.h"
#include "FontConfig.h"
#include "TextureFont.h"
#include "VectorFont.h"

RefCountedPtr<TextureFont> FontCache::GetTextureFont(const std::string &name)
{
	std::map< std::string,RefCountedPtr<TextureFont> >::iterator i = m_textureFonts.find(name);
	if (i != m_textureFonts.end())
		return (*i).second;

	RefCountedPtr<TextureFont> font(new TextureFont(FontConfig(PIONEER_DATA_DIR "/fonts/" + name + ".ini")));
	m_textureFonts.insert(std::pair< std::string,RefCountedPtr<TextureFont> >(name, font));

	return font;
}

RefCountedPtr<VectorFont> FontCache::GetVectorFont(const std::string &name)
{
	std::map< std::string, RefCountedPtr<VectorFont> >::iterator i = m_vectorFonts.find(name);
	if (i != m_vectorFonts.end())
		return (*i).second;

	RefCountedPtr<VectorFont> font(new VectorFont(FontConfig(PIONEER_DATA_DIR "/fonts/" + name + ".ini")));
	m_vectorFonts.insert(std::pair< std::string,RefCountedPtr<VectorFont> >(name, font));

	return font;
}
