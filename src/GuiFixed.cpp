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
	m_wantedSize[0] = m_wantedSize[1] = 0;
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

}
