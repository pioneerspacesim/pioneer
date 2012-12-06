// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Widget.h"
#include "Container.h"
#include "Context.h"

namespace UI {

Widget::Widget(Context *context) :
	m_context(context),
	m_container(0),
	m_position(0),
	m_size(0),
	m_sizeControlFlags(0),
	m_drawOffset(0),
	m_activeOffset(0),
	m_activeArea(0),
	m_font(FONT_INHERIT),
	m_floating(false),
	m_disabled(false),
	m_mouseOver(false),
	m_mouseActive(false)
{
	assert(m_context);
}

Widget::~Widget()
{
	// a container owns its widgets. this ensures that only the container can
	// delete the widget by requiring it to clear the widgets references
	// before deletion
	assert(!m_container);
}

Point Widget::GetAbsolutePosition() const
{
	if (IsFloating()) return m_position + m_drawOffset;
	if (!m_container) return Point() + m_drawOffset;
	return m_container->GetAbsolutePosition() + m_position + m_drawOffset;
}

void Widget::Attach(Container *container)
{
	assert(m_context == container->GetContext());
	assert(container);
	m_container = container;
}

void Widget::Detach()
{
	m_container = 0;
	m_position = Point();
	m_size = Point();
	m_floating = false;
}

void Widget::SetDimensions(const Point &position, const Point &size)
{
	m_position = position;
	SetSize(size);
	SetActiveArea(size);
}

void Widget::SetActiveArea(const Point &activeArea, const Point &activeOffset)
{
	m_activeArea = Point(Clamp(activeArea.x, 0, GetSize().x), Clamp(activeArea.y, 0, GetSize().y));
	m_activeOffset = activeOffset;
}

Widget *Widget::SetFont(Font font)
{
	m_font = font;
	GetContext()->RequestLayout();
	return this;
}

Widget::Font Widget::GetFont() const
{
	if (m_font == FONT_INHERIT) {
		if (m_container) return m_container->GetFont();
		return FONT_NORMAL;
	}
	return m_font;
}

void Widget::Disable()
{
	SetDisabled(true);
	GetContext()->DisableWidget(this);
}

void Widget::Enable()
{
	SetDisabled(false);
	GetContext()->EnableWidget(this);
}

bool Widget::TriggerKeyDown(const KeyboardEvent &event, bool emit)
{
	HandleKeyDown(event);
	if (emit) emit = !onKeyDown.emit(event);
	if (GetContainer() && !IsFloating()) GetContainer()->TriggerKeyDown(event, emit);
	return !emit;
}

bool Widget::TriggerKeyUp(const KeyboardEvent &event, bool emit)
{
	HandleKeyUp(event);
	if (emit) emit = !onKeyUp.emit(event);
	if (GetContainer() && !IsFloating()) GetContainer()->TriggerKeyUp(event, emit);
	return !emit;
}

bool Widget::TriggerKeyPress(const KeyboardEvent &event, bool emit)
{
	HandleKeyPress(event);
	if (emit) emit = !onKeyDown.emit(event);
	if (GetContainer() && !IsFloating()) GetContainer()->TriggerKeyPress(event, emit);
	return !emit;
}

bool Widget::TriggerMouseDown(const MouseButtonEvent &event, bool emit)
{
	HandleMouseDown(event);
	if (emit) emit = !onMouseDown.emit(event);
	if (GetContainer() && !IsFloating()) {
		MouseButtonEvent translatedEvent = MouseButtonEvent(event.action, event.button, event.pos+GetPosition());
		GetContainer()->TriggerMouseDown(translatedEvent, emit);
	}
	return !emit;
}

bool Widget::TriggerMouseUp(const MouseButtonEvent &event, bool emit)
{
	HandleMouseUp(event);
	if (emit) emit = !onMouseUp.emit(event);
	if (GetContainer() && !IsFloating()) {
		MouseButtonEvent translatedEvent = MouseButtonEvent(event.action, event.button, event.pos+GetPosition());
		GetContainer()->TriggerMouseUp(translatedEvent, emit);
	}
	return !emit;
}

bool Widget::TriggerMouseMove(const MouseMotionEvent &event, bool emit)
{
	HandleMouseMove(event);
	if (emit) emit = !onMouseMove.emit(event);
	if (GetContainer() && !IsFloating()) {
		MouseMotionEvent translatedEvent = MouseMotionEvent(event.pos+GetPosition(), event.rel);
		GetContainer()->TriggerMouseMove(translatedEvent, emit);
	}
	return !emit;
}

bool Widget::TriggerMouseWheel(const MouseWheelEvent &event, bool emit)
{
	HandleMouseWheel(event);
	if (emit) emit = !onMouseWheel.emit(event);
	if (GetContainer() && !IsFloating()) {
		MouseWheelEvent translatedEvent = MouseWheelEvent(event.direction, event.pos+GetPosition());
		GetContainer()->TriggerMouseWheel(translatedEvent, emit);
	}
	return !emit;
}

bool Widget::TriggerMouseOver(const Point &pos, bool emit, Widget *stop)
{
	// only send external events on state change
	if (!m_mouseOver && Contains(pos)) {
		m_mouseOver = true;
		HandleMouseOver();
		if (emit) emit = !onMouseOver.emit();
	}
	if (stop == this) return !emit;
	if (GetContainer() && !IsFloating()) GetContainer()->TriggerMouseOver(pos+GetPosition(), emit, stop);
	return !emit;
}

bool Widget::TriggerMouseOut(const Point &pos, bool emit, Widget *stop)
{
	// only send external events on state change
	if (m_mouseOver && !Contains(pos)) {
		HandleMouseOut();
		if (emit) emit = !onMouseOut.emit();
		m_mouseOver = false;
	}
	if (stop == this) return !emit;
	if (GetContainer() && !IsFloating()) GetContainer()->TriggerMouseOut(pos+GetPosition(), emit, stop);
	return !emit;
}

bool Widget::TriggerClick(bool emit)
{
	HandleClick();
	if (emit) emit = !onClick.emit();
	if (GetContainer() && !IsFloating()) GetContainer()->TriggerClick(emit);
	return !emit;
}

void Widget::TriggerMouseActivate()
{
	m_mouseActive = true;
	HandleMouseActivate();
	if (GetContainer() && !IsFloating()) GetContainer()->TriggerMouseActivate();
}

void Widget::TriggerMouseDeactivate()
{
	m_mouseActive = false;
	HandleMouseDeactivate();
	if (GetContainer() && !IsFloating()) GetContainer()->TriggerMouseDeactivate();
}

void Widget::TriggerSelect()
{
	m_selected = true;
	HandleSelect();
}

void Widget::TriggerDeselect()
{
	m_selected = false;
	HandleDeselect();
}

}
