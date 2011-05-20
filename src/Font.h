#ifndef _FONT_H
#define _FONT_H

#include "FontManager.h"

class Font
{
protected:
	Font(FontManager &fm) : m_fontManager(fm) {}

	FontManager &GetFontManager() { return m_fontManager; }

	FT_Face m_face;

private:
	FontManager &m_fontManager;
};

#endif
