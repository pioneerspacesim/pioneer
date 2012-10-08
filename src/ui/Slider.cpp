// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Slider.h"
#include "Context.h"

namespace UI {

static const Uint32 MIN_SLIDER_INNER_SIZE = 32; // XXX skin

Point Slider::PreferredSize()
{
	// XXX use slider gutter size
	if (m_orient == SLIDER_HORIZONTAL)
		return Point(MIN_SLIDER_INNER_SIZE+GetContext()->GetSkin().ButtonNormal().borderWidth);
	else
		return Point(MIN_SLIDER_INNER_SIZE+GetContext()->GetSkin().ButtonNormal().borderWidth);
}

void Slider::Layout()
{
	UpdateButton();
	Widget::Layout();
}

void Slider::UpdateButton()
{
	const Uint32 buttonBorderWidth = GetContext()->GetSkin().ButtonNormal().borderWidth;
	const Point activeArea(GetActiveArea());
	if (m_orient == SLIDER_HORIZONTAL) {
		m_buttonSize = Point(std::min(activeArea.x-buttonBorderWidth*2, MIN_SLIDER_INNER_SIZE), activeArea.y-buttonBorderWidth*2);
		m_buttonPos = Point((activeArea.x-buttonBorderWidth*2-m_buttonSize.x)*m_value+buttonBorderWidth, buttonBorderWidth);
	}
	else {
		m_buttonSize = Point(activeArea.x-buttonBorderWidth*2, std::min(activeArea.y-buttonBorderWidth*2, MIN_SLIDER_INNER_SIZE));
		m_buttonPos = Point(buttonBorderWidth, (activeArea.y-buttonBorderWidth*2-m_buttonSize.y)*m_value+buttonBorderWidth);
	}
}

void Slider::Draw()
{
	GetContext()->GetSkin().DrawButtonActive(GetActiveOffset(), GetActiveArea());        // XXX gutter
	if (m_buttonDown && IsMouseActive())
		GetContext()->GetSkin().DrawButtonActive(GetActiveOffset()+m_buttonPos, m_buttonSize); // XXX button
	else
		GetContext()->GetSkin().DrawButtonNormal(GetActiveOffset()+m_buttonPos, m_buttonSize); // XXX button
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
