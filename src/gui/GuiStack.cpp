// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "Gui.h"

namespace Gui {

Stack::Stack(): Container()
{
	m_eventMask = EVENT_ALL;
}

Stack::~Stack()
{
	Clear();
}

void Stack::GetSizeRequested(float size[2])
{
	if (m_widgets.empty())
		size[0] = size[1] = 0.0;
	else
		m_widgets.top()->GetSizeRequested(size);
}

void Stack::OnChildResizeRequest(Widget *w)
{
	float container_size[2];
	GetSize(container_size);

	float widget_size[2];
	w->GetSizeRequested(widget_size);

	widget_size[0] = std::min(widget_size[0], container_size[0]);
	widget_size[1] = std::min(widget_size[1], container_size[1]);

	w->SetSize(widget_size[0], widget_size[1]);
}

void Stack::UpdateAllChildSizes()
{
	if (m_widgets.empty()) return;

	OnChildResizeRequest(m_widgets.top());
}

void Stack::ShowAll()
{
	if (!m_widgets.empty())
		m_widgets.top()->ShowAll();
	Show();
}

Widget *Stack::Top()
{
	return m_widgets.empty() ? 0 : m_widgets.top();
}

int Stack::Size()
{
	return m_widgets.size();
}

void Stack::Push(Widget *w)
{
	if (!m_widgets.empty())
		m_widgets.top()->Hide();

	m_widgets.push(w);
	AppendChild(w,0,0);

	ResizeRequest();
}

void Stack::Pop()
{
	if (m_widgets.empty()) return;

	Widget *w = m_widgets.top();
	m_widgets.pop();

	RemoveChild(w);
	delete w;

	ResizeRequest();
}

void Stack::Clear()
{
	DeleteAllChildren();
	while (!m_widgets.empty()) m_widgets.pop();

	ResizeRequest();
}

void Stack::JumpTo(Widget *w)
{
	Clear();
	Push(w);
}

}
