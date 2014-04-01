// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef UI_TEXTLAYOUT_H
#define UI_TEXTLAYOUT_H

#include "Point.h"
#include "RefCounted.h"
#include "Color.h"
#include <string>
#include <vector>

namespace Text { class TextureFont; }
namespace Graphics { class Renderer; }

namespace UI {

class TextLayout {
public:
	TextLayout(const RefCountedPtr<Text::TextureFont> &font, const std::string &text);

	Point ComputeSize(const Point &layoutSize);

	void Draw(const Point &layoutSize, const Point &drawPos, const Point &drawSize, const Color &color = Color::WHITE);

private:
	struct Word {
		Word(const std::string &_text) : text(_text) {}
		std::string text;
		Point    pos;
	};
	std::vector<Word> m_words;

	Point m_lastRequested;   // the layout area we were asked to compute size for
	Point m_lastSize;        // and the resulting size

	RefCountedPtr<Text::TextureFont> m_font;
};

}

#endif
