#ifndef _FONTMANAGER_H
#define _FONTMANAGER_H

#include <map>
#include <string>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "glfreetype.h"

class FontManager {
public:
	FontManager();
	~FontManager();

	FT_Library GetFreeTypeLibrary() { return m_library; }

	TextureFontFace *GetFont(const std::string &name);

private:
	FontManager(const FontManager &);
	FontManager &operator=(const FontManager &);

    FT_Library m_library;

    std::map<std::string, TextureFontFace*> m_faces;
};

#endif

