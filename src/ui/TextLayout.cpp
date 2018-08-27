// Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "TextLayout.h"
#include "Widget.h"
#include "RefCounted.h"
#include "text/TextureFont.h"
#include "Color.h"
#include "graphics/VertexArray.h"
#include "graphics/VertexBuffer.h"

namespace UI {

TextLayout::TextLayout(const RefCountedPtr<Text::TextureFont> &font, const std::string &text)
	: m_font(font),	m_lastDrawPos(Point(INT_MIN, INT_MIN)), m_lastDrawSize(Point(INT_MIN, INT_MIN)), m_prevColor(Color::WHITE)
{
	if (!text.size())
		return;

	// split text on space/newline into words
	const std::string delim(" \n");

	size_t start = 0, end = 0;
	while (end != std::string::npos) {

		// start where we left off last time
		start = end;

		// skip over delimeter chars
		while (start < text.size() && delim.find_first_of(text[start]) != std::string::npos) {
			// if we found a newline, push an empty "word". the layout will
			// handle this specially
			if (text[start] == '\n')
				m_words.push_back(Word(""));

			// eat
			start++;
		}

		// reached the end, no more to do
		if (start == text.size())
			break;

		// find the end - next delim or end of string
		end = text.find_first_of(delim, start);

		// extract the word and remember it
		std::string word = text.substr(start, (end == std::string::npos) ? std::string::npos : end - start);
		m_words.push_back(Word(word));
	}
}

Point TextLayout::ComputeSize(const Point &layoutSize)
{
	if (layoutSize == Point()) return Point();

	if (layoutSize == m_lastRequested)
		return m_lastSize;

	int spaceWidth = ceilf(m_font->GetGlyph(' ').advX);
	int lineHeight = ceilf(m_font->GetHeight());

	Point pos;
	Point bounds;

	for (std::vector<Word>::iterator i = m_words.begin(); i != m_words.end(); ++i) {

		// newline. move to start of next line
		if (!(*i).text.size()) {
			pos = Point(0, std::max(bounds.y,pos.y+lineHeight));
			continue;
		}

		vector2f _wordSize;
		m_font->MeasureString((*i).text.c_str(), _wordSize.x, _wordSize.y);
		Point wordSize(_wordSize.x, _wordSize.y);

		// we add the word to this line if:
		// - we're at the start of the line; OR
		// - the word does not go past the right edge of the box
		bool wordAdded = false;
		while (!wordAdded) {
			if (pos.x == 0 || pos.x + wordSize.x <= layoutSize.x) {
				(*i).pos = pos;

				// move to the end of the word
				pos.x += wordSize.x;
				bounds = Point(std::max(bounds.x,pos.x), std::max(bounds.y,pos.y+wordSize.y));

				wordAdded = true;
			}

			else
				// retry at start of new line
				pos = Point(0,std::max(bounds.y,pos.y+lineHeight));
		}

		// add a space at the end of each word. its only used to set the start
		// point for the next word if there is one. if there's not then no
		// words are added so it won't push the bounds out
		pos.x += spaceWidth;
	}

	m_lastRequested = layoutSize;
	m_lastSize = bounds;

	return bounds;
}

void TextLayout::Draw(const Point &layoutSize, const Point &drawPos, const Point &drawSize, const Color &color)
{
	// Has anything changed between passes
	const bool bAnyNew = (layoutSize != m_lastRequested) || (m_lastDrawPos != drawPos) || (m_lastDrawSize != drawSize) || (m_prevColor != color);
	if (bAnyNew)
	{
		ComputeSize(layoutSize);
		const int top = -drawPos.y - m_font->GetHeight();
		const int bottom = -drawPos.y + drawSize.y;

		Graphics::VertexArray va(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_DIFFUSE | Graphics::ATTRIB_UV0);

		for (std::vector<Word>::iterator i = m_words.begin(); i != m_words.end(); ++i) {
			if ((*i).pos.y >= top && (*i).pos.y < bottom) {
				m_font->PopulateString(va, (*i).text, (*i).pos.x, (*i).pos.y, color);
			}
		}

		if (!m_vbuffer.Valid() || (m_vbuffer->GetCapacity() < va.GetNumVerts())) {
			m_vbuffer.Reset(m_font->CreateVertexBuffer(va, true));
		}
		else if(!va.IsEmpty()) {
			m_vbuffer->Populate(va);
		}
	}

	m_font->RenderBuffer( m_vbuffer.Get() );

	// store current params
	m_lastRequested = layoutSize;
	m_lastDrawPos = drawPos;
	m_lastDrawSize = drawSize;
	m_prevColor = color;
}

}
