#include "Widget.h"
#include "Container.h"

namespace UI {

Widget::Widget(Context *context) : m_context(context), m_container(0), m_position(0), m_size(0), m_activeArea(0), m_mouseOver(false), m_mouseActive(false)
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

vector2f Widget::GetAbsolutePosition() const
{
	if (!m_container) return 0;
	return m_container->GetAbsolutePosition() + m_position;
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
	m_position = 0;
	m_size = 0;
}

void Widget::SetDimensions(const vector2f &position, const vector2f &size)
{
	m_position = position;
	SetSize(size);
	SetActiveArea(size);
}

bool Widget::HandleKeyDown(const KeyboardEvent &event, bool emit)
{
	if (emit) emit = !onKeyDown.emit(event);
	if (GetContainer()) GetContainer()->HandleKeyDown(event, emit);
	return emit;
}

bool Widget::HandleKeyUp(const KeyboardEvent &event, bool emit)
{
	if (emit) emit = !onKeyUp.emit(event);
	if (GetContainer()) GetContainer()->HandleKeyUp(event, emit);
	return emit;
}

bool Widget::HandleMouseDown(const MouseButtonEvent &event, bool emit)
{
	MouseButtonEvent translatedEvent = MouseButtonEvent(event.action, event.button, event.pos-GetPosition());
	if (emit) emit = !onMouseDown.emit(translatedEvent);
	if (GetContainer()) GetContainer()->HandleMouseDown(translatedEvent, emit);
	return emit;
}

bool Widget::HandleMouseUp(const MouseButtonEvent &event, bool emit)
{
	MouseButtonEvent translatedEvent = MouseButtonEvent(event.action, event.button, event.pos-GetPosition());
	if (emit) emit = !onMouseUp.emit(translatedEvent);
	if (GetContainer()) GetContainer()->HandleMouseUp(translatedEvent, emit);
	return emit;
}

bool Widget::HandleMouseMove(const MouseMotionEvent &event, bool emit)
{
	MouseMotionEvent translatedEvent = MouseMotionEvent(event.pos-GetPosition());
	if (emit) emit = !onMouseMove.emit(translatedEvent);
	if (GetContainer()) GetContainer()->HandleMouseMove(translatedEvent, emit);
	return emit;
}

bool Widget::HandleMouseWheel(const MouseWheelEvent &event, bool emit)
{
	MouseWheelEvent translatedEvent = MouseWheelEvent(event.direction, event.pos-GetPosition());
	if (emit) emit = !onMouseWheel.emit(translatedEvent);
	if (GetContainer()) GetContainer()->HandleMouseWheel(translatedEvent, emit);
	return emit;
}

bool Widget::HandleMouseOver(const vector2f &pos, bool emit)
{
	// only send external events on state change
	if (!m_mouseOver && Contains(pos)) {
		m_mouseOver = true;
		if (emit) emit = !onMouseOver.emit();
	}
	if (GetContainer()) GetContainer()->HandleMouseOver(pos+GetPosition(), emit);
	return emit;
}

bool Widget::HandleMouseOut(const vector2f &pos, bool emit)
{
	// only send external events on state change
	if (m_mouseOver && !Contains(pos)) {
		if (emit) emit = !onMouseOut.emit();
		m_mouseOver = false;
	}
	if (GetContainer()) GetContainer()->HandleMouseOut(pos+GetPosition(), emit);
	return emit;
}

bool Widget::HandleClick(bool emit)
{
	if (emit) emit = !onClick.emit();
	if (GetContainer()) GetContainer()->HandleClick(emit);
	return emit;
}

void Widget::MouseActivate()
{
	m_mouseActive = true;
	if (GetContainer()) GetContainer()->MouseActivate();
}

void Widget::MouseDeactivate()
{
	if (GetContainer()) GetContainer()->MouseDeactivate();
	m_mouseActive = false;
}

}
