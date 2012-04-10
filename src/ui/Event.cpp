#include "Event.h"
#include "Widget.h"
#include "Container.h"
#include <typeinfo>

namespace UI {

static inline MouseButtonEvent::ButtonType MouseButtonFromSDLButton(Uint8 sdlButton) {
	return
		sdlButton == SDL_BUTTON_LEFT   ? MouseButtonEvent::BUTTON_LEFT   :
		sdlButton == SDL_BUTTON_MIDDLE ? MouseButtonEvent::BUTTON_MIDDLE :
		                                 MouseButtonEvent::BUTTON_RIGHT;
}

bool Event::DispatchSDLEvent(const SDL_Event &event, Widget *target)
{
	switch (event.type) {
		case SDL_KEYDOWN:
			return Dispatch(KeyboardEvent(KeyboardEvent::KEY_DOWN, event.key.keysym), target);

		case SDL_KEYUP:
			return Dispatch(KeyboardEvent(KeyboardEvent::KEY_UP, event.key.keysym), target);

		case SDL_MOUSEBUTTONDOWN:
			if (event.button.button == SDL_BUTTON_WHEELUP || event.button.button == SDL_BUTTON_WHEELDOWN)
				return Dispatch(MouseWheelEvent(event.button.button == SDL_BUTTON_WHEELUP ? MouseWheelEvent::WHEEL_UP : MouseWheelEvent::WHEEL_DOWN), target);
			return Dispatch(MouseButtonEvent(MouseButtonEvent::BUTTON_DOWN, MouseButtonFromSDLButton(event.button.button), vector2f(event.button.x,event.button.y)), target);

		case SDL_MOUSEBUTTONUP:
			if (event.button.button == SDL_BUTTON_WHEELUP || event.button.button == SDL_BUTTON_WHEELDOWN)
				return false;
			return Dispatch(MouseButtonEvent(MouseButtonEvent::BUTTON_UP, MouseButtonFromSDLButton(event.button.button), vector2f(event.button.x,event.button.y)), target);

		case SDL_MOUSEMOTION:
			return Dispatch(MouseMotionEvent(vector2f(event.motion.x,event.motion.y)), target);
	}

	return false;
}

bool Event::Dispatch(const Event &event, Widget *target)
{
	switch (event.type) {
		case Event::KEYBOARD: {
			const KeyboardEvent keyEvent = static_cast<const KeyboardEvent&>(event);
			switch (keyEvent.action) {
				case KeyboardEvent::KEY_DOWN: return target->HandleKeyDown(keyEvent);
				case KeyboardEvent::KEY_UP:   return target->HandleKeyUp(keyEvent);
			}
			return false;
		}

		case Event::MOUSE_BUTTON: {
			const MouseButtonEvent mouseButtonEvent = static_cast<const MouseButtonEvent&>(event);
			switch (mouseButtonEvent.action) {
				case MouseButtonEvent::BUTTON_DOWN: return target->HandleMouseDown(mouseButtonEvent);
				case MouseButtonEvent::BUTTON_UP:   return target->HandleMouseUp(mouseButtonEvent);
			}
			return false;
		}

		case Event::MOUSE_MOTION: {
			const MouseMotionEvent mouseMotionEvent = static_cast<const MouseMotionEvent&>(event);
			return target->HandleMouseMove(mouseMotionEvent);
		}

		case Event::MOUSE_WHEEL: {
			const MouseWheelEvent mouseWheelEvent = static_cast<const MouseWheelEvent&>(event);
			return target->HandleMouseWheel(mouseWheelEvent);
		}

		default:
			return false;
	}

	return false;
}

}
