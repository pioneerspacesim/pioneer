#include "FontConfig.h"

FontConfig::FontConfig(const std::string &filename) : IniConfig(filename)
{
	// set defaults
	(*this)["PixelWidth"] = "12";
	(*this)["PixelHeight"] = "12";
	(*this)["AdvanceXAdjustment"] = "0";

	Load();
}
