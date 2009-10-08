#include "Gui.h"

namespace Gui {

Label::Label(const char *text)
{
	m_shadow = false;
	m_layout = 0;
	m_dlist = 0;
	SetText(text);
	m_color[0] = m_color[1] = m_color[2] = 1.0f;
}

Label::Label(const std::string &text)
{
	m_shadow = false;
	m_layout = 0;
	m_dlist = 0;
	SetText(text);
	m_color[0] = m_color[1] = m_color[2] = 1.0f;
}

Label::~Label()
{
	if (m_layout) delete m_layout;
}

void Label::UpdateLayout()
{
	if (m_layout) delete m_layout;
	m_layout = new TextLayout(m_text.c_str());
}

void Label::RecalcSize()
{
//	float size[2];
//	Screen::MeasureLayout(m_text, FLT_MAX, size);
	ResizeRequest();
}

Label *Label::Color(const float rgb[3])
{
	m_color[0] = rgb[0];
	m_color[1] = rgb[1];
	m_color[2] = rgb[2];
	return this;
}

Label *Label::Color(float r, float g, float b)
{
	m_color[0] = r;
	m_color[1] = g;
	m_color[2] = b;
	return this;
}

void Label::SetText(const char *text)
{
	m_text = text;
	UpdateLayout();
	RecalcSize();
}

void Label::SetText(const std::string text)
{
	m_text = text;
	UpdateLayout();
	RecalcSize();
}

void Label::Draw()
{
	if (!m_layout) UpdateLayout();
	float size[2]; GetSize(size);
/*	glColor3f(1,0,0);
	glBegin(GL_QUADS);
		glVertex2f(0, size[1]);
		glVertex2f(size[0], size[1]);
		glVertex2f(size[0], 0);
		glVertex2f(0, 0);
	glEnd();*/
	if (m_shadow) {
		glColor3f(0,0,0);
		glTranslatef(1,1,0);
		m_layout->Render(size[0]);
		glTranslatef(-1,-1,0);
	}
	glColor3fv(m_color);
	m_layout->Render(size[0]);
}

void Label::GetSizeRequested(float size[2])
{
	m_layout->MeasureSize(size[0], size);
}

}
