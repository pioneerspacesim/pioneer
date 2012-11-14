// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Slider.h"
#include "Context.h"

namespace UI {

Point Slider::PreferredSize()
{
	const Skin &skin = GetContext()->GetSkin();

	return m_orient == SLIDER_HORIZONTAL ? Point(SIZE_EXPAND, skin.SliderHorizontalButtonNormal().size.y) : Point(skin.SliderVerticalButtonNormal().size.x, SIZE_EXPAND);
}

void Slider::Layout()
{
	const Skin &skin = GetContext()->GetSkin();
	const Point &activeArea = GetActiveArea();

	if (m_orient == SLIDER_HORIZONTAL) {
		const Skin::EdgedRectElement &gutterRect = skin.SliderHorizontalGutter();
		m_gutterPos  = Point(0, (activeArea.y-gutterRect.size.y)/2);
		m_gutterSize = Point(activeArea.x, gutterRect.size.y);
	}
	else {
		const Skin::EdgedRectElement &gutterRect = skin.SliderVerticalGutter();
		m_gutterPos  = Point((activeArea.x-gutterRect.size.x)/2, 0);
		m_gutterSize = Point(gutterRect.size.x, activeArea.y);
	}

	UpdateButton();
	Widget::Layout();
}

void Slider::UpdateButton()
{
	const Skin &skin = GetContext()->GetSkin();

	const Point activeArea(GetActiveArea());

	if (m_orient == SLIDER_HORIZONTAL) {
		const Skin::EdgedRectElement &gutterRect = skin.SliderHorizontalGutter();
		const Skin::RectElement &buttonRect = skin.SliderHorizontalButtonNormal();

		m_buttonSize = Point(buttonRect.size.x, buttonRect.size.y);
		m_buttonPos  = Point(((activeArea.x-buttonRect.pos.x)*m_value)+gutterRect.edgeWidth, buttonRect.size.y/2);
	}

	else {
		const Skin::EdgedRectElement &gutterRect = skin.SliderVerticalGutter();
		const Skin::RectElement &buttonRect = skin.SliderVerticalButtonNormal();

		m_buttonSize = Point(buttonRect.size.x, buttonRect.size.y);
		m_buttonPos  = Point((activeArea.x-buttonRect.size.x)/2, ((activeArea.y-buttonRect.pos.y)*m_value)+gutterRect.edgeWidth);
	}
}

void Slider::Draw()
{
	const Skin &skin = GetContext()->GetSkin();

	if (m_orient == SLIDER_HORIZONTAL) {
		skin.DrawSliderHorizontalGutter(m_gutterPos, m_gutterSize);
		if (m_buttonDown && IsMouseActive())
			skin.DrawSliderHorizontalButtonActive(m_buttonPos, m_buttonSize);
		else
			skin.DrawSliderHorizontalButtonNormal(m_buttonPos, m_buttonSize);
	}

	else {
		skin.DrawSliderVerticalGutter(m_gutterPos, m_gutterSize);
		if (m_buttonDown && IsMouseActive())
			skin.DrawSliderVerticalButtonActive(m_buttonPos, m_buttonSize);
		else
			skin.DrawSliderVerticalButtonNormal(m_buttonPos, m_buttonSize);
	}
}

void Slider::SetValue(float v)
{
	m_value = Clamp(v, 0.0f, 1.0f);
	onValueChanged.emit(m_value);
	UpdateButton();
}

void Slider::HandleMouseDown(const MouseButtonEvent &event)
{
	m_buttonDown = event.pos.x >= m_buttonPos.x && event.pos.y >= m_buttonPos.y && event.pos.x < m_buttonPos.x+m_buttonSize.x && event.pos.y < m_buttonPos.y+m_buttonSize.y;
	Widget::HandleMouseDown(event);
}

void Slider::HandleMouseUp(const MouseButtonEvent &event)
{
	m_buttonDown = false;
	Widget::HandleMouseUp(event);
}

void Slider::HandleMouseMove(const MouseMotionEvent &event)
{
	if (m_buttonDown && IsMouseActive()) {
		const Point::Component c = m_orient == SLIDER_HORIZONTAL ? Point::X : Point::Y;

		const int effectiveLength = GetActiveArea()[c] - m_buttonSize[c];
		const int pos = Clamp(event.pos[c] - GetActiveOffset()[c], 0, effectiveLength);
		const float travel = float(pos) / effectiveLength;

		SetValue(travel);
	}

	Widget::HandleMouseMove(event);
}

}
