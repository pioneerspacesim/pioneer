#ifndef _FONT_H
#define _FONT_H

#include "FontManager.h"
#include "FontConfig.h"
#include <ft2build.h>
#include FT_FREETYPE_H

class Font
{
protected:
	Font(const FontConfig &fc);
	virtual ~Font();

	FT_Library GetFreeTypeLibrary() const { return m_freeTypeLibrary; }
	FontConfig GetConfig() const { return m_config; }

	FT_Face m_face;

private:
	FT_Library m_freeTypeLibrary;
	FontConfig m_config;
};

#endif
