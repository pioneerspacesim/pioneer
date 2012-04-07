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

	int pixelWidth = Int("PixelWidth");
	if (pixelWidth) {
		int pixelHeight = Int("PixelHeight");
		bool outline = Int("Outline") ? true : false;
		float advanceXAdjustment = Float("AdvanceXAdjustment");
		return Text::FontDescriptor(filename, pixelWidth, pixelHeight, outline, advanceXAdjustment);
	}

	float pointSize = Float("PointSize");
	return Text::FontDescriptor(filename, pointSize);
}
