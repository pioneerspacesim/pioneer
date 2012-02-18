#include "Font.h"
#include "FileSystem.h"

Font::Font(FontManager &fm, const std::string &config_file) : m_fontManager(fm)
{
	RefCountedPtr<FileSystem::FileData> config_data = FileSystem::gameDataFiles.ReadFile(config_file);
	m_config.Load(*config_data);
	config_data.Reset();

	std::string filename_ttf = m_config.String("FontFile");
	if (filename_ttf.length() == 0) {
		fprintf(stderr, "'%s' does not name a FontFile to use\n", config_file.c_str());
		abort();
	}

	m_fontFileData = FileSystem::gameDataFiles.ReadFile("fonts/" + filename_ttf);
	if (! m_fontFileData) {
		fprintf(stderr, "Terrible error! Couldn't load '%s'.\n", filename_ttf.c_str());
		abort();
	}

	int err;
	if (0 != (err = FT_New_Memory_Face(
			GetFontManager().GetFreeTypeLibrary(),
			reinterpret_cast<const FT_Byte*>(m_fontFileData->GetData()),
			m_fontFileData->GetSize(), 0, &m_face))) {
		fprintf(stderr, "Terrible error! Couldn't understand '%s'; error %d.\n", filename_ttf.c_str(), err);
		abort();
	}
}

Font::~Font()
{
	FT_Done_Face(m_face);
}
