// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "Gui.h"

namespace Gui {

Fixed::Fixed(): Container()
{
	_Init();
	SetSize(float(Gui::Screen::GetWidth()), float(Gui::Screen::GetHeight()));
}

Fixed::Fixed(float w, float h): Container()
{
	float s[2] = { w, h };
	_Init();
	SetSize(w, h);
	SetSizeRequest(s);
}

void Fixed::_Init()
{
	m_userWantedSize[0] = m_userWantedSize[1] = 0;
	m_eventMask = EVENT_ALL;
}

void Fixed::SetSizeRequest(float x, float y)
{
	m_userWantedSize[0] = x;
	m_userWantedSize[1] = y;
}

void Fixed::SetSizeRequest(float size[2])
{
	SetSizeRequest(size[0], size[1]);
}

void Fixed::GetSizeRequested(float size[2])
{
	if (m_userWantedSize[0] > 0.0f && m_userWantedSize[1] > 0.0f) {
		size[0] = m_userWantedSize[0];
		size[1] = m_userWantedSize[1];
	} else {
		float wanted[2];
		wanted[0] = wanted[1] = 0;
		for (WidgetList::iterator i = m_children.begin(), itEnd = m_children.end(); i != itEnd; ++i) {
			float rsize[2] = { size[0] - (*i).pos[0],
					   size[1] - (*i).pos[1] };
			(*i).w->GetSizeRequested(rsize);
			if ((*i).pos[0] + rsize[0] > size[0]) rsize[0] = size[0] - (*i).pos[0];
			if ((*i).pos[1] + rsize[1] > size[1]) rsize[1] = size[1] - (*i).pos[1];
			wanted[0] = std::max(wanted[0], rsize[0] + (*i).pos[0]);
			wanted[1] = std::max(wanted[1], rsize[1] + (*i).pos[1]);
		}
		size[0] = wanted[0];
		size[1] = wanted[1];
	}
}

Fixed::~Fixed()
{
}

void Fixed::UpdateAllChildSizes()
{
	float size[2];
	GetSize(size);
	for (WidgetList::iterator i = m_children.begin(), itEnd = m_children.end(); i != itEnd; ++i) {
		float rsize[2] = { size[0] - (*i).pos[0],
				   size[1] - (*i).pos[1] };
		(*i).w->GetSizeRequested(rsize);
		if ((*i).pos[0] + rsize[0] > size[0]) rsize[0] = size[0] - (*i).pos[0];
		if ((*i).pos[1] + rsize[1] > size[1]) rsize[1] = size[1] - (*i).pos[1];
		(*i).w->SetSize(rsize[0], rsize[1]);
	}
}

void Fixed::OnChildResizeRequest(Widget *child)
{
	float size[2];
	GetSize(size);
	for (WidgetList::iterator i = m_children.begin(), itEnd = m_children.end(); i != itEnd; ++i) {
		if ((*i).w == child) {
			float rsize[2] = { size[0] - (*i).pos[0],
					   size[1] - (*i).pos[1] };
			(*i).w->GetSizeRequested(rsize);
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
