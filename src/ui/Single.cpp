#include "Single.h"

namespace UI {

vector2f Single::PreferredSize()
{
	if (!m_innerWidget) return vector2f();
	return m_innerWidget->PreferredSize();
}

void Single::Layout()
{
	if (!m_innerWidget) return;
	SetWidgetDimensions(m_innerWidget, 0, GetSize());
	m_innerWidget->Layout();
}

void Single::RequestResize()
{
	if (GetContainer()) GetContainer()->RequestResize();
}

Single *Single::SetInnerWidget(Widget *widget)
{
	assert(widget);
	assert(!m_innerWidget);

	AddWidget(widget);
	m_innerWidget = widget;

	return this;
}

void Single::RemoveInnerWidget()
{
	assert(m_innerWidget);

	RemoveWidget(m_innerWidget);
	m_innerWidget = 0;
}

}
