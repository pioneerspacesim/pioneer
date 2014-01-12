// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef UI_EVENT_H
#define UI_EVENT_H

#include "libs.h"
#include "Point.h"

struct lua_State;

namespace UI {

class Widget;

class NoEvent;
class KeyboardEvent;
class MouseButtonEvent;
class MouseMotionEvent;
class MouseWheelEvent;
class TextInputEvent;
class JoystickAxisMotionEvent;
class JoystickHatMotionEvent;
class JoystickButtonEvent;

// base event. can't be instantiated directly
class Event {
public:
	enum Type { // <enum scope='UI::Event' name=UIEventType public>
		KEYBOARD,
		TEXT_INPUT,
		MOUSE_BUTTON,
		MOUSE_MOTION,
		MOUSE_WHEEL,
		JOYSTICK_AXIS_MOTION,
		JOYSTICK_HAT_MOTION,
		JOYSTICK_BUTTON
	};
	const Type type;

	virtual void ToLuaTable(lua_State *l) const = 0;
protected:
	Event(Type _type) : type(_type) {}
};

struct KeySym {
	KeySym(const SDL_Keycode &_sym, const SDL_Keymod _mod, const Uint32 _unicode) : sym(_sym), mod(safe_mods(_mod)) {}
	KeySym(const SDL_Keycode &_sym, const SDL_Keymod &_mod) : sym(_sym), mod(safe_mods(_mod)) {}
	KeySym(const SDL_Keycode &_sym) : sym(_sym), mod(KMOD_NONE) {}
	SDL_Keycode sym;
	SDL_Keymod mod;

	static KeySym FromString(const std::string &spec);

	bool operator<(const KeySym &b) const {
		return sym < b.sym || (sym == b.sym && mod < b.mod);
	}

	bool operator==(const KeySym &b) const {
		return sym == b.sym && mod == b.mod;
	}

private:
	// mask off stuff like caps/numlock
	static SDL_Keymod safe_mods(const SDL_Keymod m) {
		return SDL_Keymod(Uint32(m) & (KMOD_CTRL | KMOD_SHIFT | KMOD_ALT | KMOD_GUI));
	}
};

// data for various events
class KeyboardEvent : public Event {
public:
	enum Action { // <enum scope='UI::KeyboardEvent' name=UIKeyboardAction prefix=KEY_ public>
		KEY_DOWN,
		KEY_UP,
	};
	KeyboardEvent(Action _action, const KeySym &_keysym, bool _repeat) : Event(Event::KEYBOARD), action(_action), keysym(_keysym), repeat(_repeat) {}
	const Action action;
	const KeySym keysym;
	const bool repeat;

	void ToLuaTable(lua_State *l) const;
};

class TextInputEvent : public Event {
public:
	TextInputEvent(Uint32 _unicode) : Event(Event::TEXT_INPUT), unicode(_unicode) {}
	const Uint32 unicode;

	void ToLuaTable(lua_State *l) const;
};

class MouseEvent : public Event {
public:
	const Point pos; // relative to widget

protected:
	MouseEvent(Event::Type _type, const Point &_pos) : Event(_type), pos(_pos) {}
};

class MouseButtonEvent : public MouseEvent {
public:
	enum Action { // <enum scope='UI::MouseButtonEvent' name=UIMouseButtonAction prefix=BUTTON_ public>
		BUTTON_DOWN,
		BUTTON_UP
	};
	enum ButtonType { // <enum scope='UI::MouseButtonEvent' name=UIMouseButtonType prefix=BUTTON_ public>
		BUTTON_LEFT,
		BUTTON_MIDDLE,
		BUTTON_RIGHT
	};
	MouseButtonEvent(Action _action, ButtonType _button, const Point &_pos) : MouseEvent(Event::MOUSE_BUTTON, _pos), action(_action), button(_button) {}
	const Action action;
	const ButtonType button;

	void ToLuaTable(lua_State *l) const;
};

class MouseMotionEvent : public MouseEvent {
public:
	MouseMotionEvent(const Point &_pos, const Point &_rel) : MouseEvent(Event::MOUSE_MOTION, _pos), rel(_rel) {}
	const Point rel;

	void ToLuaTable(lua_State *l) const;
};

class MouseWheelEvent : public MouseEvent {
public:
	enum WheelDirection { // <enum scope='UI::MouseWheelEvent' name=UIMouseWheelDirection prefix=WHEEL_ public>
		WHEEL_UP,
		WHEEL_DOWN
	};
	MouseWheelEvent(WheelDirection _direction, const Point &_pos) : MouseEvent(Event::MOUSE_WHEEL, _pos), direction(_direction) {}
	WheelDirection direction;

	void ToLuaTable(lua_State *l) const;
};

class JoystickEvent : public Event {
public:
	const SDL_JoystickID joystick;

protected:
	JoystickEvent(Event::Type _type, SDL_JoystickID _joystick): Event(_type), joystick(_joystick) {}
};

class JoystickAxisMotionEvent : public JoystickEvent {
public:
	const float value; // -1 to 1
	const int axis;

	JoystickAxisMotionEvent(SDL_JoystickID _joystick, float _value, int _axis):
		JoystickEvent(Event::JOYSTICK_AXIS_MOTION, _joystick), value(_value), axis(_axis) {}

	void ToLuaTable(lua_State *l) const;
};

class JoystickHatMotionEvent : public JoystickEvent {
public:
	enum JoystickHatDirection { // <enum scope='UI::JoystickHatMotionEvent' name=UIJoystickHatDirection prefix=HAT_ public>
		HAT_CENTRE    = SDL_HAT_CENTERED,
		HAT_UP        = SDL_HAT_UP,
		HAT_RIGHT     = SDL_HAT_RIGHT,
		HAT_DOWN      = SDL_HAT_DOWN,
		HAT_LEFT      = SDL_HAT_LEFT,
		HAT_RIGHTUP   = SDL_HAT_RIGHTUP,
		HAT_RIGHTDOWN = SDL_HAT_RIGHTDOWN,
		HAT_LEFTUP    = SDL_HAT_LEFTUP,
		HAT_LEFTDOWN  = SDL_HAT_LEFTDOWN
	};

	const JoystickHatDirection direction;
	const int hat;

	JoystickHatMotionEvent(SDL_JoystickID _joystick, JoystickHatDirection _direction, int _hat):
		JoystickEvent(Event::JOYSTICK_HAT_MOTION, _joystick), direction(_direction), hat(_hat) {}

	void ToLuaTable(lua_State *l) const;
};

class JoystickButtonEvent : public JoystickEvent {
public:
	enum Action { // <enum scope='UI::JoystickButtonEvent' name=UIJoystickButtonAction prefix=BUTTON_ public>
		BUTTON_DOWN,
		BUTTON_UP
	};

	const Action action;
	const int button;

	JoystickButtonEvent(SDL_JoystickID _joystick, Action _action, int _button):
		JoystickEvent(Event::JOYSTICK_BUTTON, _joystick), action(_action), button(_button) {}

	void ToLuaTable(lua_State *l) const;
};

}

#endif
