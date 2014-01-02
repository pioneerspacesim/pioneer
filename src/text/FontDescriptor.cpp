// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "FontDescriptor.h"
#include "FileSystem.h"
#include "FloatComparison.h"
#include "IniConfig.h"

namespace Text {

	FontDescriptor FontDescriptor::Load(FileSystem::FileSource &fs, const std::string &path, const std::string &lang)
	{
		return FontDescriptor::Load(fs, path, lang, 1.0f, 1.0f);
	}

	FontDescriptor FontDescriptor::Load(FileSystem::FileSource &fs, const std::string &path, const std::string &lang, float scale_x, float scale_y)
	{
		IniConfig cfg;

		cfg.Read(fs, path);

		std::string section;
		if (cfg.HasEntry(lang, "FontFile")) { section = lang; }

		const float pointSize = cfg.Float(section, "PointSize", 0.0f);
		if (!is_zero_general(pointSize))
			return FontDescriptor(cfg.String(section, "FontFile", ""), pointSize);

		return FontDescriptor(
			cfg.String(section, "FontFile", ""),
			cfg.Int(section, "PixelWidth", 12) / scale_x,
			cfg.Int(section, "PixelHeight", 12) / scale_y,
			cfg.Int(section, "Outline", 0) ? true : false,
			cfg.Float(section, "AdvanceXAdjustment", 0.0f)
		);
	}

} // namespace Text
