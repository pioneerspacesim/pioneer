// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Scroller.h"
#include "Context.h"

namespace UI {

Scroller::Scroller(Context *context) : Container(context), m_innerWidget(0)
{
	m_slider = GetContext()->VSlider();
	AddWidget(m_slider);

	m_slider->onValueChanged.connect(sigc::mem_fun(this, &Scroller::OnScroll));
	m_slider->onMouseWheel.connect(sigc::mem_fun(this, &Scroller::OnMouseWheel));
}

Point Scroller::PreferredSize()
{
	const Point sliderSize = m_slider->PreferredSize();
	if (!m_innerWidget)
		return sliderSize;

	const Point innerWidgetSize = m_innerWidget->PreferredSize();

	return Point(innerWidgetSize.x+sliderSize.x, std::max(innerWidgetSize.y, sliderSize.y));
}

void Scroller::Layout()
{
	const Point size(GetSize());
	const Point sliderSize = m_slider->PreferredSize();

	SetWidgetDimensions(m_slider, Point(size.x-sliderSize.x, 0), Point(sliderSize.x, size.y));
	m_slider->Layout();

	if (m_innerWidget) {
		SetWidgetDimensions(m_innerWidget, Point(), Point(size.x-sliderSize.x, std::max(size.y, m_innerWidget->PreferredSize().y)));
		m_innerWidget->Layout();
	}
}

void Scroller::OnScroll(float value)
{
	if (!m_innerWidget) return;

	m_innerWidget->SetDrawOffset(Point(0, -float(m_innerWidget->PreferredSize().y-GetSize().y)*value));
}

bool Scroller::OnMouseWheel(const MouseWheelEvent &event)
{
	m_slider->SetValue(m_slider->GetValue() + (event.direction == MouseWheelEvent::WHEEL_UP ? -0.01f : 0.01f));
	return true;
}

Scroller *Scroller::SetInnerWidget(Widget *widget)
{
	assert(widget);

	if (m_innerWidget) {
		m_onMouseWheelConn.disconnect();
		RemoveWidget(m_innerWidget);
	}

	AddWidget(widget);
	m_innerWidget = widget;

	m_onMouseWheelConn = m_innerWidget->onMouseWheel.connect(sigc::mem_fun(this, &Scroller::OnMouseWheel));

	return this;
}

void Scroller::RemoveInnerWidget()
{
	if (m_innerWidget) {
		RemoveWidget(m_innerWidget);
		m_innerWidget = 0;
	}
}

}
