#include "FontCache.h"
#include "FontConfig.h"
#include "text/TextureFont.h"
#include "text/VectorFont.h"
#include "FileSystem.h"

static FontConfig font_config(const std::string &path) {
	RefCountedPtr<FileSystem::FileData> config_data = FileSystem::gameDataFiles.ReadFile(path);
	FontConfig fc;
	fc.Load(*config_data);
	config_data.Reset();
	return fc;
}

RefCountedPtr<Text::TextureFont> FontCache::GetTextureFont(const std::string &name)
{
	std::map< std::string,RefCountedPtr<Text::TextureFont> >::iterator i = m_textureFonts.find(name);
	if (i != m_textureFonts.end())
		return (*i).second;

	RefCountedPtr<Text::TextureFont> font(new Text::TextureFont(font_config("fonts/" + name + ".ini").GetDescriptor()));
	m_textureFonts.insert(std::pair< std::string,RefCountedPtr<Text::TextureFont> >(name, font));

	return font;
}

RefCountedPtr<Text::VectorFont> FontCache::GetVectorFont(const std::string &name)
{
	std::map< std::string, RefCountedPtr<Text::VectorFont> >::iterator i = m_vectorFonts.find(name);
	if (i != m_vectorFonts.end())
		return (*i).second;

	RefCountedPtr<Text::VectorFont> font(new Text::VectorFont(font_config("fonts/" + name + ".ini").GetDescriptor()));
	m_vectorFonts.insert(std::pair< std::string,RefCountedPtr<Text::VectorFont> >(name, font));

	return font;
}
