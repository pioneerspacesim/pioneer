#include "Gui.h"

namespace Gui {

Label::Label(const char *text)
{
	SetText(text);
	m_color[0] = m_color[1] = m_color[2] = 1.0f;
}

Label::Label(std::string &text)
{
	SetText(text);
	m_color[0] = m_color[1] = m_color[2] = 1.0f;
}

void Label::RecalcSize()
{
	float w, h;
	Screen::MeasureString(m_text, w, h);
	SetSize(w, h);
}

void Label::SetText(const char *text)
{
	m_text = text;
	RecalcSize();
}

void Label::SetText(std::string &text)
{
	m_text = text;
	RecalcSize();
}

void Label::Draw()
{
	glColor3fv(m_color);
	Screen::RenderMarkup(m_text);
}

void Label::GetSizeRequested(float size[2])
{
	RecalcSize();
	GetSize(size);
}

void Label::SetColor(float r, float g, float b)
{
	m_color[0] = r;
	m_color[1] = g;
	m_color[2] = b;
}

}
