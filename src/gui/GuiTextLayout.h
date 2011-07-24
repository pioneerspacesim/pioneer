#ifndef _GUITEXTLAYOUT_H
#define _GUITEXTLAYOUT_H

#include "TextureFont.h"

namespace Gui {
class TextLayout {
public:
	TextLayout(const char *_str, TextureFont *font = 0);
	void Render(float layoutWidth) const;
	void MeasureSize(const float layoutWidth, float outSize[2]) const;
	void _RenderRaw(float layoutWidth) const;
	void _MeasureSizeRaw(const float layoutWidth, float outSize[2]) const;
	~TextLayout() { free(str); }
	void SetJustified(bool v) { m_justify = v; }
private:
	struct word_t {
		char *word;
		float advx;
		word_t(char *_word, float _advx): word(_word), advx(_advx) {}
	};
	std::list<word_t> words;
	char *str;
	bool m_justify;

    TextureFont *m_font;
};
}

#endif /* _GUITEXTLAYOUT_H */
