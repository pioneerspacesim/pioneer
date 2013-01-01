// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "FontCache.h"
#include "text/TextureFont.h"
#include "text/VectorFont.h"
#include "FileSystem.h"
#include "gui/GuiScreen.h"

RefCountedPtr<Text::TextureFont> FontCache::GetTextureFont(const std::string &name)
{
	std::map< std::string,RefCountedPtr<Text::TextureFont> >::iterator i = m_textureFonts.find(name);
	if (i != m_textureFonts.end())
		return (*i).second;

	float scale[2];
	Gui::Screen::GetCoords2Pixels(scale);

	const Text::FontDescriptor desc =
		Text::FontDescriptor::Load(FileSystem::gameDataFiles, "fonts/" + name + ".ini", scale[0], scale[1]);

	RefCountedPtr<Text::TextureFont> font(new Text::TextureFont(desc, Gui::Screen::GetRenderer()));
	m_textureFonts.insert(std::pair< std::string,RefCountedPtr<Text::TextureFont> >(name, font));

	return font;
}

RefCountedPtr<Text::VectorFont> FontCache::GetVectorFont(const std::string &name)
{
	std::map< std::string, RefCountedPtr<Text::VectorFont> >::iterator i = m_vectorFonts.find(name);
	if (i != m_vectorFonts.end())
		return (*i).second;

	const Text::FontDescriptor desc =
		Text::FontDescriptor::Load(FileSystem::gameDataFiles, "fonts/" + name + ".ini");
	RefCountedPtr<Text::VectorFont> font(new Text::VectorFont(desc));
	m_vectorFonts.insert(std::pair< std::string,RefCountedPtr<Text::VectorFont> >(name, font));

	return font;
}
