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
	(*this)["PixelWidth"] = "12";
	(*this)["PixelHeight"] = "12";
	(*this)["AdvanceXAdjustment"] = "0";
}
