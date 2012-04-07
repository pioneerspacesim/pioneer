#include "Font.h"
#include "FileSystem.h"

namespace Text {

Font::Font(const FontDescriptor &descriptor) : m_descriptor(descriptor)
{
	FT_Error err = FT_Init_FreeType(&m_freeTypeLibrary);
	if (err != 0) {
		fprintf(stderr, "Couldn't create FreeType library context (%d)\n", err);
		abort();
	}

	m_fontFileData = FileSystem::gameDataFiles.ReadFile("fonts/" + m_descriptor.filename);
	if (! m_fontFileData) {
		fprintf(stderr, "Terrible error! Couldn't load '%s'.\n", m_descriptor.filename.c_str());
		abort();
	}

	if (0 != (err = FT_New_Memory_Face(m_freeTypeLibrary,
			reinterpret_cast<const FT_Byte*>(m_fontFileData->GetData()),
			m_fontFileData->GetSize(), 0, &m_face))) {
		fprintf(stderr, "Terrible error! Couldn't understand '%s'; error %d.\n", m_descriptor.filename.c_str(), err);
		abort();
	}
}

Font::~Font()
{
	FT_Done_Face(m_face);
	FT_Done_FreeType(m_freeTypeLibrary);
}

}
