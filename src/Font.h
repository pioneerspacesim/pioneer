#ifndef _FONT_H
#define _FONT_H

#include "FontManager.h"
#include "FontConfig.h"
#include "RefCounted.h"

namespace FileSystem { class FileData; }

class Font
{
protected:
	Font(FontManager &fm, const std::string &config_file);
	~Font();

	FontManager &GetFontManager() { return m_fontManager; }
	FontConfig &GetConfig() { return m_config; }

	// XXX is m_face even used anywhere other than during construction of derived classes?
	FT_Face m_face;
	RefCountedPtr<FileSystem::FileData> m_fontFileData;

private:
	FontManager &m_fontManager;
	FontConfig m_config;
};

#endif
