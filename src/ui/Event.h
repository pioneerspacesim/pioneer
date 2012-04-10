#ifndef _UI_EVENT_H
#define _UI_EVENT_H

#include "libs.h"

namespace UI {

class Widget;

class NoEvent;
class KeyboardEvent;
class MouseButtonEvent;
class MouseMotionEvent;
class MouseWheelEvent;

class Event {
public:
	enum Type {
		NONE,
		KEYBOARD,
		MOUSE_BUTTON,
		MOUSE_MOTION,
		MOUSE_WHEEL
	};
	const Type type;

	static bool Dispatch(const Event &event, Widget *target);
	static bool DispatchSDLEvent(const SDL_Event &event, Widget *target);

protected:
	Event(Type _type) : type(_type) {}

private:
	static bool KeyDownDispatch(const KeyboardEvent &event, Widget *target);
	static bool KeyUpDispatch(const KeyboardEvent &event, Widget *target);
	static bool MouseDownDispatch(const MouseButtonEvent &event, Widget *target);
	static bool MouseUpDispatch(const MouseButtonEvent &event, Widget *target);
	static bool MouseMoveDispatch(const MouseMotionEvent &event, Widget *target);
	static bool MouseWheelDispatch(const MouseWheelEvent &event, Widget *target);
};

// data for various events
class KeyboardEvent : public Event {
public:
	enum Action {
		KEY_DOWN,
		KEY_UP
	};
	KeyboardEvent(Action _action, SDL_keysym _keysym) : Event(Event::KEYBOARD), action(_action), keysym(_keysym) {}
	const Action action;
	const SDL_keysym keysym;
};

class MouseButtonEvent : public Event {
public:
	enum Action {
		BUTTON_DOWN,
		BUTTON_UP
	};
	enum ButtonType {
		BUTTON_LEFT,
		BUTTON_MIDDLE,
		BUTTON_RIGHT
	};
	MouseButtonEvent(Action _action, ButtonType _button, const vector2f &_pos) : Event(Event::MOUSE_BUTTON), action(_action), button(_button), pos(_pos) {}
	const Action action;
	const ButtonType button;
	const vector2f pos; // relative to widget
};

class MouseMotionEvent : public Event {
public:
	MouseMotionEvent(const vector2f &_pos) : Event(Event::MOUSE_MOTION), pos(_pos) {}
	const vector2f pos; // relative to widget
};

class MouseWheelEvent : public Event {
public:
	enum WheelDirection {
		WHEEL_UP,
		WHEEL_DOWN
	};
	MouseWheelEvent(WheelDirection _direction) : Event(Event::MOUSE_WHEEL), direction(_direction) {}
	WheelDirection direction;
};

}

#endif
