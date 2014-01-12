// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt


#include "libs.h"
#include "Gui.h"

namespace Gui {

Box::Box(BoxOrientation orient): Container()
{
	m_orient = orient;
	_Init();
}

void Box::_Init()
{
	m_spacing = 0;
	m_wantedSize[0] = m_wantedSize[1] = 0;
	m_eventMask = EVENT_ALL;
}

void Box::SetSizeRequest(float x, float y)
{
	m_wantedSize[0] = x;
	m_wantedSize[1] = y;
}

void Box::SetSizeRequest(float size[2])
{
	SetSizeRequest(size[0], size[1]);
}

void Box::GetSizeRequestedOrMinimum(float size[2], bool minimum)
{
	if (m_wantedSize[0] > 0.0f && m_wantedSize[1] > 0.0f) {
		size[0] = m_wantedSize[0];
		size[1] = m_wantedSize[1];
	} else {
		int num_kids = 0;
		float want[2];
		want[0] = want[1] = 0;
		// see how big we need to be
		for (WidgetList::iterator i = m_children.begin(), itEnd = m_children.end(); i != itEnd; ++i) {
			if (!(*i).w->IsVisible()) continue;
			num_kids++;
			float rsize[2];
			rsize[0] = size[0];
			rsize[1] = size[1];
			if (!minimum)
				(*i).w->GetSizeRequested(rsize);
			else
				(*i).w->GetMinimumSize(rsize);
			if (m_orient == BOX_VERTICAL) {
				want[0] = std::max(want[0], rsize[0]);
				want[1] += rsize[1];
			} else {
				want[0] += rsize[0];
				want[1] = std::max(want[1], rsize[1]);
			}
		}
		if (num_kids) want[m_orient] += (num_kids-1)*m_spacing;
		size[0] = want[0];
		size[1] = want[1];
	}
}

Box::~Box()
{
	Screen::RemoveBaseWidget(this);
}

void Box::OnChildResizeRequest(Widget *child)
{
	UpdateAllChildSizes();
}

void Box::PackStart(Widget *child)
{
	PrependChild(child, 0, 0);
	ResizeRequest();
}

void Box::PackEnd(Widget *child)
{
	AppendChild(child, 0, 0);
	ResizeRequest();
}

void Box::UpdateAllChildSizes()
{
	float size[2];
	GetSize(size);
	float pos = 0;
	float space = (m_orient == BOX_VERTICAL ? size[1] : size[0]);
	int num_expand_children = 0;
	// look at all children...
	for (WidgetList::iterator i = m_children.begin(), itEnd = m_children.end(); i != itEnd; ++i) {
		float msize[2], bsize[2];

		if (m_orient == BOX_VERTICAL) {
			msize[0] = bsize[0] = size[0];
			msize[1] = bsize[1] = space;
			(*i).w->GetMinimumSize(msize);
			(*i).w->GetSizeRequested(bsize);

			(*i).flags = (msize[1] < bsize[1]) ? 1 : 0;

			if (msize[0] > size[0]) msize[0] = size[0];
			if (msize[1] > space) msize[1] = space;
			(*i).w->SetSize(size[0], msize[1]);
			(*i).pos[0] = 0;
			(*i).pos[1] = pos;
			pos += msize[1] + m_spacing;
			space -= msize[1] + m_spacing;
		} else {
			msize[0] = bsize[0] = size[0];
			msize[1] = bsize[1] = space;
			(*i).w->GetMinimumSize(msize);
			(*i).w->GetSizeRequested(bsize);

			(*i).flags = (msize[0] < bsize[0]) ? 1 : 0;

			if (msize[0] > space) msize[0] = space;
			if (msize[1] > size[1]) msize[1] = size[1];
			(*i).w->SetSize(msize[0], size[1]);
			(*i).pos[0] = pos;
			(*i).pos[1] = 0;
			pos += msize[0] + m_spacing;
			space -= msize[0] + m_spacing;
		}

		if ((*i).flags) num_expand_children++;
	}
	// last item does not need spacing after it...
	space += m_spacing;
	pos = 0;
	if ((space > 0) && num_expand_children) {
		/* give expand children the space space */
		for (WidgetList::iterator i = m_children.begin(), itEnd = m_children.end(); i != itEnd; ++i) {
			bool expand = (*i).flags != 0;
			float s[2];
			(*i).w->GetSize(s);

			if (m_orient == BOX_VERTICAL) {
				(*i).pos[0] = 0;
				(*i).pos[1] = pos;
				if (expand) {
					s[1] += space / num_expand_children;
					(*i).w->SetSize(s[0], s[1]);
				}
				pos += s[1] + m_spacing;
			} else {
				(*i).pos[0] = pos;
				(*i).pos[1] = 0;
				if (expand) {
					s[0] += space / num_expand_children;
					(*i).w->SetSize(s[0], s[1]);
				}
				pos += s[0] + m_spacing;
			}
		}
	}
}

void Box::Remove(Widget *child)
{
	Container::RemoveChild(child);
	ResizeRequest();
}

}
