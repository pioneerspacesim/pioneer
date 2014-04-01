// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
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
	m_disabled(false),
	m_mouseOver(false),
	m_visible(false),
	m_animatedOpacity(1.0f),
	m_animatedPositionX(1.0f),
	m_animatedPositionY(1.0f)
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
	if (!m_container) return Point() + m_drawOffset;
	return m_container->GetAbsolutePosition() + m_position + m_drawOffset;
}

Point Widget::GetMousePos() const
{
	return m_context->GetMousePos() - GetAbsolutePosition();
}

void Widget::Attach(Container *container)
{
	assert(container);
	assert(m_context == container->GetContext());
	m_container = container;

	// we should never be visible while we're detached, and we should
	// always be detached before being attached to something else
	assert(!m_visible);
	NotifyVisible(container->IsVisible());
}

void Widget::Detach()
{
	NotifyVisible(false);
	m_container = 0;
	m_position = Point();
	m_size = Point();
}

void Widget::SetDimensions(const Point &position, const Point &size)
{
	m_position = position;
	SetSize(size);
	SetActiveArea(size);
}

void Widget::NotifyVisible(bool visible)
{
	if (m_visible != visible) {
		m_visible = visible;
		if (m_visible) { HandleVisible(); } else { HandleInvisible(); }
	}
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

bool Widget::IsMouseActive() const
{
	return (GetContext()->GetMouseActive() == this);
}

bool Widget::IsSelected() const
{
	return (GetContext()->GetSelected() == this);
}

bool Widget::IsOnTopLayer() const
{
	const Layer *topLayer = GetContext()->GetTopLayer();
	Widget *scan = GetContainer();
	while (scan) {
		if (scan == topLayer)
			return true;
        scan = scan->GetContainer();
    }
	return false;
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

bool Widget::TriggerKeyDown(const KeyboardEvent &event, bool handled)
{
	HandleKeyDown(event);
	if (!handled) handled = onKeyDown.emit(event);
	if (GetContainer()) handled = GetContainer()->TriggerKeyDown(event, handled);
	return handled;
}

bool Widget::TriggerKeyUp(const KeyboardEvent &event, bool handled)
{
	HandleKeyUp(event);
	if (!handled) handled = onKeyUp.emit(event);
	if (GetContainer()) handled = GetContainer()->TriggerKeyUp(event, handled);
	return handled;
}

bool Widget::TriggerTextInput(const TextInputEvent &event, bool handled)
{
	HandleTextInput(event);
	if (!handled) handled = onTextInput.emit(event);
	if (GetContainer()) handled = GetContainer()->TriggerTextInput(event, handled);
	return handled;
}

bool Widget::TriggerMouseDown(const MouseButtonEvent &event, bool handled)
{
	HandleMouseDown(event);
	if (!handled) handled = onMouseDown.emit(event);
	if (GetContainer()) {
		MouseButtonEvent translatedEvent = MouseButtonEvent(event.action, event.button, event.pos+GetPosition());
		handled = GetContainer()->TriggerMouseDown(translatedEvent, handled);
	}
	return handled;
}

bool Widget::TriggerMouseUp(const MouseButtonEvent &event, bool handled)
{
	HandleMouseUp(event);
	if (!handled) handled = onMouseUp.emit(event);
	if (GetContainer()) {
		MouseButtonEvent translatedEvent = MouseButtonEvent(event.action, event.button, event.pos+GetPosition());
		handled = GetContainer()->TriggerMouseUp(translatedEvent, handled);
	}
	return handled;
}

bool Widget::TriggerMouseMove(const MouseMotionEvent &event, bool handled)
{
	HandleMouseMove(event);
	if (!handled) handled = onMouseMove.emit(event);
	if (GetContainer()) {
		MouseMotionEvent translatedEvent = MouseMotionEvent(event.pos+GetPosition(), event.rel);
		handled = GetContainer()->TriggerMouseMove(translatedEvent, handled);
	}
	return handled;
}

bool Widget::TriggerMouseWheel(const MouseWheelEvent &event, bool handled)
{
	HandleMouseWheel(event);
	if (!handled) handled = onMouseWheel.emit(event);
	if (GetContainer()) {
		MouseWheelEvent translatedEvent = MouseWheelEvent(event.direction, event.pos+GetPosition());
		handled = GetContainer()->TriggerMouseWheel(translatedEvent, handled);
	}
	return handled;
}

bool Widget::TriggerJoystickButtonDown(const JoystickButtonEvent &event, bool handled)
{
	HandleJoystickButtonDown(event);
	if (!handled) handled = onJoystickButtonDown.emit(event);
	if (GetContainer()) handled = GetContainer()->TriggerJoystickButtonDown(event, handled);
	return handled;
}

bool Widget::TriggerJoystickButtonUp(const JoystickButtonEvent &event, bool handled)
{
	HandleJoystickButtonUp(event);
	if (!handled) handled = onJoystickButtonUp.emit(event);
	if (GetContainer()) handled = GetContainer()->TriggerJoystickButtonUp(event, handled);
	return handled;
}

bool Widget::TriggerJoystickAxisMove(const JoystickAxisMotionEvent &event, bool handled)
{
	HandleJoystickAxisMove(event);
	if (!handled) handled = onJoystickAxisMove.emit(event);
	if (GetContainer()) handled = GetContainer()->TriggerJoystickAxisMove(event, handled);
	return handled;
}

bool Widget::TriggerJoystickHatMove(const JoystickHatMotionEvent &event, bool handled)
{
	HandleJoystickHatMove(event);
	if (!handled) handled = onJoystickHatMove.emit(event);
	if (GetContainer()) handled = GetContainer()->TriggerJoystickHatMove(event, handled);
	return handled;
}

bool Widget::TriggerMouseOver(const Point &pos, bool handled, Widget *stop)
{
	// only send external events on state change
	if (!m_mouseOver) {
		m_mouseOver = true;
		HandleMouseOver();
		if (!handled) handled = onMouseOver.emit();
	}
	if (stop == this) return handled;
	if (GetContainer()) handled = GetContainer()->TriggerMouseOver(pos+GetPosition(), handled, stop);
	return handled;
}

bool Widget::TriggerMouseOut(const Point &pos, bool handled, Widget *stop)
{
	// only send external events on state change
	if (m_mouseOver) {
		HandleMouseOut();
		if (!handled) handled = onMouseOut.emit();
		m_mouseOver = false;
	}
	if (stop == this) return handled;
	if (GetContainer()) handled = GetContainer()->TriggerMouseOut(pos+GetPosition(), handled, stop);
	return handled;
}

bool Widget::TriggerClick(bool handled)
{
	HandleClick();
	if (!handled) handled = onClick.emit();
	if (GetContainer()) handled = GetContainer()->TriggerClick(handled);
	return handled;
}

void Widget::TriggerMouseActivate()
{
	HandleMouseActivate();
}

void Widget::TriggerMouseDeactivate()
{
	HandleMouseDeactivate();
}

void Widget::TriggerSelect()
{
	HandleSelect();
}

void Widget::TriggerDeselect()
{
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
