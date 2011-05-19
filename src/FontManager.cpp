#include "FontManager.h"
#include "GuiScreen.h"

FontManager::FontManager()
{
	FT_Error err = FT_Init_FreeType(&m_library);
	if (err != 0) {
		fprintf(stderr, "Couldn't create FreeType library context (%d)\n", err);
		abort();
	}
}

FontManager::~FontManager()
{
	FT_Done_FreeType(m_library);
}

TextureFontFace *FontManager::GetFont(const std::string &name)
{
	std::map<std::string, TextureFontFace*>::iterator i = m_faces.find(name);
	if (i != m_faces.end())
		return (*i).second;

	// XXX temporary until the config stuff gets merged into this
    float scale[2];
	Gui::Screen::GetCoords2Pixels(scale);

	TextureFontFace *font = new TextureFontFace((PIONEER_DATA_DIR "/fonts/" + name + ".ttf").c_str(), int(12/scale[0]), int(12/scale[1]));
	m_faces.insert( std::make_pair(name, font) );

	return font;
}
