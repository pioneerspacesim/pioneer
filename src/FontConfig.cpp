#include "FontConfig.h"

FontConfig::FontConfig(const std::string &filename) : IniConfig(filename)
{
	// set defaults
	(*this)["PixelWidth"] = "12";
	(*this)["PixelHeight"] = "14";

	Load();
}
