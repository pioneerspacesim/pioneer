// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
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

// base event. can't be instantiated directly
class Event {
public:
	enum Type { // <enum scope='UI::Event' name=UIEventType>
		KEYBOARD,
		MOUSE_BUTTON,
		MOUSE_MOTION,
		MOUSE_WHEEL
	};
	const Type type;

	virtual void ToLuaTable(lua_State *l) const = 0;
protected:
	Event(Type _type) : type(_type) {}
};

struct KeySym {
	KeySym(const SDLKey &_sym, const SDLMod &_mod, const Uint16 _unicode) : sym(_sym), mod(safe_mods(_mod)), unicode(_unicode) {}
	KeySym(const SDLKey &_sym, const SDLMod &_mod) : sym(_sym), mod(safe_mods(_mod)), unicode(0) {}
	KeySym(const SDLKey &_sym) : sym(_sym), mod(KMOD_NONE), unicode(0) {}
	SDLKey sym;
	SDLMod mod;
	Uint16 unicode;

	bool operator<(const KeySym &b) const {
		return sym < b.sym || (sym == b.sym && mod < b.mod);
	}

	bool operator==(const KeySym &b) const {
		return sym == b.sym && mod == b.mod;
	}

private:
	// mask off stuff like caps/numlock
	static SDLMod safe_mods(const SDLMod m) {
		return SDLMod(Uint32(m) & (KMOD_CTRL | KMOD_SHIFT | KMOD_ALT | KMOD_META));
	}
};

// data for various events
class KeyboardEvent : public Event {
public:
	enum Action { // <enum scope='UI::KeyboardEvent' name=UIKeyboardAction prefix=KEY_>
		KEY_DOWN,
		KEY_UP,
		KEY_PRESS
	};
	KeyboardEvent(Action _action, const KeySym &_keysym) : Event(Event::KEYBOARD), action(_action), keysym(_keysym) {}
	const Action action;
	const KeySym keysym;

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
	enum Action { // <enum scope='UI::MouseButtonEvent' name=UIMouseButtonAction prefix=BUTTON_>
		BUTTON_DOWN,
		BUTTON_UP
	};
	enum ButtonType { // <enum scope='UI::MouseButtonEvent' name=UIMouseButtonType prefix=BUTTON_>
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
	enum WheelDirection { // <enum scope='UI::MouseWheelEvent' name=UIMouseWheelDirection prefix=WHEEL_>
		WHEEL_UP,
		WHEEL_DOWN
	};
	MouseWheelEvent(WheelDirection _direction, const Point &_pos) : MouseEvent(Event::MOUSE_WHEEL, _pos), direction(_direction) {}
	WheelDirection direction;

	void ToLuaTable(lua_State *l) const;
};

}

#endif
