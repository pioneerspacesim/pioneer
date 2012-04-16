#include "FontCache.h"
#include "FontConfig.h"
#include "text/TextureFont.h"
#include "text/VectorFont.h"
#include "FileSystem.h"
#include "gui/GuiScreen.h"

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
	
	float scale[2];
	Gui::Screen::GetCoords2Pixels(scale);

	FontConfig fc = font_config("fonts/" + name + ".ini");
	fc.SetInt("PixelWidth", fc.Int("PixelWidth") / scale[0]);
	fc.SetInt("PixelHeight", fc.Int("PixelHeight") / scale[1]);

	RefCountedPtr<Text::TextureFont> font(new Text::TextureFont(fc.GetDescriptor(), Gui::Screen::GetRenderer()));
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
