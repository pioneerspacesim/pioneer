// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef TEXT_FONTCONFIG_H
#define TEXT_FONTCONFIG_H

#include "libs.h"
#include <string>

namespace Text {

class FontConfig {
public:
	// XXX scale is to support to the old UI, and will be removed
	FontConfig(const std::string &name, float scaleX = 1.0f, float scaleY = 1.0f);

	struct Face {
		Face(const std::string &fontFile_, int pixelWidth_, int pixelHeight_, float advanceXAdjustment_, Uint32 rangeMin_, Uint32 rangeMax_) :
			fontFile(fontFile_), pixelWidth(pixelWidth_), pixelHeight(pixelHeight_), advanceXAdjustment(advanceXAdjustment_), rangeMin(rangeMin_), rangeMax(rangeMax_) {}
		const std::string fontFile;
		const int pixelWidth;
		const int pixelHeight;
		const float advanceXAdjustment;
		const Uint32 rangeMin;
		const Uint32 rangeMax;

		void operator=(const Face &o) {
			const_cast<std::string&>(fontFile) = o.fontFile;
			const_cast<int&>(pixelWidth) = o.pixelWidth;
			const_cast<int&>(pixelHeight) = o.pixelHeight;
			const_cast<float&>(advanceXAdjustment) = o.advanceXAdjustment;
			const_cast<Uint32&>(rangeMin) = o.rangeMin;
			const_cast<Uint32&>(rangeMax) = o.rangeMax;
		}

		bool operator<(const Face &o) const {
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

}

#endif
