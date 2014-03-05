// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "EventDispatcher.h"
#include "Widget.h"
#include "Container.h"
#include "text/TextSupport.h"
#include <climits>

namespace UI {

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
			return Dispatch(KeyboardEvent(KeyboardEvent::KEY_DOWN, KeySym(event.key.keysym.sym, SDL_Keymod(event.key.keysym.mod)), event.key.repeat));

		case SDL_KEYUP:
			return Dispatch(KeyboardEvent(KeyboardEvent::KEY_UP, KeySym(event.key.keysym.sym, SDL_Keymod(event.key.keysym.mod)), event.key.repeat));

		case SDL_TEXTINPUT:
			Uint32 unicode;
			Text::utf8_decode_char(&unicode, event.text.text);
			return Dispatch(TextInputEvent(unicode));

		case SDL_MOUSEWHEEL:
			return Dispatch(MouseWheelEvent(event.wheel.y > 0 ? MouseWheelEvent::WHEEL_UP : MouseWheelEvent::WHEEL_DOWN, m_lastMousePosition));

		case SDL_MOUSEBUTTONDOWN:
			return Dispatch(MouseButtonEvent(MouseButtonEvent::BUTTON_DOWN, MouseButtonFromSDLButton(event.button.button), Point(event.button.x,event.button.y)));

		case SDL_MOUSEBUTTONUP:
			return Dispatch(MouseButtonEvent(MouseButtonEvent::BUTTON_UP, MouseButtonFromSDLButton(event.button.button), Point(event.button.x,event.button.y)));

		case SDL_MOUSEMOTION:
			return Dispatch(MouseMotionEvent(Point(event.motion.x,event.motion.y), Point(event.motion.xrel, event.motion.yrel)));

		case SDL_JOYAXISMOTION:
			// SDL joystick axis value is documented to have the range -32768 to 32767
			// unfortunately this places the centre at -0.5, not at zero, which is clearly nuts...
			// so since that doesn't make any sense, we assume the range is *actually* -32767 to +32767,
			// and scale it accordingly, clamping the output so that if we *do* get -32768, it turns into -1
			return Dispatch(JoystickAxisMotionEvent(event.jaxis.which, Clamp(event.jaxis.value * (1.0f / 32767.0f), -1.0f, 1.0f), event.jaxis.axis));

		case SDL_JOYHATMOTION:
			return Dispatch(JoystickHatMotionEvent(event.jhat.which, JoystickHatMotionEvent::JoystickHatDirection(event.jhat.value), event.jhat.hat));

		case SDL_JOYBUTTONDOWN:
			return Dispatch(JoystickButtonEvent(event.jbutton.which, JoystickButtonEvent::BUTTON_DOWN, event.jbutton.button));

		case SDL_JOYBUTTONUP:
			return Dispatch(JoystickButtonEvent(event.jbutton.which, JoystickButtonEvent::BUTTON_UP, event.jbutton.button));
	}

	return false;
}

