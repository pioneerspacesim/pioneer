// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "FontCache.h"
#include "text/TextureFont.h"
#include "FileSystem.h"
#include "gui/GuiScreen.h"
#include "Lang.h"

RefCountedPtr<Text::TextureFont> FontCache::GetTextureFont(const std::string &name)
{
	std::map< std::string,RefCountedPtr<Text::TextureFont> >::iterator i = m_textureFonts.find(name);
	if (i != m_textureFonts.end())
		return (*i).second;

	float scale[2];
	Gui::Screen::GetCoords2Pixels(scale);

	// XXX reload the font on lang change
	const std::string &lang = Lang::GetCore().GetLangCode();

	const Text::FontDescriptor desc =
		Text::FontDescriptor::Load(FileSystem::gameDataFiles, "fonts/" + name + ".ini", lang, scale[0], scale[1]);

	RefCountedPtr<Text::TextureFont> font(new Text::TextureFont(desc, Gui::Screen::GetRenderer()));
	m_textureFonts.insert(std::pair< std::string,RefCountedPtr<Text::TextureFont> >(name, font));

	return font;
}
