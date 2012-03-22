#include "Font.h"
#include "FileSystem.h"

Font::Font(const FontConfig &fc) : m_config(fc)
{
	FT_Error err = FT_Init_FreeType(&m_freeTypeLibrary);
	if (err != 0) {
		fprintf(stderr, "Couldn't create FreeType library context (%d)\n", err);
		abort();
	}

	std::string filename_ttf = m_config.String("FontFile");
	if (filename_ttf.length() == 0) {
		fprintf(stderr, "'%s' does not name a FontFile to use\n", m_config.GetFilename().c_str());
		abort();
	}

	m_fontFileData = FileSystem::gameDataFiles.ReadFile("fonts/" + filename_ttf);
	if (! m_fontFileData) {
		fprintf(stderr, "Terrible error! Couldn't load '%s'.\n", filename_ttf.c_str());
		abort();
	}

	if (0 != (err = FT_New_Memory_Face(m_freeTypeLibrary,
			reinterpret_cast<const FT_Byte*>(m_fontFileData->GetData()),
			m_fontFileData->GetSize(), 0, &m_face))) {
		fprintf(stderr, "Terrible error! Couldn't understand '%s'; error %d.\n", filename_ttf.c_str(), err);
		abort();
	}
}

Font::~Font()
{
	FT_Done_Face(m_face);
	FT_Done_FreeType(m_freeTypeLibrary);
}
