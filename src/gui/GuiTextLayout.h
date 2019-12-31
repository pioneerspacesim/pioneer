// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GUITEXTLAYOUT_H
#define _GUITEXTLAYOUT_H

#include "text/TextureFont.h"

namespace Graphics {
	class Vertexbuffer;
}

namespace Gui {
	class TextLayout {
	public:
		enum ColourMarkupMode {
			ColourMarkupNone, // treats markup as normal text
			ColourMarkupSkip, // skips markup tags
			ColourMarkupUse // interprets markup tags
		};
		explicit TextLayout(const char *_str, RefCountedPtr<Text::TextureFont> font = RefCountedPtr<Text::TextureFont>(0), ColourMarkupMode markup = ColourMarkupUse);
		void Render(const float layoutWidth, const Color &color = Color::WHITE) const;
		void Update(const float layoutWidth, const Color &color = Color::WHITE);
		void SetText(const char *_str);
		void MeasureSize(const float layoutWidth, float outSize[2]) const;
		void _MeasureSizeRaw(const float layoutWidth, float outSize[2]) const;
		void SetJustified(bool v) { m_justify = v; }

	private:
		struct word_t {
			const char *word;
			const float advx;
			word_t(const char *_word, const float _advx) :
				word(_word),
				advx(_advx) {}
		};
		std::list<word_t> words;
		std::string str;
		bool m_justify;
		ColourMarkupMode m_colourMarkup;

		float prevWidth;
		Color prevColor;

		RefCountedPtr<Text::TextureFont> m_font;
		RefCountedPtr<Graphics::VertexBuffer> m_vbuffer;
	};
} // namespace Gui

#endif /* _GUITEXTLAYOUT_H */
