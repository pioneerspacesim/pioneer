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

vector2f Scroller::PreferredSize()
{
	const vector2f sliderSize = m_slider->PreferredSize();
	if (!m_innerWidget)
		return sliderSize;

	const vector2f innerWidgetSize = m_innerWidget->PreferredSize();

	return vector2f(innerWidgetSize.x+sliderSize.x, std::max(innerWidgetSize.y, sliderSize.y));
}

void Scroller::Layout()
{
	const vector2f size(GetSize());
	const vector2f sliderSize = m_slider->PreferredSize();

	SetWidgetDimensions(m_slider, vector2f(size.x-sliderSize.x, 0), vector2f(sliderSize.x, size.y));
	m_slider->Layout();

	if (m_innerWidget) {
		SetWidgetDimensions(m_innerWidget, vector2f(), vector2f(size.x-sliderSize.x, size.y));
		m_innerWidget->Layout();
	}
}

void Scroller::OnScroll(float value)
{
	if (!m_innerWidget) return;

	m_innerWidget->SetTransform(matrix4x4f::Translation(0, -(m_innerWidget->PreferredSize().y-GetSize().y)*value, 0));
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
