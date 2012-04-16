#ifndef _TEXT_FONT_H
#define _TEXT_FONT_H

#include "FontDescriptor.h"
#include "RefCounted.h"
#include <ft2build.h>
#include FT_FREETYPE_H

namespace FileSystem { class FileData; }

namespace Text {

class Font : public RefCounted {
protected:
	Font(const FontDescriptor &descriptor);
	virtual ~Font();

	FT_Library GetFreeTypeLibrary() const { return m_freeTypeLibrary; }
	const FontDescriptor& GetDescriptor() const { return m_descriptor; }

	// XXX is m_face even used anywhere other than during construction of derived classes?
	FT_Face m_face;
	RefCountedPtr<FileSystem::FileData> m_fontFileData;

private:
	Font(const Font &);
	Font &operator=(const Font &);

	FT_Library m_freeTypeLibrary;
	FontDescriptor m_descriptor;
};

}

#endif
