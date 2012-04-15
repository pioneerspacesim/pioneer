#include "Widget.h"
#include "Container.h"

namespace UI {

Widget::Widget(Context *context) : m_context(context), m_container(0), m_position(0), m_size(0), m_style(0)
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

bool Widget::HandleKeyDown(const KeyboardEvent &event)
{
	return onKeyDown.emit(event);
}

bool Widget::HandleKeyUp(const KeyboardEvent &event)
{
	return onKeyUp.emit(event);
}

bool Widget::HandleMouseDown(const MouseButtonEvent &event)
{
	if (!Contains(event.pos)) return false;
	MouseButtonEvent translatedEvent = MouseButtonEvent(event.action, event.button, event.pos-GetPosition());
	return onMouseDown.emit(translatedEvent);
}

bool Widget::HandleMouseUp(const MouseButtonEvent &event)
{
	if (!Contains(event.pos)) return false;
	MouseButtonEvent translatedEvent = MouseButtonEvent(event.action, event.button, event.pos-GetPosition());
	return onMouseUp.emit(translatedEvent);
}

bool Widget::HandleMouseMove(const MouseMotionEvent &event)
{
	if (!Contains(event.pos)) return false;
	MouseMotionEvent translatedEvent = MouseMotionEvent(event.pos-GetPosition());
	return onMouseMove.emit(translatedEvent);
}

bool Widget::HandleMouseWheel(const MouseWheelEvent &event)
{
	return onMouseWheel.emit(event);
}

void Widget::HandleClick()
{
	onClick.emit();
}

}
