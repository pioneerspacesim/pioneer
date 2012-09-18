#ifndef _TEXT_FONTDESCRIPTOR_H
#define _TEXT_FONTDESCRIPTOR_H

#include <string>

namespace Text {

struct FontDescriptor {
	FontDescriptor(const std::string &_filename, int _pixelWidth, int _pixelHeight, bool _outline = false, float _advanceXAdjustment = 0.0f) :
		filename(_filename), pixelWidth(_pixelWidth), pixelHeight(_pixelHeight), pointSize(0.0f), outline(_outline), advanceXAdjustment(_advanceXAdjustment)
	{}

	FontDescriptor(const std::string &_filename, float _pointSize) :
		filename(_filename), pixelWidth(0), pixelHeight(0), pointSize(_pointSize), outline(false), advanceXAdjustment(0.0f)
	{}

	const std::string filename;
	const int pixelWidth, pixelHeight; // for texture (ui) fonts
	const float pointSize;             // for vector (world) fonts
	const bool outline;                // stroked outline (texture only)
	const float advanceXAdjustment;    // adjust horizontal distance between glyphs (texture only)

	void operator=(const FontDescriptor &o) {
		const_cast<std::string&>(filename) = o.filename;
		const_cast<int&>(pixelWidth) = o.pixelWidth;
		const_cast<int&>(pixelHeight) = o.pixelHeight;
		const_cast<float&>(pointSize) = o.pointSize;
		const_cast<bool&>(outline) = o.outline;
		const_cast<float&>(advanceXAdjustment) = o.advanceXAdjustment;
	}
};

}

#endif
