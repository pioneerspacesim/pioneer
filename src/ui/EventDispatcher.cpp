// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "EventDispatcher.h"
#include "Widget.h"
#include "Container.h"
#include <climits>

namespace UI {

static const Uint32 KEY_REPEAT_PAUSE    = 500;
static const Uint32 KEY_REPEAT_INTERVAL = 50;

static inline MouseButtonEvent::ButtonType MouseButtonFromSDLButton(Uint8 sdlButton) {
	return
		sdlButton == SDL_BUTTON_LEFT   ? MouseButtonEvent::BUTTON_LEFT   :
		sdlButton == SDL_BUTTON_MIDDLE ? MouseButtonEvent::BUTTON_MIDDLE :
		                                 MouseButtonEvent::BUTTON_RIGHT;
}

bool EventDispatcher::DispatchSDLEvent(const SDL_Event &event)
{
	switch (event.type) {
		case SDL_KEYDOWN:
			return Dispatch(KeyboardEvent(KeyboardEvent::KEY_DOWN, KeySym(event.key.keysym.sym, event.key.keysym.mod, event.key.keysym.unicode)));

		case SDL_KEYUP:
			return Dispatch(KeyboardEvent(KeyboardEvent::KEY_UP, KeySym(event.key.keysym.sym, event.key.keysym.mod, event.key.keysym.unicode)));

		case SDL_MOUSEBUTTONDOWN:
			if (event.button.button == SDL_BUTTON_WHEELUP || event.button.button == SDL_BUTTON_WHEELDOWN)
				return Dispatch(MouseWheelEvent(event.button.button == SDL_BUTTON_WHEELUP ? MouseWheelEvent::WHEEL_UP : MouseWheelEvent::WHEEL_DOWN, Point(event.button.x,event.button.y)));
			return Dispatch(MouseButtonEvent(MouseButtonEvent::BUTTON_DOWN, MouseButtonFromSDLButton(event.button.button), Point(event.button.x,event.button.y)));

		case SDL_MOUSEBUTTONUP:
			if (event.button.button == SDL_BUTTON_WHEELUP || event.button.button == SDL_BUTTON_WHEELDOWN)
				return false;
			return Dispatch(MouseButtonEvent(MouseButtonEvent::BUTTON_UP, MouseButtonFromSDLButton(event.button.button), Point(event.button.x,event.button.y)));

		case SDL_MOUSEMOTION:
			return Dispatch(MouseMotionEvent(Point(event.motion.x,event.motion.y), Point(event.motion.xrel, event.motion.yrel)));
	}

	return false;
}

bool EventDispatcher::Dispatch(const Event &event)
{
	switch (event.type) {

		case Event::KEYBOARD: {
			const KeyboardEvent keyEvent = static_cast<const KeyboardEvent&>(event);
			switch (keyEvent.action) {
				case KeyboardEvent::KEY_DOWN: {
					bool handled = m_baseContainer->TriggerKeyDown(keyEvent);

					// if there's no keysym then this is some kind of
					// synthesized event from the window system (eg a compose
					// sequence). still dispatch it, but don't repeat because
					// we may never see a corresponding keyup for it
					if (keyEvent.keysym.sym == SDLK_UNKNOWN) {
						Dispatch(KeyboardEvent(KeyboardEvent::KEY_PRESS, keyEvent.keysym));
						return handled;
					}

					m_keyRepeatSym = keyEvent.keysym;
					m_keyRepeatActive = true;
					m_nextKeyRepeat = SDL_GetTicks() + KEY_REPEAT_PAUSE;

					Dispatch(KeyboardEvent(KeyboardEvent::KEY_PRESS, m_keyRepeatSym));

					return handled;
				}

				case KeyboardEvent::KEY_UP: {
					if (m_keyRepeatActive && keyEvent.keysym == m_keyRepeatSym)
						m_keyRepeatActive = false;

					ShortcutMap::iterator i = m_shortcuts.find(keyEvent.keysym);
					if (i != m_shortcuts.end()) {
						(*i).second->TriggerClick();
						DispatchSelect((*i).second);
						return true;
					}
					return m_baseContainer->TriggerKeyUp(keyEvent);
				}

				case KeyboardEvent::KEY_PRESS: {
					Widget *target = m_selected ? m_selected.Get() : m_baseContainer;
					target->TriggerKeyPress(keyEvent);
				}
			}
			return false;
		}

		case Event::MOUSE_BUTTON: {
			const MouseButtonEvent mouseButtonEvent = static_cast<const MouseButtonEvent&>(event);
			m_lastMousePosition = mouseButtonEvent.pos;

			RefCountedPtr<Widget> target(m_baseContainer->GetWidgetAtAbsolute(m_lastMousePosition));

			switch (mouseButtonEvent.action) {

				case MouseButtonEvent::BUTTON_DOWN: {
					if (target->IsDisabled())
						return false;

					// activate widget and remember it
					if (!m_mouseActiveReceiver) {
						m_mouseActiveReceiver = target;
						m_mouseActiveTrigger = mouseButtonEvent.button;
						target->TriggerMouseActivate();
					}

					MouseButtonEvent translatedEvent = MouseButtonEvent(mouseButtonEvent.action, mouseButtonEvent.button, m_lastMousePosition-target->GetAbsolutePosition());
					return target->TriggerMouseDown(translatedEvent);
				}

				case MouseButtonEvent::BUTTON_UP: {

					// if there's an active widget, deactivate it
					if (m_mouseActiveReceiver && m_mouseActiveTrigger == mouseButtonEvent.button) {
						m_mouseActiveReceiver->TriggerMouseDeactivate();

						// if we released over the active widget, then we clicked it
						if (m_mouseActiveReceiver.Get() == target) {
							m_mouseActiveReceiver->TriggerClick();
							DispatchSelect(m_mouseActiveReceiver.Get());
						}

						m_mouseActiveReceiver.Reset();

						// send the straight up event too
						bool ret = false;
						if (!target->IsDisabled()) {
							MouseButtonEvent translatedEvent = MouseButtonEvent(mouseButtonEvent.action, mouseButtonEvent.button, m_lastMousePosition-target->GetAbsolutePosition());
							ret = target->TriggerMouseUp(translatedEvent);
						}

						DispatchMouseOverOut(target.Get(), m_lastMousePosition);

						return ret;
					}

					MouseButtonEvent translatedEvent = MouseButtonEvent(mouseButtonEvent.action, mouseButtonEvent.button, m_lastMousePosition-target->GetAbsolutePosition());
					return target->TriggerMouseUp(translatedEvent);
				}

				default:
					return false;
			}
		}

		case Event::MOUSE_MOTION: {
			const MouseMotionEvent mouseMotionEvent = static_cast<const MouseMotionEvent&>(event);
			m_lastMousePosition = mouseMotionEvent.pos;

			// if there's a mouse-active widget, just send motion events directly into it
			if (m_mouseActiveReceiver) {
				MouseMotionEvent translatedEvent = MouseMotionEvent(m_lastMousePosition-m_mouseActiveReceiver->GetAbsolutePosition(), mouseMotionEvent.rel);
				return m_mouseActiveReceiver->TriggerMouseMove(translatedEvent);
			}

			// widget directly under the mouse
			RefCountedPtr<Widget> target(m_baseContainer->GetWidgetAtAbsolute(m_lastMousePosition));

			bool ret = false;
			if (!target->IsDisabled()) {
				MouseMotionEvent translatedEvent = MouseMotionEvent(m_lastMousePosition-target->GetAbsolutePosition(), mouseMotionEvent.rel);
				ret = target->TriggerMouseMove(translatedEvent);
			}

			DispatchMouseOverOut(target.Get(), m_lastMousePosition);

			return ret;
		}

		case Event::MOUSE_WHEEL: {
			const MouseWheelEvent mouseWheelEvent = static_cast<const MouseWheelEvent&>(event);
			m_lastMousePosition = mouseWheelEvent.pos;

			RefCountedPtr<Widget> target(m_baseContainer->GetWidgetAtAbsolute(m_lastMousePosition));
			return target->TriggerMouseWheel(mouseWheelEvent);
		}

		default:
			return false;
	}

	return false;
}

void EventDispatcher::DispatchMouseOverOut(Widget *target, const Point &mousePos)
{
	// do over/out handling for wherever the mouse is right now
	if (target != m_lastMouseOverTarget.Get() || target->IsDisabled()) {

		if (m_lastMouseOverTarget) {

			// if we're switching from float to non-float then we need to force the out event, even if the mouse is still over the last target.

			// only the base widget of a floating stack is marked floating, so walk up to find it
			// XXX this is doing too much work. should we flag this on the widget somewhere?
			Widget *targetBase = target;
			while (!targetBase->IsFloating() && targetBase->GetContainer()) targetBase = targetBase->GetContainer();
			Widget *lastTargetBase = m_lastMouseOverTarget.Get();
			while (!lastTargetBase->IsFloating() && lastTargetBase->GetContainer()) lastTargetBase = lastTargetBase->GetContainer();

			// if we're moving from float->non-float or non-float->float,
			// or the two targets don't have the same base (eg one just got
			// removed from the context in whole ui switch)
			// force the out event on the last target by reporting a position
			// that is by definition outside itself
			const Point outPos =
				(targetBase->IsFloating() != lastTargetBase->IsFloating() || targetBase != lastTargetBase) ? Point(-INT_MAX) : mousePos-m_lastMouseOverTarget->GetAbsolutePosition();
			m_lastMouseOverTarget->TriggerMouseOut(outPos);
		}

		if (target->IsDisabled())
			m_lastMouseOverTarget.Reset(0);
		else {
			m_lastMouseOverTarget.Reset(target);
			m_lastMouseOverTarget->TriggerMouseOver(mousePos-m_lastMouseOverTarget->GetAbsolutePosition());
		}
	}
}

void EventDispatcher::DispatchSelect(Widget *target)
{
	if (m_selected) {
		if (m_selected == target)
			return;

		m_selected->TriggerDeselect();
	}

	while (target) {
		if (target->IsSelectable()) {
			m_selected.Reset(target);
			m_selected->TriggerSelect();
			return;
		}

		target = target->GetContainer();
	}

	m_selected.Reset();
}

void EventDispatcher::SelectWidget(Widget *target)
{
	DispatchSelect(target);
}

void EventDispatcher::DeselectWidget(Widget *target)
{
	if (!target->IsSelected())
		return;
	DispatchSelect(0);
}

void EventDispatcher::DisableWidget(Widget *target)
{
	DeselectWidget(target);

	if (m_mouseActiveReceiver && m_mouseActiveReceiver.Get() == target) {
		m_mouseActiveReceiver->TriggerMouseDeactivate();
		m_mouseActiveReceiver.Reset();
	}

	// if the mouse is over the target, then the mouse is also over all of the
	// children. find the top one and deliver a MouseOut event to them all
	if (target->IsMouseOver()) {
		RefCountedPtr<Widget> top(m_baseContainer->GetWidgetAtAbsolute(m_lastMousePosition));
		top->TriggerMouseOut(top->GetAbsolutePosition(), true, target); // stop at target
		m_lastMouseOverTarget.Reset(0);
	}
}

void EventDispatcher::EnableWidget(Widget *target)
{
	RefCountedPtr<Widget> top(m_baseContainer->GetWidgetAtAbsolute(m_lastMousePosition));
	DispatchMouseOverOut(top.Get(), m_lastMousePosition);
}

void EventDispatcher::Update()
{
	if (!m_keyRepeatActive) return;

	Uint32 now = SDL_GetTicks();
	if (m_nextKeyRepeat <= now) {
		Dispatch(KeyboardEvent(KeyboardEvent::KEY_PRESS, m_keyRepeatSym));
		m_nextKeyRepeat = now + KEY_REPEAT_INTERVAL;
	}
}

void EventDispatcher::LayoutUpdated()
{
	RefCountedPtr<Widget> target(m_baseContainer->GetWidgetAtAbsolute(m_lastMousePosition));
	DispatchMouseOverOut(target.Get(), m_lastMousePosition);
}

void EventDispatcher::AddShortcut(const KeySym &keysym, Widget *target)
{
	m_shortcuts[keysym] = target;
}

void EventDispatcher::RemoveShortcut(const KeySym &keysym)
{
	ShortcutMap::iterator i = m_shortcuts.find(keysym);
	if (i != m_shortcuts.end())
		m_shortcuts.erase(i);
}

void EventDispatcher::ClearShortcuts()
{
	m_shortcuts.clear();
}

}
