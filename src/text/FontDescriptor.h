// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _TEXT_FONTDESCRIPTOR_H
#define _TEXT_FONTDESCRIPTOR_H

#include <string>

namespace FileSystem {
	class FileSource;
}

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

	static FontDescriptor Load(FileSystem::FileSource &fs, const std::string &path, const std::string &lang);
	/// XXX this one is a hack to support the old Gui code
	static FontDescriptor Load(FileSystem::FileSource &fs, const std::string &path, const std::string &lang, float scale_x, float scale_y);
};

}

#endif
