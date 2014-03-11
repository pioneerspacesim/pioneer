// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "FontConfig.h"
#include "FileSystem.h"
#include "json/json.h"
#include "utils.h"

namespace Text {

FontConfig::FontConfig(const std::string &name, float scaleX, float scaleY)
{
	const std::string path("fonts/" + name + ".json");

	Json::Reader reader;
	Json::Value data;

	RefCountedPtr<FileSystem::FileData> fd = FileSystem::gameDataFiles.ReadFile(path);
	if (!fd) {
		Output("couldn't open font file '%s'\n", path.c_str());
		abort();
	}

	if (!reader.parse(fd->GetData(), fd->GetData()+fd->GetSize(), data)) {
		Output("couldn't read font file '%s': %s\n", path.c_str(), reader.getFormattedErrorMessages().c_str());
		abort();
	}

	fd.Reset();

	m_name = data.get("name", Json::Value("unknown")).asString();
	m_outline = data.get("outline", Json::Value(false)).asBool();

	Json::Value faces = data.get("faces", Json::arrayValue);
	for (Json::Value::iterator i = faces.begin(); i != faces.end(); ++i) {
		const std::string &fontFile = (*i).get("fontFile", Json::nullValue).asString();
		const int pixelWidth = 1.0f/scaleX * (*i).get("pixelWidth", Json::Value(14)).asInt();
		const int pixelHeight = 1.0f/scaleY * (*i).get("pixelWidth", Json::Value(14)).asInt();
		const float advanceXAdjustment = (*i).get("advanceXAdjustment", Json::Value(0.0f)).asFloat();

		Json::Value ranges = (*i).get("ranges", Json::arrayValue);
		if (ranges.empty())
			m_faces.push_back(Face(fontFile, pixelWidth, pixelHeight, advanceXAdjustment, 0x000000, 0x1fffff));
		else {
			for (Json::Value::iterator j = ranges.begin(); j != ranges.end(); ++j) {
				Uint32 rangeMin = 0x000000, rangeMax = 0x1fffff;
				const std::string rangeMinStr = (*j).get(Json::ArrayIndex(0), Json::Value("0x000000")).asString();
				const std::string rangeMaxStr = (*j).get(Json::ArrayIndex(1), Json::Value("0x1fffff")).asString();
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

}
