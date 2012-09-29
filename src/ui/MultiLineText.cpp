// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "MultiLineText.h"
#include "Context.h"
#include "TextLayout.h"

namespace UI {

MultiLineText::MultiLineText(Context *context, const std::string &text) : Widget(context), m_text(text)
{
    // XXX hook SetFontSize and update to remake TextLayout
	m_layout.Reset(new TextLayout(GetContext()->GetFont(GetFontSize()), m_text));
}

Point MultiLineText::PreferredSize()
{
	if (m_preferredSize != Point())
		return m_preferredSize;
	return m_layout->ComputeSize(Point());
}

void MultiLineText::Layout()
{
	m_preferredSize = m_layout->ComputeSize(GetSize());
	SetActiveArea(m_preferredSize);
}

void MultiLineText::Draw()
{
	m_layout->Draw(GetSize());
}

MultiLineText *MultiLineText::SetText(const std::string &text)
{
	m_text = text;
	m_layout.Reset(new TextLayout(GetContext()->GetFont(GetFontSize()), m_text));
	GetContext()->RequestLayout();
	return this;
}

MultiLineText *MultiLineText::AppendText(const std::string &text)
{
	return SetText(m_text + text);
}

}
