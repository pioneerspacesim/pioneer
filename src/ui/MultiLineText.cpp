// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "MultiLineText.h"
#include "Context.h"
#include "TextLayout.h"

namespace UI {

MultiLineText::MultiLineText(Context *context, const std::string &text) : Widget(context), m_text(text)
{
	m_layout.Reset(new TextLayout(GetContext()->GetFont(GetFont()), m_text));

	SetSizeControlFlags(EXPAND_WIDTH);
}

Point MultiLineText::PreferredSize()
{
	if (m_preferredSize != Point())
		return m_preferredSize;
	return Point();
}

void MultiLineText::Layout()
{
	const Point newSize(m_layout->ComputeSize(GetSize()));
	if (m_preferredSize != newSize) GetContext()->RequestLayout();
	m_preferredSize = newSize;
	SetActiveArea(m_preferredSize);
}

void MultiLineText::Draw()
{
	m_layout->Draw(GetSize());
}

Widget *MultiLineText::SetFont(Font font) {
	Widget::SetFont(font);
	m_layout.Reset(new TextLayout(GetContext()->GetFont(GetFont()), m_text));
	return this;
}

MultiLineText *MultiLineText::SetText(const std::string &text)
{
	m_text = text;
	m_layout.Reset(new TextLayout(GetContext()->GetFont(GetFont()), m_text));
	GetContext()->RequestLayout();
	return this;
}

MultiLineText *MultiLineText::AppendText(const std::string &text)
{
	return SetText(m_text + text);
}

}
