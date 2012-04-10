#ifndef _UI_TEXTLAYOUT_H
#define _UI_TEXTLAYOUT_H

#include "vector2.h"
#include "RefCounted.h"
#include <string>
#include <vector>

namespace Text { class TextureFont; }
namespace Graphics { class Renderer; }

namespace UI {

class TextLayout {
public:
	TextLayout(const RefCountedPtr<Text::TextureFont> &font, const std::string &text);

	vector2f ComputeSize(const vector2f &hint);

	void Draw(const vector2f &size);

private:
	struct Word {
		Word(const std::string &_text) : text(_text) {}
		std::string text;
		vector2f    pos;
	};
	std::vector<Word> m_words;

	vector2f m_lastRequested;   // the layout area we were asked to compute size for
	vector2f m_lastSize;        // and the resulting size

	RefCountedPtr<Text::TextureFont> m_font;
};

}

#endif
