// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef TEXT_FONTCONFIG_H
#define TEXT_FONTCONFIG_H

#include "libs.h"
#include <string>

namespace Text {

	// WARNING: FontConfig is intended to be immutable; internal values shall not be changed
	class FontConfig {
	public:
		// XXX scale is to support to the old UI, and will be removed
		FontConfig(const std::string &name, float scaleX = 1.0f, float scaleY = 1.0f);

		struct Face {
			Face(const std::string &fontFile_, int pixelWidth_, int pixelHeight_, float advanceXAdjustment_, Uint32 rangeMin_, Uint32 rangeMax_) :
				fontFile(fontFile_),
				pixelWidth(pixelWidth_),
				pixelHeight(pixelHeight_),
				advanceXAdjustment(advanceXAdjustment_),
				rangeMin(rangeMin_),
				rangeMax(rangeMax_) {}

			// WARNING: these values shall not be changed
			std::string fontFile;
			int pixelWidth;
			int pixelHeight;
			float advanceXAdjustment;
			Uint32 rangeMin;
			Uint32 rangeMax;

			bool operator<(const Face &o) const
			{
				if (pixelWidth < o.pixelWidth) return true;
				if (pixelHeight < o.pixelHeight) return true;
				if (fontFile < o.fontFile) return true;
				return false;
			}
		};

		const std::string &GetName() const { return m_name; }
		bool IsOutline() const { return m_outline; }

		const Face &GetFaceForCodePoint(Uint32 cp);

	private:
		std::string m_name;
		bool m_outline;
		std::vector<Face> m_faces;
	};

} // namespace Text

#endif
