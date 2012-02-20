#ifndef _FONT_H
#define _FONT_H

#include "FontConfig.h"
#include "RefCounted.h"
#include <ft2build.h>
#include FT_FREETYPE_H

namespace FileSystem { class FileData; }

class Font : public RefCounted {
protected:
	Font(const FontConfig &fc);
	virtual ~Font();

	FT_Library GetFreeTypeLibrary() const { return m_freeTypeLibrary; }
	FontConfig GetConfig() const { return m_config; }

	// XXX is m_face even used anywhere other than during construction of derived classes?
	FT_Face m_face;
	RefCountedPtr<FileSystem::FileData> m_fontFileData;

private:
	Font(const Font &);
	Font &operator=(const Font &);

	FT_Library m_freeTypeLibrary;
	FontConfig m_config;
};

#endif
