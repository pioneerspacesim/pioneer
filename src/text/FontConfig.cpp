// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "FontConfig.h"
#include "FileSystem.h"
#include "JsonUtils.h"
#include "utils.h"

namespace Text {

	FontConfig::FontConfig(const std::string &name, float scaleX, float scaleY)
	{
		const std::string path("fonts/" + name + ".json");
		Json data = JsonUtils::LoadJsonFile(path, FileSystem::gameDataFiles);
		if (data.is_null()) {
			Output("couldn't read font file '%s'\n", path.c_str());
			abort();
		}

		m_name = data.value("name", "unknown");
		m_outline = data.value("outline", false);

		Json faces = data.value("faces", Json::array());
		for (auto &i : faces) {
			const std::string &fontFile = i.value("fontFile", "");
			const int pixelWidth = 1.0f / scaleX * i.value("pixelWidth", 14);
			const int pixelHeight = 1.0f / scaleY * i.value("pixelWidth", 14);
			const float advanceXAdjustment = i.value("advanceXAdjustment", 0.0f);

			Json ranges = i.value("ranges", Json::array());
			if (ranges.empty())
				m_faces.push_back(Face(fontFile, pixelWidth, pixelHeight, advanceXAdjustment, 0x000000, 0x1fffff));
			else {
				for (auto &j : ranges) {
					Uint32 rangeMin = 0x000000, rangeMax = 0x1fffff;
					const std::string rangeMinStr = j[0].is_string() ? j[0] : "0x000000";
					const std::string rangeMaxStr = j[1].is_string() ? j[1] : "0x1fffff";
					sscanf(rangeMinStr.c_str(), "%x", &rangeMin);
					sscanf(rangeMaxStr.c_str(), "%x", &rangeMax);
					m_faces.push_back(Face(fontFile, pixelWidth, pixelHeight, advanceXAdjustment, rangeMin, rangeMax));
				}
			}
		}
	}

	const FontConfig::Face &FontConfig::GetFaceForCodePoint(Uint32 cp)
	{
		// XXX naive. map and custom comparator would be better
		auto best = m_faces.end();
		for (auto i = m_faces.begin(); i != m_faces.end(); ++i) {
			if (cp < (*i).rangeMin || cp > (*i).rangeMax)
				continue;
			if (best == m_faces.end()) {
				best = i;
				continue;
			}
			if ((*i).rangeMax - (*i).rangeMin < (*best).rangeMax - (*best).rangeMin) {
				best = i;
				continue;
			}
		}
		return *best;
	}

} // namespace Text
