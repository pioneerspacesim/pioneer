// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "TextEntry.h"
#include "Context.h"
#include "text/TextureFont.h"

namespace UI {

TextEntry::TextEntry(Context *context, const std::string &text) : Container(context),
	m_cursor(0)
{
	m_label = GetContext()->Label(text);
	AddWidget(m_label);
}

Point TextEntry::PreferredSize()
{
	const Point labelPreferredSize(m_label->PreferredSize());
	const Point borderSize(GetContext()->GetSkin().BackgroundNormal().borderWidth*2);
	return labelPreferredSize + borderSize;
}

void TextEntry::Layout()
{
	const Uint32 borderWidth = GetContext()->GetSkin().BackgroundNormal().borderWidth;

	const Point &size = GetSize();

	const Point innerPos(borderWidth, borderWidth);
	const Point innerSize(size.x - borderWidth*2, size.y - borderWidth*2);

	SetWidgetDimensions(m_label, innerPos, innerSize);

	m_label->Layout();
}

void TextEntry::Draw()
{
	if (IsSelected())
		GetContext()->GetSkin().DrawBackgroundActive(Point(), GetSize());
	else
		GetContext()->GetSkin().DrawBackgroundNormal(Point(), GetSize());

	Container::Draw();
}

TextEntry *TextEntry::SetText(const std::string &text)
{
	m_label->SetText(text);
	GetContext()->RequestLayout();
	return this;
}

void TextEntry::HandleKeyPress(const KeyboardEvent &event)
{
	std::string text(m_label->GetText());

	if (event.keysym.sym == SDLK_BACKSPACE) {
		if (text.size() > 0) {
			// XXX not UTF-8 safe
			m_cursor = m_cursor == 0 ? 0 : m_cursor-1;
			text.erase(m_cursor, 1);

			m_label->SetText(text);
		}
	}

	// naively accept anything outside C0 and C1. probably safe enough for
	// now, but needs revisiting if we one day support rtl, cjk, etc
	else if ((event.keysym.unicode > 0x1f && event.keysym.unicode < 0x7f) || event.keysym.unicode > 0x9f) {
		// XXX this is just taking the bottom 8 bits, breaking unicode
		char s[2];
		s[0] = event.keysym.unicode;
		s[1] = 0;
		text.insert(m_cursor, s);
		m_cursor++;

		m_label->SetText(text);
	}
}

}
