// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Scroller.h"
#include "Context.h"

namespace UI {

Point Scroller::PreferredSize()
{
	const Point sliderSize = m_slider ? m_slider->PreferredSize() : Point(0);
	if (!m_innerWidget)
		return sliderSize;

	const Point innerWidgetSize = m_innerWidget->PreferredSize();

	return Point(SizeAdd(innerWidgetSize.x, sliderSize.x), innerWidgetSize.y);
}

void Scroller::Layout()
{
	if (!m_innerWidget) return;

	const Point size(GetSize());

	const Point childPreferredSize = m_innerWidget->PreferredSize();

	// if the child can fit then we don't need the slider
	if (childPreferredSize.y <= size.y) {
		if (m_slider) {
			Container::RemoveWidget(m_slider);
			m_slider = 0;
		}

		SetWidgetDimensions(m_innerWidget, Point(), size);
		m_innerWidget->Layout();
	}

	else {
		if (!m_slider) {
			m_slider = GetContext()->VSlider();
			m_slider->onValueChanged.connect(sigc::mem_fun(this, &Scroller::OnScroll));
			m_slider->onMouseWheel.connect(sigc::mem_fun(this, &Scroller::OnMouseWheel));
			AddWidget(m_slider);
		}

		const Point sliderSize = m_slider->PreferredSize();

		SetWidgetDimensions(m_slider, Point(size.x-sliderSize.x, 0), Point(sliderSize.x, size.y));
		m_slider->Layout();

		SetWidgetDimensions(m_innerWidget, Point(), Point(size.x-sliderSize.x, std::max(size.y, m_innerWidget->PreferredSize().y)));
		m_innerWidget->Layout();
	}
}

float Scroller::GetScrollPosition() const
{
	return m_slider ? m_slider->GetValue() : 0.0f;
}

void Scroller::SetScrollPosition(float v)
{
	if (m_slider) m_slider->SetValue(v);
}

void Scroller::OnScroll(float value)
{
	if (!m_innerWidget) return;
	m_innerWidget->SetDrawOffset(Point(0, -float(m_innerWidget->GetActiveArea().y-GetSize().y)*value));
}

bool Scroller::OnMouseWheel(const MouseWheelEvent &event)
{
	if (!m_slider) return false;
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
		Container::RemoveWidget(m_innerWidget);
		m_innerWidget = 0;
	}
}

void Scroller::RemoveWidget(Widget *widget)
{
	if (m_innerWidget != widget)
		return;
	RemoveInnerWidget();
}

}
