// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "TextEntry.h"
#include "Context.h"
#include "text/TextureFont.h"

namespace UI {

TextEntry::TextEntry(Context *context, const std::string &text) : Container(context)
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
			text.erase(text.size()-1);
			m_label->SetText(text);
		}
	}
	else {
		text.push_back(event.keysym.unicode);
		m_label->SetText(text);
	}
}

}
