#include "Gui.h"

namespace Gui {

Label::Label(const char *text, TextLayout::ColourMarkupMode colourMarkupMode)
{
	Init(std::string(text), colourMarkupMode);
}

Label::Label(const std::string &text, TextLayout::ColourMarkupMode colourMarkupMode)
{
	Init(text, colourMarkupMode);
}

Label::~Label()
{
	delete m_layout;
}

void Label::Init(const std::string &text, TextLayout::ColourMarkupMode colourMarkupMode)
{
	m_colourMarkupMode = colourMarkupMode;
	m_shadow = false;
	m_layout = 0;
	m_dlist = 0;
	m_font = Gui::Screen::GetFont();
	m_color = ::Color(1.0f, 1.0f, 1.0f, 1.0f);
	SetText(text);
}

void Label::UpdateLayout()
{
	if (m_layout) delete m_layout;
	m_layout = new TextLayout(m_text.c_str(), m_font, m_colourMarkupMode);
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
	m_color[3] = 1.0f;
	return this;
}

Label *Label::Color(float r, float g, float b)
{
	m_color[0] = r;
	m_color[1] = g;
	m_color[2] = b;
	m_color[3] = 1.0f;
	return this;
}

Label *Label::Color(const ::Color &c)
{
	m_color = c;
	return this;
}

void Label::SetText(const char *text)
{
	SetText(std::string(text));
}

void Label::SetText(const std::string &text)
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
	if (m_color.a < 1.0f) glEnable(GL_BLEND);
	glColor4fv(m_color);
	m_layout->Render(size[0]);
	glDisable(GL_BLEND);
}

void Label::GetSizeRequested(float size[2])
{
	m_layout->MeasureSize(size[0], size);
}

}
