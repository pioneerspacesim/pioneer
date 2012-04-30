#include "Scroller.h"
#include "Context.h"

namespace UI {

Scroller::Scroller(Context *context) : Container(context), m_innerWidget(0)
{
	m_slider = GetContext()->VSlider();
	AddWidget(m_slider);

	m_slider->onValueChanged.connect(sigc::mem_fun(this, &Scroller::HandleScroll));
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
		SetWidgetDimensions(m_innerWidget, 0, vector2f(size.x-sliderSize.x, size.y));
		m_innerWidget->Layout();
	}
}

void Scroller::HandleScroll(float value)
{
	if (!m_innerWidget) return;

	m_innerWidget->SetTransform(matrix4x4f::Translation(0, -(m_innerWidget->PreferredSize().y-GetSize().y)*value, 0));
}

Scroller *Scroller::SetInnerWidget(Widget *widget)
{
	assert(!m_innerWidget);

	AddWidget(widget);

	m_innerWidget = widget;

	return this;
}

}
