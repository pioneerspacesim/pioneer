// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Gui.h"
#include "text/TextSupport.h"
#include "utils.h"

static const float PARAGRAPH_SPACING = 1.5f;

namespace Gui {

TextLayout::TextLayout(const char *_str, RefCountedPtr<Text::TextureFont> font, ColourMarkupMode markup)
{
	// XXX ColourMarkupSkip not correctly implemented yet
	assert(markup != ColourMarkupSkip);

	m_colourMarkup = markup;
	m_font = font ? font : Gui::Screen::GetFont();

	str = static_cast<char *>(malloc(strlen(_str)+1));
	strcpy(str, _str);

	m_justify = false;
	float wordWidth = 0;
	char *wordstart = str;

	int i = 0;
	while (str[i]) {
		wordWidth = 0;
		wordstart = &str[i];

		while (str[i] && str[i] != ' ' && str[i] != '\r' && str[i] != '\n') {
			/* skip color control code things! */
			if ((markup != ColourMarkupNone) && (str[i] == '#')) {
				unsigned int hexcol;
				if (sscanf(&str[i], "#%3x", &hexcol)==1) {
					i+=4;
					continue;
				}
			}

			Uint32 chr;
			int n = Text::utf8_decode_char(&chr, &str[i]);
			assert(n);
			i += n;

			const Text::TextureFont::glfglyph_t &glyph = m_font->GetGlyph(chr);
			wordWidth += glyph.advx;

			// XXX this should do kerning
		}

		words.push_back(word_t(wordstart, wordWidth));

		if (str[i]) {
			if (str[i] == '\n') words.push_back(word_t(0,0));
			str[i++] = 0;
		}
	}
}

void TextLayout::MeasureSize(const float width, float outSize[2]) const
{
	float fontScale[2];
	Gui::Screen::GetCoords2Pixels(fontScale);
	_MeasureSizeRaw(width / fontScale[0], outSize);
	outSize[0] = ceil(outSize[0] * fontScale[0]);
	outSize[1] = ceil(outSize[1] * fontScale[1]);
}

void TextLayout::Render(const float width, const Color &color) const
{
	PROFILE_SCOPED()
	float fontScale[2];
	Gui::Screen::GetCoords2Pixels(fontScale);

	Graphics::Renderer *pRenderer = Gui::Screen::GetRenderer();
	if(!pRenderer) return;
	
	const matrix4x4f &modelMatrix = pRenderer->GetCurrentModelView();
	pRenderer->PushMatrix();
	{
		const float x = modelMatrix[12];
		const float y = modelMatrix[13];
		pRenderer->LoadIdentity();
		pRenderer->Translate(floor(x/fontScale[0])*fontScale[0], floor(y/fontScale[1])*fontScale[1], 0);
		pRenderer->Scale(fontScale[0], fontScale[1], 1);
		_RenderRaw(width / fontScale[0], color);
	}
	pRenderer->PopMatrix();
}

void TextLayout::_RenderRaw(float maxWidth, const Color &color) const
{
	PROFILE_SCOPED()
	float py = 0;

	Graphics::Renderer *pRenderer = Gui::Screen::GetRenderer();
	if(!pRenderer) return;

	pRenderer->PushMatrix();

	const float spaceWidth = m_font->GetGlyph(' ').advx;

	Color c = color;

	std::list<word_t>::const_iterator wpos = this->words.begin();
	// build lines of text
	while (wpos != this->words.end()) {
		float len = 0;
		int num = 0;

		std::list<word_t>::const_iterator i = wpos;
		len += (*i).advx;
		num++;
		bool overflow = false;
		bool explicit_newline = false;
		if ((*i).word != 0) {
			++i;
			for (; i != this->words.end(); ++i) {
				if ((*i).word == 0) {
					// newline
					explicit_newline = true;
					num++;
					break;
				}
				if (len + spaceWidth + (*i).advx > maxWidth) { overflow = true; break; }
				len += (*i).advx + spaceWidth;
				num++;
			}
		}

		float _spaceWidth;
		if ((m_justify) && (num>1) && overflow) {
			float spaceleft = maxWidth - len;
			_spaceWidth = spaceWidth + (spaceleft/float(num-1));
		} else {
			_spaceWidth = spaceWidth;
		}

		float px = 0;
		for (int j=0; j<num; j++) {
			if ((*wpos).word) {
				if (m_colourMarkup == ColourMarkupUse)
					c = m_font->RenderMarkup((*wpos).word, round(px), round(py), c);
				else
					m_font->RenderString((*wpos).word, round(px), round(py), c);
			}
			px += (*wpos).advx + _spaceWidth;
			wpos++;
		}
		py += m_font->GetHeight() * (explicit_newline ? PARAGRAPH_SPACING : 1.0f);
	}
	pRenderer->PopMatrix();
}

void TextLayout::_MeasureSizeRaw(const float layoutWidth, float outSize[2]) const
{
	outSize[0] = 0;
	outSize[1] = 0;

	const float spaceWidth = m_font->GetGlyph(' ').advx;

	// build lines of text
	for (std::list<word_t>::const_iterator wpos = words.begin(); wpos != words.end(); ) {
		float len = 0;
		int num = 0;
		bool explicit_newline = false;

		std::list<word_t>::const_iterator i = wpos;
		len += (*i).advx;
		num++;
		bool overflow = false;
		if ((*i).word != 0) {
			++i;
			for (; i != words.end(); ++i) {
				if ((*i).word == 0) {
					// newline
					explicit_newline = true;
					num++;
					break;
				}
				if (len + spaceWidth + (*i).advx > layoutWidth) { overflow = true; break; }
				len += (*i).advx + spaceWidth;
				num++;
			}
		}

		float _spaceWidth;
		if ((m_justify) && (num>1) && overflow) {
			float spaceleft = layoutWidth - len;
			_spaceWidth = spaceWidth + (spaceleft/float(num-1));
		} else {
			_spaceWidth = spaceWidth;
		}

		float lineLen = 0;
		for (int j=0; j<num; j++) {
			word_t word = (*wpos);
			lineLen += word.advx;
			if (j < num-1) lineLen += _spaceWidth;
			wpos++;
		}
		if (lineLen > outSize[0]) outSize[0] = lineLen;
		outSize[1] += m_font->GetHeight() * (explicit_newline ? PARAGRAPH_SPACING : 1.0f);
	}
	if (outSize[1] > 0.0f)
		outSize[1] += m_font->GetDescender();
}

}
