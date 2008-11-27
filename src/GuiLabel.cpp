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
#if 0
	float size[2]; GetSize(size);
	glColor3f(1,0,0);
	glBegin(GL_QUADS);
		glVertex2f(0, size[1]);
		glVertex2f(size[0], size[1]);
		glVertex2f(size[0], 0);
		glVertex2f(0, 0);
	glEnd();
#endif /* 0 */
	glColor3fv(m_color);
	Screen::RenderMarkup(m_text);
//	Screen::LayoutString(m_text, 400);
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
