// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
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

			const Text::TextureFont::Glyph &glyph = m_font->GetGlyph(chr);
			wordWidth += glyph.advX;

			// XXX this should do kerning
		}

		words.push_back(word_t(wordstart, wordWidth));

		if (str[i]) {
			if (str[i] == '\n') words.push_back(word_t(0,0));
			str[i++] = 0;
		}
	}

	prevWidth = -1.0f;
	prevColor = Color::WHITE;
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
	if(words.empty())
		return;

	float fontScale[2];
	Gui::Screen::GetCoords2Pixels(fontScale);

	Graphics::Renderer *r = Gui::Screen::GetRenderer();

	const matrix4x4f &modelMatrix = r->GetCurrentModelView();
	Graphics::Renderer::MatrixTicket ticket(r, Graphics::MatrixMode::MODELVIEW);
	{
		const float x = modelMatrix[12];
		const float y = modelMatrix[13];
		r->LoadIdentity();
		r->Translate(floor(x/fontScale[0])*fontScale[0], floor(y/fontScale[1])*fontScale[1], 0);
		r->Scale(fontScale[0], fontScale[1], 1);
		m_font->RenderBuffer( m_vbuffer.Get(), color );
	}
}


void TextLayout::Update(const float width, const Color &color)
{
	PROFILE_SCOPED()
	if(words.empty()) {
		m_vbuffer.Reset();
		return;
	}

	// see if anything has changed
	if(is_equal_exact(prevWidth,width) && (prevColor==color)) {
		return;
	}

	prevWidth = width;
	prevColor = color;

	float fontScale[2];
	Gui::Screen::GetCoords2Pixels(fontScale);

	Graphics::Renderer *r = Gui::Screen::GetRenderer();
	Graphics::Renderer::MatrixTicket ticket(r, Graphics::MatrixMode::MODELVIEW);

	Graphics::VertexArray va(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_DIFFUSE | Graphics::ATTRIB_UV0);

	if( r )
	{
		const float maxWidth = width / fontScale[0];

		float py = 0;

		const float spaceWidth = m_font->GetGlyph(' ').advX;

		Color c = color;

		// vertex array pre-assignment, because TextureFont botches it
		// over-reserves for markup, but we don't care
		int numChars = 0;
		std::list<word_t>::const_iterator wpos = this->words.begin();
		for (; wpos != this->words.end(); ++wpos)
			if ((*wpos).word) numChars += strlen((*wpos).word);

		va.position.reserve(6 * numChars);
		va.diffuse.reserve(6 * numChars);
		va.uv0.reserve(6 * numChars);

		// build lines of text
		wpos = this->words.begin();
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
					const std::string word( (*wpos).word );
					if (m_colourMarkup == ColourMarkupUse)
					{
						Color newColor = m_font->PopulateMarkup(va, word, round(px), round(py), c);
						if(!word.empty())
							c = std::move(newColor);
					}
					else
						m_font->PopulateString(va, word, round(px), round(py), c);
				}
				px += (*wpos).advx + _spaceWidth;
				++wpos;
			}
			py += m_font->GetHeight() * (explicit_newline ? PARAGRAPH_SPACING : 1.0f);
		}
	}

	if( va.GetNumVerts() > 0 ) {
		if( !m_vbuffer.Valid() || m_vbuffer->GetVertexCount() != va.GetNumVerts() ) {
			//create buffer and upload data
			Graphics::VertexBufferDesc vbd;
			vbd.attrib[0].semantic = Graphics::ATTRIB_POSITION;
			vbd.attrib[0].format   = Graphics::ATTRIB_FORMAT_FLOAT3;
			vbd.attrib[1].semantic = Graphics::ATTRIB_DIFFUSE;
			vbd.attrib[1].format   = Graphics::ATTRIB_FORMAT_UBYTE4;
			vbd.attrib[2].semantic = Graphics::ATTRIB_UV0;
			vbd.attrib[2].format   = Graphics::ATTRIB_FORMAT_FLOAT2;
			vbd.numVertices = va.GetNumVerts();
			vbd.usage = Graphics::BUFFER_USAGE_DYNAMIC;	// we could be updating this per-frame
			m_vbuffer.Reset( r->CreateVertexBuffer(vbd) );
		}

		m_vbuffer->Populate(va);
	} else {
		m_vbuffer.Reset();
	}
}

void TextLayout::_MeasureSizeRaw(const float layoutWidth, float outSize[2]) const
{
	outSize[0] = 0;
	outSize[1] = 0;

	const float spaceWidth = m_font->GetGlyph(' ').advX;

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
			++wpos;
		}
		if (lineLen > outSize[0]) outSize[0] = lineLen;
		outSize[1] += m_font->GetHeight() * (explicit_newline ? PARAGRAPH_SPACING : 1.0f);
	}
	if (outSize[1] > 0.0f)
		outSize[1] += m_font->GetDescender();
}

}
