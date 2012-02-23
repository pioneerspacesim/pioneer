#include "Font.h"

Font::Font(const FontConfig &fc) : m_config(fc)
{
	FT_Error err = FT_Init_FreeType(&m_freeTypeLibrary);
	if (err != 0) {
		fprintf(stderr, "Couldn't create FreeType library context (%d)\n", err);
		abort();
	}
}

Font::~Font()
{
	FT_Done_FreeType(m_freeTypeLibrary);
}
