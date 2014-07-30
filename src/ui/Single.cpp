// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Single.h"
#include "Context.h"

namespace UI {

Point Single::PreferredSize()
{
	if (!m_innerWidget) return Point();
	return m_innerWidget->CalcLayoutContribution();
}

void Single::Layout()
{
	if (!m_innerWidget) return;
	SetWidgetDimensions(m_innerWidget, Point(), m_innerWidget->CalcSize(GetSize()));
	m_innerWidget->Layout();
}

Single *Single::SetInnerWidget(Widget *widget)
{
	assert(widget);

	RemoveAllWidgets();

	AddWidget(widget);
	m_innerWidget = widget;

	GetContext()->RequestLayout();

	return this;
}

void Single::RemoveInnerWidget()
{
	if (m_innerWidget) {
		Container::RemoveWidget(m_innerWidget);
		m_innerWidget = 0;
		GetContext()->RequestLayout();
	}
}

void Single::RemoveWidget(Widget *widget)
{
	if (m_innerWidget != widget)
		return;
	RemoveInnerWidget();
}

}
