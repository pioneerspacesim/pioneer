// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Adjustment.h"
#include "Context.h"

namespace UI {

Point Adjustment::PreferredSize()
{
	const Point sliderSize = m_slider ? m_slider->PreferredSize() : Point(0);
	if (!m_innerWidget)
		return sliderSize;

	const Point innerWidgetSize = m_innerWidget->PreferredSize();

	return Point(innerWidgetSize.x, SizeAdd(innerWidgetSize.y, sliderSize.y));
}

void Adjustment::Layout()
{
	if (!m_innerWidget) return;

	const Point size(GetSize());

	const Point childPreferredSize = m_innerWidget->PreferredSize();


	if (!m_slider) {
		m_slider = GetContext()->HSlider();
		m_slider->onValueChanged.connect(sigc::mem_fun(this, &Adjustment::OnScroll));
		m_slider->onMouseWheel.connect(sigc::mem_fun(this, &Adjustment::OnMouseWheel));
		AddWidget(m_slider);
	}

	const Point sliderSize = m_slider->PreferredSize();

	SetWidgetDimensions(m_slider, Point(0, childPreferredSize.y), Point(size.x, sliderSize.y));
	m_slider->Layout();
	SetScrollPosition(ConvertToRange(std::pair<float,float>(0.0f,1.0f),m_range,m_pos));

	SetWidgetDimensions(m_innerWidget, Point(), Point(childPreferredSize.x, std::max(size.y, childPreferredSize.y)));
	m_innerWidget->Layout();
	
}

float Adjustment::GetScrollPosition() const
{
	return ConvertToRange(std::pair<float,float>(0.0f,1.0f), m_range,m_slider ? m_slider->GetValue() : 0.0f);
	
}

void Adjustment::SetScrollPosition(float v)
{
	m_pos = ConvertToRange(m_range, std::pair<float,float>(0.0f,1.0f), v);
	if ( m_slider) m_slider->SetValue(m_pos);	
}

void Adjustment::OnScroll(float value)
{
	m_pos = value;
	onSliderChanged.emit();
}

bool Adjustment::OnMouseWheel(const MouseWheelEvent &event)
{
	if (!m_slider) return false;
	SetScrollPosition(ConvertToRange(std::pair<float,float>(0.0f,1.0f),m_range, m_slider->GetValue() + (event.direction == MouseWheelEvent::WHEEL_UP ? 0.01f : -0.01f)));
	return true;
}

Adjustment *Adjustment::SetInnerWidget(Widget *widget)
{
	assert(widget);

	if (m_innerWidget) {
		m_onMouseWheelConn.disconnect();
		RemoveWidget(m_innerWidget);
	}

	AddWidget(widget);
	m_innerWidget = widget;

	m_onMouseWheelConn = m_innerWidget->onMouseWheel.connect(sigc::mem_fun(this, &Adjustment::OnMouseWheel));

	return this;
}

void Adjustment::RemoveInnerWidget()
{
	if (m_innerWidget) {
		Container::RemoveWidget(m_innerWidget);
		m_innerWidget = 0;
	}
}

void Adjustment::RemoveWidget(Widget *widget)
{
	if (m_innerWidget != widget)
		return;
	RemoveInnerWidget();
}

}
