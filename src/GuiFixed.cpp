#include "libs.h"
#include "Gui.h"
#include "Pi.h"

namespace Gui {

Fixed::Fixed(float w, float h)
{
	SetSize(w, h);
	memcpy(m_bgcol, Color::bg, 3*sizeof(float));
	m_w = w; m_h = h;
	m_transparent = false;
	m_eventMask = EVENT_ALL;
}

void Fixed::GetSizeRequested(float size[2])
{
	GetSize(size);
}

Fixed::~Fixed()
{
	Screen::RemoveBaseWidget(this);
}

void Fixed::Draw()
{
	if (!m_transparent) {
		glBegin(GL_QUADS);
			glColor3f(m_bgcol[0], m_bgcol[1], m_bgcol[2]);
			glVertex2f(m_w, 0);
			glVertex2f(m_w, m_h);
			glVertex2f(0, m_h);
			glVertex2f(0, 0);
		glEnd();
	}
	Container::Draw();
}

void Fixed::Add(Widget *child, float x, float y)
{
	AppendChild(child, x, y);
}

void Fixed::Remove(Widget *child)
{
	for (std::list<widget_pos>::iterator i = m_children.begin(); i != m_children.end(); ++i) {
		if ((*i).w == child) {
			m_children.erase(i);
			return;
		}
	}
}

void Fixed::SetBgColor(float rgb[3])
{
	SetBgColor(rgb[0], rgb[1], rgb[2]);
}
void Fixed::SetBgColor(float r, float g, float b)
{
	m_bgcol[0] = r;
	m_bgcol[1] = g;
	m_bgcol[2] = b;
}

}
