// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Label.h"
#include "Context.h"
#include "text/TextureFont.h"

namespace UI {

Label::Label(Context *context, const std::string &text) : Widget(context), m_text(text), m_color(Color::WHITE)
{
	RegisterBindPoint("text", sigc::mem_fun(this, &Label::BindText));
}

Point Label::PreferredSize()
{
	vector2f textSize;
	GetContext()->GetFont(GetFont())->MeasureString(m_text.c_str(), textSize.x, textSize.y);
	m_preferredSize = Point(ceilf(textSize.x), ceilf(textSize.y));
	return m_preferredSize;
}

void Label::Layout()
{
	if (m_preferredSize == Point())
		PreferredSize();

	const Point &size = GetSize();
	SetActiveArea(Point(std::min(m_preferredSize.x,size.x), std::min(m_preferredSize.y,size.y)));
}

void Label::Draw()
{
	static const Color disabledColor(204, 204, 204, 255);
	const Color color(IsDisabled() ? disabledColor : m_color);
	GetContext()->GetFont(GetFont())->RenderString(m_text.c_str(), 0.0f, 0.0f, Color(color.r, color.g, color.b, color.a*GetContext()->GetOpacity()));
}

Label *Label::SetText(const std::string &text)
{
	m_text = text;
	GetContext()->RequestLayout();
	return this;
}

void Label::BindText(PropertyMap &p, const std::string &k)
{
	std::string text;
	p.Get(k, text);
	SetText(text);
}

}
