// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "FontDescriptor.h"
#include "FileSystem.h"
#include "FloatComparison.h"
#include "utils.h"
#include "json/json.h"

namespace Text {

	FontDescriptor FontDescriptor::Load(FileSystem::FileSource &fs, const std::string &path, const std::string &lang)
	{
		return FontDescriptor::Load(fs, path, lang, 1.0f, 1.0f);
	}

	FontDescriptor FontDescriptor::Load(FileSystem::FileSource &fs, const std::string &path, const std::string &lang, float scale_x, float scale_y)
	{
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

		Json::Value langData = data.get(lang, Json::nullValue);
		if (langData.isNull())
			langData = data["default"];
		if (!langData.isObject()) {
			Output("couldn't parse font file '%s': no lang section for '%s'\n", path.c_str(), lang.c_str());
			abort();
		}

		return FontDescriptor(
			langData.get("FontFile", "").asString(),
			langData.get("PixelWidth", 12).asInt() / scale_x,
			langData.get("PixelHeight", 12).asInt() / scale_y,
			langData.get("Outline", false).asBool(),
			langData.get("AdvanceXAdjustment",  0.0f).asFloat()
		);
	}

} // namespace Text
