// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "FontDescriptor.h"
#include "FileSystem.h"
#include "FloatComparison.h"
#include "IniConfig.h"

namespace Text {

	FontDescriptor FontDescriptor::Load(FileSystem::FileSource &fs, const std::string &path)
	{
		return FontDescriptor::Load(fs, path, 1.0f, 1.0f);
	}

	FontDescriptor FontDescriptor::Load(FileSystem::FileSource &fs, const std::string &path, float scale_x, float scale_y)
	{
		IniConfig cfg;
		// set defaults
		cfg.SetInt("PixelWidth", 12);
		cfg.SetInt("PixelHeight", 12);
		cfg.SetInt("AdvanceXAdjustment", 0);
		cfg.Read(fs, path);

		const float pointSize = cfg.Float("PointSize");
		if (!is_zero_general(pointSize))
			return FontDescriptor(cfg.String("FontFile"), pointSize);

		return FontDescriptor(
			cfg.String("FontFile"),
			cfg.Int("PixelWidth") / scale_x,
			cfg.Int("PixelHeight") / scale_y,
			cfg.Int("Outline") ? true : false,
			cfg.Float("AdvanceXAdjustment")
		);
	}

} // namespace Text
