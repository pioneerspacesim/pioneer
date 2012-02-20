#include "FontCache.h"
#include "FontConfig.h"
#include "TextureFont.h"
#include "VectorFont.h"
#include "FileSystem.h"

static FontConfig font_config(const std::string &path) {
	RefCountedPtr<FileSystem::FileData> config_data = FileSystem::gameDataFiles.ReadFile(path);
	FontConfig fc;
	fc.Load(*config_data);
	config_data.Reset();
	return fc;
}

RefCountedPtr<TextureFont> FontCache::GetTextureFont(const std::string &name)
{
	std::map< std::string,RefCountedPtr<TextureFont> >::iterator i = m_textureFonts.find(name);
	if (i != m_textureFonts.end())
		return (*i).second;

	RefCountedPtr<TextureFont> font(new TextureFont(font_config("fonts/" + name + ".ini")));
	m_textureFonts.insert(std::pair< std::string,RefCountedPtr<TextureFont> >(name, font));

	return font;
}

RefCountedPtr<VectorFont> FontCache::GetVectorFont(const std::string &name)
{
	std::map< std::string, RefCountedPtr<VectorFont> >::iterator i = m_vectorFonts.find(name);
	if (i != m_vectorFonts.end())
		return (*i).second;

	RefCountedPtr<VectorFont> font(new VectorFont(font_config("fonts/" + name + ".ini")));
	m_vectorFonts.insert(std::pair< std::string,RefCountedPtr<VectorFont> >(name, font));

	return font;
}
