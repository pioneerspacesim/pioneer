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
	SetWidgetDimensions(m_innerWidget, vector2f(), GetSize());
	m_innerWidget->Layout();
}

Single *Single::SetInnerWidget(Widget *widget)
{
	assert(widget);

	RemoveAllWidgets();

	AddWidget(widget);
	m_innerWidget = widget;

	return this;
}

void Single::RemoveInnerWidget()
{
	if (m_innerWidget) {
		RemoveWidget(m_innerWidget);
		m_innerWidget = 0;
	}
}

}
