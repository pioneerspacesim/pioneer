#ifndef _FONT_H
#define _FONT_H

#include "FontManager.h"
#include "FontConfig.h"

class Font
{
protected:
	Font(FontManager &fm, const FontConfig &fc) : m_fontManager(fm), m_config(fc) {}

	FontManager &GetFontManager() { return m_fontManager; }
	FontConfig &GetConfig() { return m_config; }

	FT_Face m_face;

private:
	FontManager &m_fontManager;
	FontConfig m_config;
};

#endif
