#include "EventDispatcher.h"
#include "Widget.h"
#include "Container.h"


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
			return Dispatch(KeyboardEvent(KeyboardEvent::KEY_DOWN, event.key.keysym));

		case SDL_KEYUP:
			return Dispatch(KeyboardEvent(KeyboardEvent::KEY_UP, event.key.keysym));

		case SDL_MOUSEBUTTONDOWN:
			if (event.button.button == SDL_BUTTON_WHEELUP || event.button.button == SDL_BUTTON_WHEELDOWN)
				return Dispatch(MouseWheelEvent(event.button.button == SDL_BUTTON_WHEELUP ? MouseWheelEvent::WHEEL_UP : MouseWheelEvent::WHEEL_DOWN, vector2f(event.button.x,event.button.y)));
			return Dispatch(MouseButtonEvent(MouseButtonEvent::BUTTON_DOWN, MouseButtonFromSDLButton(event.button.button), vector2f(event.button.x,event.button.y)));

		case SDL_MOUSEBUTTONUP:
			if (event.button.button == SDL_BUTTON_WHEELUP || event.button.button == SDL_BUTTON_WHEELDOWN)
				return false;
			return Dispatch(MouseButtonEvent(MouseButtonEvent::BUTTON_UP, MouseButtonFromSDLButton(event.button.button), vector2f(event.button.x,event.button.y)));

		case SDL_MOUSEMOTION:
			return Dispatch(MouseMotionEvent(vector2f(event.motion.x,event.motion.y)));
	}

	return false;
}

bool EventDispatcher::Dispatch(const Event &event)
{
	switch (event.type) {
		case Event::KEYBOARD: {
			const KeyboardEvent keyEvent = static_cast<const KeyboardEvent&>(event);
			switch (keyEvent.action) {
				case KeyboardEvent::KEY_DOWN: return m_baseContainer->HandleKeyDown(keyEvent);
				case KeyboardEvent::KEY_UP:   return m_baseContainer->HandleKeyUp(keyEvent);
			}
			return false;
		}

		case Event::MOUSE_BUTTON: {
			const MouseButtonEvent mouseButtonEvent = static_cast<const MouseButtonEvent&>(event);
			switch (mouseButtonEvent.action) {
				case MouseButtonEvent::BUTTON_DOWN: {
					assert(!m_mouseDownReceiver);
					m_mouseDownReceiver = m_baseContainer->GetWidgetAt(mouseButtonEvent.pos);
					return m_baseContainer->HandleMouseDown(mouseButtonEvent);
				}
				case MouseButtonEvent::BUTTON_UP: {
					if (m_mouseDownReceiver && m_mouseDownReceiver == m_baseContainer->GetWidgetAt(mouseButtonEvent.pos))
						m_mouseDownReceiver->HandleClick();
					m_mouseDownReceiver = 0;
					return m_baseContainer->HandleMouseUp(mouseButtonEvent);
				}
			}
			return false;
		}

		case Event::MOUSE_MOTION: {
			const MouseMotionEvent mouseMotionEvent = static_cast<const MouseMotionEvent&>(event);
			return m_baseContainer->HandleMouseMove(mouseMotionEvent);
		}

		case Event::MOUSE_WHEEL: {
			const MouseWheelEvent mouseWheelEvent = static_cast<const MouseWheelEvent&>(event);
			return m_baseContainer->HandleMouseWheel(mouseWheelEvent);
		}

		default:
			return false;
	}

	return false;
}

}
