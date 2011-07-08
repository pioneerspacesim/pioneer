#ifndef _FONT_H
#define _FONT_H

#include "FontManager.h"
#include "FontConfig.h"

class Font
{
protected:
	Font(FontManager &fm, const std::string &config_file) : m_fontManager(fm), m_config(FontConfig(config_file)) {}

	FontManager &GetFontManager() { return m_fontManager; }
	FontConfig &GetConfig() { return m_config; }

	FT_Face m_face;

private:
	FontManager &m_fontManager;
	FontConfig m_config;
};

#endif
