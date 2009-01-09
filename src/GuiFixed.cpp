#include "libs.h"
#include "Gui.h"

namespace Gui {

Fixed::Fixed(): Container()
{
	_Init();
}

Fixed::Fixed(float w, float h): Container()
{
	float s[2] = { w, h };
	_Init();
	SetSizeRequest(s);
}

void Fixed::_Init()
{
	memcpy(m_bgcol, Color::bg, 3*sizeof(float));
	m_wantedSize[0] = m_wantedSize[1] = 0;
	m_transparent = true;
	m_eventMask = EVENT_ALL;
}

void Fixed::SetSizeRequest(float size[2])
{
	m_wantedSize[0] = size[0];
	m_wantedSize[1] = size[1];
}

void Fixed::GetSizeRequested(float size[2])
{
	if (m_wantedSize[0] && m_wantedSize[1]) {
		size[0] = m_wantedSize[0];
		size[1] = m_wantedSize[1];
	}
}

Fixed::~Fixed()
{
	Screen::RemoveBaseWidget(this);
}

void Fixed::Draw()
{
	float size[2];
	GetSize(size);
	if (!m_transparent) {
		glBegin(GL_QUADS);
			glColor3f(m_bgcol[0], m_bgcol[1], m_bgcol[2]);
			glVertex2f(0, size[1]);
			glVertex2f(size[0], size[1]);
			glVertex2f(size[0], 0);
			glVertex2f(0, 0);
		glEnd();
	}
	Container::Draw();
}

void Fixed::OnChildResizeRequest(Widget *child)
{
	float size[2];
	GetSize(size);
	for (std::list<widget_pos>::iterator i = m_children.begin(); i != m_children.end(); ++i) {
		if ((*i).w == child) {
			float rsize[2] = { size[0] - (*i).pos[0],
					   size[1] - (*i).pos[1] };
			child->GetSizeRequested(rsize);
			if ((*i).pos[0] + rsize[0] > size[0]) rsize[0] = size[0] - (*i).pos[0];
			if ((*i).pos[1] + rsize[1] > size[1]) rsize[1] = size[0] - (*i).pos[1];
			child->SetSize(rsize[0], rsize[1]);
		}
	}
}

void Fixed::Add(Widget *child, float x, float y)
{
	float size[2];
	GetSize(size);
	AppendChild(child, x, y);
	float rsize[2] = { size[0] - x, size[1] - y };
	child->GetSizeRequested(rsize);
	if (x+rsize[0] > size[0]) rsize[0] = size[0]-x;
	if (y+rsize[1] > size[1]) rsize[1] = size[1]-y;
	child->SetSize(rsize[0], rsize[1]);
}

void Fixed::Remove(Widget *child)
{
	Container::RemoveChild(child);
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
