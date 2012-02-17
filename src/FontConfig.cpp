#include "FontConfig.h"

FontConfig::FontConfig()
{
	SetDefaults();
}

FontConfig::FontConfig(const std::string &filename) : IniConfig(filename)
{
	SetDefaults();
	Load();
}

void FontConfig::SetDefaults()
{
	// set defaults
	m_map["PixelWidth"] = "12";
	m_map["PixelHeight"] = "12";
	m_map["AdvanceXAdjustment"] = "0";
}
