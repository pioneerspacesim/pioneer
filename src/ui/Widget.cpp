// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
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

	for (std::map<std::string,sigc::connection>::iterator i = m_binds.begin(); i != m_binds.end(); ++i)
		(*i).second.disconnect();
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

Point Widget::CalcLayoutContribution()
{
	Point preferredSize = PreferredSize();
	const Uint32 flags = GetSizeControlFlags();

	if (flags & NO_WIDTH)
		preferredSize.x = 0;
	if (flags & NO_HEIGHT)
		preferredSize.y = 0;

	if (flags & EXPAND_WIDTH)
		preferredSize.x = SIZE_EXPAND;
	if (flags & EXPAND_HEIGHT)
		preferredSize.y = SIZE_EXPAND;

	return preferredSize;
}

Point Widget::CalcSize(const Point &avail)
{
	if (!(GetSizeControlFlags() & PRESERVE_ASPECT))
		return avail;

	const Point preferredSize = PreferredSize();

	float wantRatio = float(preferredSize.x) / float(preferredSize.y);

	// more room on X than Y, use full X, scale Y
	if (avail.x > avail.y)
		return Point(float(avail.y) * wantRatio, avail.y);

	// more room on Y than X, use full Y, scale X
	else
		return Point(avail.x, float(avail.x) / wantRatio);
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
	if (emit) emit = !onKeyPress.emit(event);
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

void Widget::RegisterBindPoint(const std::string &bindName, sigc::slot<void,PropertyMap &,const std::string &> method)
{
	m_bindPoints[bindName] = method;
}

void Widget::Bind(const std::string &bindName, PropertiedObject *object, const std::string &propertyName)
{
	std::map< std::string,sigc::slot<void,PropertyMap &,const std::string &> >::const_iterator bindPointIter = m_bindPoints.find(bindName);
	if (bindPointIter == m_bindPoints.end())
		return;

	sigc::connection conn = object->Properties().Connect(propertyName, (*bindPointIter).second);

	std::map<std::string,sigc::connection>::iterator bindIter = m_binds.find(bindName);
	if (bindIter != m_binds.end()) {
		(*bindIter).second.disconnect();
		(*bindIter).second = conn;
	}
	else
		m_binds.insert(bindIter, std::make_pair(bindName, conn));

	(*bindPointIter).second(object->Properties(), propertyName);
}

}