bool EventDispatcher::Dispatch(const Event &event)
{
	switch (event.type) {

		case Event::KEYBOARD: {
			const KeyboardEvent keyEvent = static_cast<const KeyboardEvent&>(event);
			switch (keyEvent.action) {
				case KeyboardEvent::KEY_DOWN:

					// all key events to the selected widget first
					if (m_selected && m_selected->IsOnTopLayer())
						return m_selected->TriggerKeyDown(keyEvent);

					return m_baseContainer->TriggerKeyDown(keyEvent);

				case KeyboardEvent::KEY_UP: {

					// all key events to the selected widget first
					if (m_selected && m_selected->IsOnTopLayer())
						return m_selected->TriggerKeyUp(keyEvent);

					// any modifier coming in will be a specific key, eg left
					// shift or right shift. shortcuts can't distinguish
					// betwen the two, and so have both set in m_shortcuts. we
					// can't just compare though, because the mods won't
					// match. so we make a new keysym with a new mod that
					// includes both of the type of key
					Uint32 mod = Uint32(keyEvent.keysym.mod);
					if (mod & KMOD_SHIFT) mod |= KMOD_SHIFT;
					if (mod & KMOD_CTRL)  mod |= KMOD_CTRL;
					if (mod & KMOD_ALT)   mod |= KMOD_ALT;
					if (mod & KMOD_GUI)   mod |= KMOD_GUI;
					const KeySym shortcutSym(keyEvent.keysym.sym, SDL_Keymod(mod));

					std::map<KeySym,Widget*>::iterator i = m_shortcuts.find(shortcutSym);
					if (i != m_shortcuts.end()) {
						// DispatchSelect must happen before TriggerClick, so
						// that Click handlers can override the selection
						DispatchSelect((*i).second);
						(*i).second->TriggerClick();
						return true;
					}

					return m_baseContainer->TriggerKeyUp(keyEvent);
				}

			}
			return false;
		}

		case Event::TEXT_INPUT: {

			const TextInputEvent textInputEvent = static_cast<const TextInputEvent&>(event);

			// selected widgets get all the text input events
			if (m_selected && m_selected->IsOnTopLayer())
				return m_selected->TriggerTextInput(textInputEvent);

			return m_baseContainer->TriggerTextInput(textInputEvent);
		}

		case Event::MOUSE_BUTTON: {
			const MouseButtonEvent mouseButtonEvent = static_cast<const MouseButtonEvent&>(event);
			m_lastMousePosition = mouseButtonEvent.pos;

			RefCountedPtr<Widget> target(m_baseContainer->GetWidgetAt(m_lastMousePosition));

			switch (mouseButtonEvent.action) {

				case MouseButtonEvent::BUTTON_DOWN: {

					if (!target->IsOnTopLayer())
						return false;

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
							// DispatchSelect must happen before TriggerClick, so
							// that Click handlers can override the selection
							DispatchSelect(m_mouseActiveReceiver.Get());
							m_mouseActiveReceiver->TriggerClick();
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

					if (!target->IsOnTopLayer())
						return false;

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
				if (!m_mouseActiveReceiver->IsOnTopLayer())
					return false;

				MouseMotionEvent translatedEvent = MouseMotionEvent(m_lastMousePosition-m_mouseActiveReceiver->GetAbsolutePosition(), mouseMotionEvent.rel);
				return m_mouseActiveReceiver->TriggerMouseMove(translatedEvent);
			}

			// widget directly under the mouse
			RefCountedPtr<Widget> target(m_baseContainer->GetWidgetAt(m_lastMousePosition));

			bool ret = false;
			if (!target->IsDisabled() && target->IsOnTopLayer()) {
				MouseMotionEvent translatedEvent = MouseMotionEvent(m_lastMousePosition-target->GetAbsolutePosition(), mouseMotionEvent.rel);
				ret = target->TriggerMouseMove(translatedEvent);
			}

			DispatchMouseOverOut(target.Get(), m_lastMousePosition);

			return ret;
		}

		case Event::MOUSE_WHEEL: {
			const MouseWheelEvent mouseWheelEvent = static_cast<const MouseWheelEvent&>(event);
			m_lastMousePosition = mouseWheelEvent.pos;

			RefCountedPtr<Widget> target(m_baseContainer->GetWidgetAt(m_lastMousePosition));
			if (!target->IsOnTopLayer())
				return false;

			return target->TriggerMouseWheel(mouseWheelEvent);
		}

		case Event::JOYSTICK_AXIS_MOTION:
			return m_baseContainer->TriggerJoystickAxisMove(static_cast<const JoystickAxisMotionEvent&>(event));

		case Event::JOYSTICK_HAT_MOTION:
			return m_baseContainer->TriggerJoystickHatMove(static_cast<const JoystickHatMotionEvent&>(event));

		case Event::JOYSTICK_BUTTON: {
			const JoystickButtonEvent &joyButtonEvent = static_cast<const JoystickButtonEvent&>(event);
			switch (joyButtonEvent.action) {
				case JoystickButtonEvent::BUTTON_DOWN:
					return m_baseContainer->TriggerJoystickButtonDown(joyButtonEvent);
				case JoystickButtonEvent::BUTTON_UP:
					return m_baseContainer->TriggerJoystickButtonUp(joyButtonEvent);
				default: assert(0); return false;
			}
		}

		default:
			return false;
	}

	return false;
}

void EventDispatcher::DispatchMouseOverOut(Widget *target, const Point &mousePos)
{
	bool onTopLayer = target->IsOnTopLayer();

	// do over/out handling for wherever the mouse is right now
	if (target != m_lastMouseOverTarget.Get() || target->IsDisabled() || !onTopLayer) {

		if (m_lastMouseOverTarget) {

			// tell the old one that the mouse isn't over it anymore
			m_lastMouseOverTarget->TriggerMouseOut(mousePos-m_lastMouseOverTarget->GetAbsolutePosition());
		}

		if (target->IsDisabled() || !onTopLayer)
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
		RefCountedPtr<Widget> top(m_baseContainer->GetWidgetAt(m_lastMousePosition));
		top->TriggerMouseOut(top->GetAbsolutePosition(), true, target); // stop at target
		m_lastMouseOverTarget.Reset(0);
	}
}

void EventDispatcher::EnableWidget(Widget *target)
{
	RefCountedPtr<Widget> top(m_baseContainer->GetWidgetAt(m_lastMousePosition));
	DispatchMouseOverOut(top.Get(), m_lastMousePosition);
}

void EventDispatcher::LayoutUpdated()
{
	if (m_selected && !m_selected->IsVisible())
		m_selected.Reset();

	m_shortcuts.clear();
	m_baseContainer->CollectShortcuts(m_shortcuts);

	RefCountedPtr<Widget> target(m_baseContainer->GetWidgetAt(m_lastMousePosition));
	DispatchMouseOverOut(target.Get(), m_lastMousePosition);
}

}
