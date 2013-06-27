// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "KeyBindings.h"
#include "Pi.h"
#include "Lang.h"
#include "StringF.h"
#include <string>
#include <sstream>

namespace KeyBindings {

#define KEY_BINDING(name,a,b,c) KeyAction name;
#define AXIS_BINDING(name,a,b,c) AxisBinding name;
#include "KeyBindings.inc.h"

// create the BindingPrototype sets for use by the UI
#define BINDING_PAGE(name) const BindingPrototype BINDING_PROTOS_ ## name[] = {
#define BINDING_PAGE_END() {0,0,0,0}};
#define BINDING_GROUP(ui_name) \
	{ ui_name, 0, 0, 0 },
#define KEY_BINDING(name, config_name, ui_name, default_value) \
	{ ui_name, config_name, &KeyBindings::name, 0 },
#define AXIS_BINDING(name, config_name, ui_name, default_value) \
	{ ui_name, config_name, 0, &KeyBindings::name },
#include "KeyBindings.inc.h"

// static binding object lists for use by the dispatch function
static KeyAction* const s_KeyBindings[] = {
#define KEY_BINDING(name, b,c,d) &KeyBindings::name,
#include "KeyBindings.inc.h"
	0
};

bool KeyBinding::Matches(const SDL_keysym *sym) const {
	if (type == KEYBOARD_KEY) {
		return (sym->sym == u.keyboard.key) && ((sym->mod & 0xfff) == u.keyboard.mod);
	} else
		return false;
}

KeyBinding KeyBinding::keyboardBinding(SDLKey key, SDLMod mod) {
	KeyBinding kb;

	kb.type = KEYBOARD_KEY;
	kb.u.keyboard.key  = key;
	kb.u.keyboard.mod  = mod;

	return kb;
}

bool KeyAction::IsActive() const
{
	if (binding.type == KEYBOARD_KEY) {
		// 0xfff filters out numlock, capslock and other shit
		if (binding.u.keyboard.mod != 0)
			return Pi::KeyState(binding.u.keyboard.key) && ((Pi::KeyModState()&0xfff) == binding.u.keyboard.mod);

		return Pi::KeyState(binding.u.keyboard.key) != 0;

	} else if (binding.type == JOYSTICK_BUTTON) {
		return Pi::JoystickButtonState(binding.u.joystickButton.joystick, binding.u.joystickButton.button) != 0;
	} else if (binding.type == JOYSTICK_HAT) {
		return Pi::JoystickHatState(binding.u.joystickHat.joystick, binding.u.joystickHat.hat) == binding.u.joystickHat.direction;
	} else
		abort();

	return false;
}

void KeyAction::CheckSDLEventAndDispatch(const SDL_Event *event) {
	switch (event->type) {
		case SDL_KEYDOWN:
		case SDL_KEYUP:
		{
			if (binding.type != KEYBOARD_KEY)
				return;
			SDL_keysym sym = event->key.keysym;
			// 0xfff filters out numlock, capslock and other shit
			if (binding.u.keyboard.mod && ((sym.mod & 0xfff) != binding.u.keyboard.mod))
				return;
			if (sym.sym == binding.u.keyboard.key) {
				if (event->key.state == SDL_PRESSED)
					onPress.emit();
				else if (event->key.state == SDL_RELEASED)
					onRelease.emit();
			}
			break;
		}
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
		{
			if (binding.type != JOYSTICK_BUTTON)
				return;
			if (binding.u.joystickButton.joystick != event->jbutton.which)
				return;
			if (binding.u.joystickButton.button != event->jbutton.button) {
				if (event->jbutton.state == SDL_PRESSED)
					onPress.emit();
				else if (event->jbutton.state == SDL_RELEASED)
					onRelease.emit();
			}
			break;
		}
		case SDL_JOYHATMOTION:
		{
			if (binding.type != JOYSTICK_HAT)
				return;
			if (binding.u.joystickHat.joystick != event->jhat.which)
				return;
			if (binding.u.joystickHat.hat != event->jhat.hat)
				return;
			if (event->jhat.value == binding.u.joystickHat.direction) {
				onPress.emit();
				// XXX to emit onRelease, we need to have access to the state of the joystick hat prior to this event,
				// so that we can detect the case of switching from a direction that matches the binding to some other direction
			}
			break;
		}
		default: break;
	}
}

std::string KeyBinding::Description() const {
	std::ostringstream oss;

	if (type == KEYBOARD_KEY) {
		if (u.keyboard.mod & KMOD_SHIFT) oss << Lang::SHIFT;
		if (u.keyboard.mod & KMOD_CTRL) oss << Lang::CTRL;
		if (u.keyboard.mod & KMOD_ALT) oss << Lang::ALT;
		if (u.keyboard.mod & KMOD_META) oss << Lang::META;
		oss << SDL_GetKeyName(u.keyboard.key);
	} else if (type == JOYSTICK_BUTTON) {
		oss << Lang::JOY << int(u.joystickButton.joystick);
		oss << Lang::BUTTON << int(u.joystickButton.button);
	} else if (type == JOYSTICK_HAT) {
		oss << Lang::JOY << int(u.joystickHat.joystick);
		oss << Lang::HAT << int(u.joystickHat.hat);
		oss << Lang::DIRECTION << int(u.joystickHat.direction);
	} else
		abort();

	return oss.str();
}

AxisBinding::AxisBinding() {
	this->joystick = 0;
	this->axis = 0;
	this->direction = POSITIVE;
}

AxisBinding::AxisBinding(Uint8 joystick_, Uint8 axis_, AxisDirection direction_) {
	this->joystick = joystick_;
	this->axis = axis_;
	this->direction = direction_;
}

float AxisBinding::GetValue() {
	float value = Pi::JoystickAxisState(joystick, axis);

	if (direction == POSITIVE)
		return value;
	else
		return -value;
}

std::string AxisBinding::Description() const {
	const char *axis_names[] = {Lang::X, Lang::Y, Lang::Z};
	std::ostringstream ossaxisnum;
	ossaxisnum << int(axis);

	return stringf(Lang::JOY_AXIS,
		formatarg("sign", direction == KeyBindings::NEGATIVE ? "-" : ""), // no + sign if positive
		formatarg("signp", direction == KeyBindings::NEGATIVE ? "-" : "+"), // optional with + sign
		formatarg("joynum", joystick),
		formatarg("axis", axis >= 0 && axis < 3 ? axis_names[axis] : ossaxisnum.str())
	);
}

/**
 * Exampe strings:
 *   Key55
 *   Joy0Button2
 *   Joy0Hat0Dir3
 */
bool KeyBindingFromString(const std::string &str, KeyBinding *kb)
{
	const char *digits = "1234567890";
	const char *p = str.c_str();

	if (strncmp(p, "Key", 3) == 0) {
		kb->type = KEYBOARD_KEY;
		p += 3;

		kb->u.keyboard.key = SDLKey(atoi(p));
		p += strspn(p, digits);

		if (strncmp(p, "Mod", 3) == 0) {
			p += 3;
			kb->u.keyboard.mod = SDLMod(atoi(p));
		} else
			kb->u.keyboard.mod = KMOD_NONE;

		return true;

	} else if (strncmp(p, "Joy", 3) == 0) {
		p += 3;

		int joy = atoi(p);
		p += strspn(p, digits);

		if (strncmp(p, "Button", 6) == 0) {
			p += 6;
			kb->type = JOYSTICK_BUTTON;
			kb->u.joystickButton.joystick = joy;
			kb->u.joystickButton.button = atoi(p);
			return true;
		} else if (strncmp(p, "Hat", 3) == 0) {
			p += 3;
			kb->type = JOYSTICK_HAT;
			kb->u.joystickHat.joystick = joy;
			kb->u.joystickHat.hat = atoi(p);
			p += strspn(p, digits);

			if (strncmp(p, "Dir", 3) != 0)
				return false;

			p += 3;
			kb->u.joystickHat.direction = atoi(p);
			return true;
		}

		return false;
	}

	return false;
}

KeyBinding KeyBindingFromString(const std::string &str) {
	KeyBinding kb;

	if (!KeyBindingFromString(str, &kb))
		abort();

	return kb;
}

std::string KeyBindingToString(const KeyBinding &kb) {
	std::ostringstream oss;

	if (kb.type == KEYBOARD_KEY) {
		oss << "Key" << int(kb.u.keyboard.key);
		if (kb.u.keyboard.mod != 0)
			oss << "Mod" << int(kb.u.keyboard.mod);
	} else if (kb.type == JOYSTICK_BUTTON) {
		oss << "Joy" << int(kb.u.joystickButton.joystick);
		oss << "Button" << int(kb.u.joystickButton.button);
	} else if (kb.type == JOYSTICK_HAT) {
		oss << "Joy" << int(kb.u.joystickHat.joystick);
		oss << "Hat" << int(kb.u.joystickHat.hat);
		oss << "Dir" << int(kb.u.joystickHat.direction);
	} else
		abort();

	return oss.str();
}

bool AxisBindingFromString(const std::string &str, AxisBinding *ab) {
	const char *digits = "1234567890";
	const char *p = str.c_str();

	if (p[0] == '-') {
		ab->direction = NEGATIVE;
		p++;
	}
	else
		ab->direction = POSITIVE;

	if (strncmp(p, "Joy", 3) != 0)
		return false;

	p += 3;
	ab->joystick = atoi(p);
	p += strspn(p, digits);

	if (strncmp(p, "Axis", 4) != 0)
		return false;

	p += 4;
	ab->axis = atoi(p);

	return true;
}

AxisBinding AxisBindingFromString(const std::string &str) {
	AxisBinding ab;

	if (!AxisBindingFromString(str, &ab))
		abort();

	return ab;
}

std::string AxisBindingToString(const AxisBinding &ab) {
	std::ostringstream oss;

	if (ab.direction == NEGATIVE)
		oss << '-';

	oss << "Joy";
	oss << int(ab.joystick);
	oss << "Axis";
	oss << int(ab.axis);

	return oss.str();
}

void DispatchSDLEvent(const SDL_Event *event) {
	switch (event->type) {
		case SDL_KEYDOWN:
		case SDL_KEYUP:
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
		case SDL_JOYHATMOTION:
			break;
		default: return;
	}

	// simplest possible approach here: just check each binding and dispatch if it matches
	for (KeyAction * const *binding = s_KeyBindings; *binding; ++binding) {
		(*binding)->CheckSDLEventAndDispatch(event);
	}
}

void InitKeyBinding(KeyAction &kb, const std::string &bindName, SDLKey defaultKey) {
	std::string keyName = Pi::config->String(bindName.c_str());
	if (keyName.length() == 0) {
		keyName = stringf("Key%0{u}", Uint32(defaultKey));
		Pi::config->SetString(bindName.c_str(), keyName.c_str());
	}
	KeyBindingFromString(keyName.c_str(), &(kb.binding));
}

void InitAxisBinding(AxisBinding &ab, const std::string &bindName, const std::string &defaultAxis) {
	std::string axisName = Pi::config->String(bindName.c_str());
	if (axisName.length() == 0) {
		axisName = defaultAxis;
		Pi::config->SetString(bindName.c_str(), axisName.c_str());
	}
	AxisBindingFromString(axisName.c_str(), &ab);
}

void UpdateBindings()
{
#define KEY_BINDING(name, config_name, b, default_value) InitKeyBinding(KeyBindings::name, config_name, default_value);
#define AXIS_BINDING(name, config_name, b, default_value) InitAxisBinding(KeyBindings::name, config_name, default_value);
#include "KeyBindings.inc.h"
}

void InitBindings()
{
	UpdateBindings();
	Pi::config->Save();
}

}
