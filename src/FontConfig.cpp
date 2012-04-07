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

Text::FontDescriptor FontConfig::GetDescriptor()
{
	std::string filename(String("FontFile"));

	float pointSize = Float("PointSize");
    if (!is_zero_general(pointSize))
		return Text::FontDescriptor(filename, pointSize);

	int pixelWidth = Int("PixelWidth");
	int pixelHeight = Int("PixelHeight");
	bool outline = Int("Outline") ? true : false;
	float advanceXAdjustment = Float("AdvanceXAdjustment");
	return Text::FontDescriptor(filename, pixelWidth, pixelHeight, outline, advanceXAdjustment);
}
