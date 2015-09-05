// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Label.h"
#include "Context.h"
#include "text/TextureFont.h"
#include "graphics/VertexArray.h"
#include "graphics/VertexBuffer.h"

namespace UI {

static const Color disabledColor(204, 204, 204, 255);

Label::Label(Context *context, const std::string &text) : Widget(context)
, m_bNeedsUpdating(true)
, m_bPrevDisabled(false)
, m_prevOpacity(-1.0f)
, m_text(text)
, m_color(Color::WHITE)
, m_font(GetContext()->GetFont(GetFont()))
{
	RegisterBindPoint("text", sigc::mem_fun(this, &Label::BindText));
}

Point Label::PreferredSize()
{
	if( m_font != GetContext()->GetFont(GetFont()) ) {
		m_font = GetContext()->GetFont(GetFont());
	}
	vector2f textSize;
	m_font->MeasureString(m_text, textSize.x, textSize.y);
	m_preferredSize = Point(ceilf(textSize.x), ceilf(textSize.y));
	return m_preferredSize;
}

void Label::Layout()
{
	if (m_preferredSize == Point())
		PreferredSize();

	const Point &size = GetSize();
	SetActiveArea(Point(std::min(m_preferredSize.x,size.x), std::min(m_preferredSize.y,size.y)));

	m_bNeedsUpdating = true;
}

void Label::Draw()
{
	if (m_text.empty())
		return;

	const Color color(IsDisabled() ? disabledColor : m_color);
	const float opacity = GetContext()->GetOpacity();
	const Color finalColor(color.r, color.g, color.b, color.a*opacity);

	if (m_bNeedsUpdating || m_font != GetContext()->GetFont(GetFont()) || !is_equal_exact(m_prevOpacity, opacity) || m_bPrevDisabled != IsDisabled())
	{
		m_font = GetContext()->GetFont(GetFont());
		Graphics::VertexArray va(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_DIFFUSE | Graphics::ATTRIB_UV0);
		m_font->PopulateString(va, m_text, 0.0f, 0.0f, finalColor);
		m_vbuffer.Reset( m_font->CreateVertexBuffer(va) );
		m_bNeedsUpdating = false;
		m_bPrevDisabled = IsDisabled();
		m_prevOpacity = opacity;
	}

	m_font->RenderBuffer( m_vbuffer.Get() );
}

Label *Label::SetText(const std::string &text)
{
	m_text = text;
	GetContext()->RequestLayout();
	m_bNeedsUpdating = true;
	return this;
}

void Label::BindText(PropertyMap &p, const std::string &k)
{
	std::string text;
	p.Get(k, text);
	SetText(text);
}

}
