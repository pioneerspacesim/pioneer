#ifndef _FONTMANAGER_H
#define _FONTMANAGER_H

#include <ft2build.h>
#include FT_FREETYPE_H

class FontManager {
public:
	FontManager();
	~FontManager();

	FT_Library GetFreeTypeLibrary() { return m_library; }

private:
	FontManager(const FontManager &);
	FontManager &operator=(const FontManager &);

    FT_Library m_library;
};

#endif

