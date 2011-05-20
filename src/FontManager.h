#ifndef _FONTMANAGER_H
#define _FONTMANAGER_H

#include <map>
#include <string>
#include <ft2build.h>
#include FT_FREETYPE_H

class TextureFont;
class VectorFont;

class FontManager {
public:
	FontManager();
	~FontManager();

	FT_Library GetFreeTypeLibrary() { return m_library; }

	TextureFont *GetTextureFont(const std::string &name);
	VectorFont  *GetVectorFont(const std::string &name);

private:
	FontManager(const FontManager &);
	FontManager &operator=(const FontManager &);

	FT_Library m_library;

	std::map<std::string, TextureFont*> m_textureFonts;
	std::map<std::string, VectorFont*> m_vectorFonts;
};

#endif

