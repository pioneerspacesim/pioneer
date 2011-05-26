#include "libs.h"
#include "Gui.h"

namespace Gui {

Stack::Stack(): Container()
{
	m_eventMask = EVENT_ALL;
}

Stack::~Stack()
{
	ClearWidgets();
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

Widget *Stack::GetTopWidget()
{
	return m_widgets.empty() ? 0 : m_widgets.top();
}

void Stack::PushWidget(Widget *w)
{
	if (!m_widgets.empty())
		m_widgets.top()->Hide();
	m_widgets.push(w);

	AppendChild(w,0,0);
	w->Show();
}

void Stack::PopWidget()
{
	if (m_widgets.empty()) return;

	Widget *w = m_widgets.top();
	m_widgets.pop();

	RemoveChild(w);
	delete w;

	if (m_widgets.empty()) return;

	w = m_widgets.top();
	w->Show();
}

void Stack::ClearWidgets()
{
	DeleteAllChildren();
	while (!m_widgets.empty()) m_widgets.pop();
}

void Stack::JumpToWidget(Widget *w)
{
	ClearWidgets();
	PushWidget(w);
}

}
