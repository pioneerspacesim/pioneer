// Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "KeyBindings.h"
#include "Pi.h"
#include "Lang.h"
#include "StringF.h"
#include <string>
#include <sstream>

static bool m_disableBindings = 0;

namespace KeyBindings {

#define KEY_BINDING(name,a,b,c,d) ActionBinding name;
#define AXIS_BINDING(name,a,b,c) JoyAxisBinding name;
#include "KeyBindings.inc.h"

// create the BindingPrototype sets for use by the UI
#define BINDING_PAGE(name) const BindingPrototype BINDING_PROTOS_ ## name[] = {
#define BINDING_PAGE_END() {0,0,0,0}};
#define BINDING_GROUP(ui_name) \
	{ ui_name, 0, 0, 0 },
#define KEY_BINDING(name, config_name, ui_name, def1, def2) \
	{ ui_name, config_name, &KeyBindings::name, 0 },
#define AXIS_BINDING(name, config_name, ui_name, default_value) \
	{ ui_name, config_name, 0, &KeyBindings::name },
#include "KeyBindings.inc.h"

// static binding object lists for use by the dispatch function
static ActionBinding* const s_KeyBindings[] = {
#define KEY_BINDING(name, a,b,c,d) &KeyBindings::name,
#include "KeyBindings.inc.h"
	0
};

bool KeyBinding::IsActive() const
{
	if (type == BINDING_DISABLED) {
		return false;
	} else if (type == KEYBOARD_KEY) {
		if (!Pi::input.KeyState(u.keyboard.key))
			return false;
		if (u.keyboard.mod == KMOD_NONE)
			return true;
		else {
			int mod = Pi::input.KeyModState();
			if (mod & KMOD_CTRL) { mod |= KMOD_CTRL; }
			if (mod & KMOD_SHIFT) { mod |= KMOD_SHIFT; }
			if (mod & KMOD_ALT) { mod |= KMOD_ALT; }
			if (mod & KMOD_GUI) { mod |= KMOD_GUI; }
			return ((mod & u.keyboard.mod) == u.keyboard.mod);
		}
	} else if (type == JOYSTICK_BUTTON) {
		return Pi::input.JoystickButtonState(u.joystickButton.joystick, u.joystickButton.button) != 0;
	} else if (type == JOYSTICK_HAT) {
		// SDL_HAT generates diagonal directions by ORing two cardinal directions.
		int hatState = Pi::input.JoystickHatState(u.joystickHat.joystick, u.joystickHat.hat);
		return (hatState & u.joystickHat.direction) == u.joystickHat.direction;
	} else
		abort();

	return false;
}

bool KeyBinding::Matches(const SDL_Keysym *sym) const {
	int mod = sym->mod;
	if (mod & KMOD_CTRL) { mod |= KMOD_CTRL; }
	if (mod & KMOD_SHIFT) { mod |= KMOD_SHIFT; }
	if (mod & KMOD_ALT) { mod |= KMOD_ALT; }
	if (mod & KMOD_GUI) { mod |= KMOD_GUI; }
	return
		(type == KEYBOARD_KEY) &&
		(sym->sym == u.keyboard.key) &&
		((mod & u.keyboard.mod) == u.keyboard.mod);
}

bool KeyBinding::Matches(const SDL_JoyButtonEvent *joy) const {
	return
		(type == JOYSTICK_BUTTON) &&
		(joy->which == u.joystickButton.joystick) &&
		(joy->button == u.joystickButton.button);
}

bool KeyBinding::Matches(const SDL_JoyHatEvent *joy) const {
	return
		(type == JOYSTICK_HAT) &&
		(joy->which == u.joystickHat.joystick) &&
		(joy->hat == u.joystickHat.hat) &&
		(joy->value == u.joystickHat.direction);
}

std::string KeyBinding::Description() const {
	std::ostringstream oss;

	if (type == BINDING_DISABLED) {
		// blank
	} else if (type == KEYBOARD_KEY) {
		if (u.keyboard.mod & KMOD_SHIFT) oss << Lang::SHIFT << " + ";
		if (u.keyboard.mod & KMOD_CTRL) oss << Lang::CTRL << " + ";
		if (u.keyboard.mod & KMOD_ALT) oss << Lang::ALT << " + ";
		if (u.keyboard.mod & KMOD_GUI) oss << Lang::META << " + ";
		oss << SDL_GetKeyName(u.keyboard.key);
	} else if (type == JOYSTICK_BUTTON) {
		oss << Pi::input.JoystickName(u.joystickButton.joystick);
		oss << Lang::BUTTON << int(u.joystickButton.button);
	} else if (type == JOYSTICK_HAT) {
		oss << Pi::input.JoystickName(u.joystickHat.joystick);
		oss << Lang::HAT << int(u.joystickHat.hat);
		oss << Lang::DIRECTION << int(u.joystickHat.direction);
	} else
		assert(0 && "invalid key binding type");

	return oss.str();
}


/**
 * In a C string pointed to by the string pointer pointed to by p, scan for
 * the character token tok, copying the bytes on the way to bufOut which is at most buflen long.
 *
 * returns true on success, returns false if the end of the input string was reached,
 *   or the buffer would be overfilled without encountering the token.
 *
 * upon return, the pointer pointed to by p will refer to the character AFTER the tok.
 */
static bool ReadToTok(char tok, const char **p, char *bufOut, size_t buflen) {
	unsigned int idx;
	for (idx = 0; idx < buflen; idx++) {
		if (**p == '\0' || **p == tok) {
			break;
		}
		bufOut[idx] = *((*p)++);
	}
	// if, after that, we're not pointing at the tok, we must have hit
	// the terminal or run out of buffer.
	if (**p != tok) {
		return false;
	}
	// otherwise, skip over the tok.
	(*p)++;
	// if there is sufficient space in the buffer, NUL terminate.
	if (idx < buflen) {
		bufOut[idx] = '\0';
	}
	return true;
}

/**
 * Example strings:
 *   Key55
 *   Joy{uuid}/Button2
 *   Joy{uuid}/Hat0Dir3
 */
bool KeyBinding::FromString(const char *str, KeyBinding &kb)
{
	const char *digits = "1234567890";
	const char *p = str;

	if (strcmp(p, "disabled") == 0) {
		kb.Clear();
	} else if (strncmp(p, "Key", 3) == 0) {
		kb.type = KEYBOARD_KEY;
		p += 3;

		kb.u.keyboard.key = SDL_Keycode(atoi(p));
		p += strspn(p, digits);

		if (strncmp(p, "Mod", 3) == 0) {
			p += 3;
			kb.u.keyboard.mod = SDL_Keymod(atoi(p));
		} else {
			kb.u.keyboard.mod = KMOD_NONE;
		}
	} else if (strncmp(p, "Joy", 3) == 0) {
		p += 3;

		const int JoyUUIDLength = 33;
		char joyUUIDBuf[JoyUUIDLength];

		// read the UUID
		if (!ReadToTok('/', &p, joyUUIDBuf, JoyUUIDLength)) {
			return false;
		}
		// force terminate
		joyUUIDBuf[JoyUUIDLength-1] = '\0';
		// now, locate the internal ID.
		int joy = Pi::input.JoystickFromGUIDString(joyUUIDBuf);
		if (joy == -1) {
			return false;
		}
		if (strncmp(p, "Button", 6) == 0) {
			p += 6;
			kb.type = JOYSTICK_BUTTON;
			kb.u.joystickButton.joystick = joy;
			kb.u.joystickButton.button = atoi(p);
		} else if (strncmp(p, "Hat", 3) == 0) {
			p += 3;
			kb.type = JOYSTICK_HAT;
			kb.u.joystickHat.joystick = joy;
			kb.u.joystickHat.hat = atoi(p);
			p += strspn(p, digits);

			if (strncmp(p, "Dir", 3) != 0)
				return false;

			p += 3;
			kb.u.joystickHat.direction = atoi(p);
		} else
			return false;
	}

	return true;
}

KeyBinding KeyBinding::FromString(const char *str) {
	KeyBinding kb;
	if (!KeyBinding::FromString(str, kb))
		kb.Clear();
	return kb;
}

std::ostream &operator<<(std::ostream &oss, const KeyBinding &kb)
{
	if (kb.type == BINDING_DISABLED) {
		oss << "disabled";
	} else if (kb.type == KEYBOARD_KEY) {
		oss << "Key" << int(kb.u.keyboard.key);
		if (kb.u.keyboard.mod != 0) {
			oss << "Mod" << int(kb.u.keyboard.mod);
		}
	} else if (kb.type == JOYSTICK_BUTTON) {
		oss << "Joy" << Pi::input.JoystickGUIDString(kb.u.joystickButton.joystick);
		oss << "/Button" << int(kb.u.joystickButton.button);
	} else if (kb.type == JOYSTICK_HAT) {
		oss << "Joy" << Pi::input.JoystickGUIDString(kb.u.joystickButton.joystick);
		oss << "/Hat" << int(kb.u.joystickHat.hat);
		oss << "Dir" << int(kb.u.joystickHat.direction);
	} else {
		assert(0 && "KeyBinding type field is invalid");
	}
	return oss;
}

std::string KeyBinding::ToString() const {
	std::ostringstream oss;
	oss << *this;
	return oss.str();
}

KeyBinding KeyBinding::FromKeyMod(SDL_Keycode key, SDL_Keymod mod)
{
	KeyBinding kb;
	kb.type = KEYBOARD_KEY;
	kb.u.keyboard.key = key;
	// expand the modifier to cover both left & right variants
	int imod = mod;
	if (imod & KMOD_CTRL) { imod |= KMOD_CTRL; }
	if (imod & KMOD_SHIFT) { imod |= KMOD_SHIFT; }
	if (imod & KMOD_ALT) { imod |= KMOD_ALT; }
	if (imod & KMOD_GUI) { imod |= KMOD_GUI; }
	kb.u.keyboard.mod = static_cast<SDL_Keymod>(imod);
	return kb;
}

KeyBinding KeyBinding::FromJoystickButton(Uint8 joystick, Uint8 button)
{
	KeyBinding kb;
	kb.type = JOYSTICK_BUTTON;
	kb.u.joystickButton.joystick = joystick;
	kb.u.joystickButton.button = button;
	return kb;
}

KeyBinding KeyBinding::FromJoystickHat(Uint8 joystick, Uint8 hat, Uint8 direction)
{
	KeyBinding kb;
	kb.type = JOYSTICK_HAT;
	kb.u.joystickHat.joystick = joystick;
	kb.u.joystickHat.hat = hat;
	kb.u.joystickHat.direction = direction;
	return kb;
}

void ActionBinding::SetFromString(const char *str)
{
	const size_t BUF_SIZE = 64;
	const size_t len = strlen(str);
	if (len >= BUF_SIZE) {
		Output("invalid ActionBinding string\n");
		binding1 = KeyBinding::FromString(str);
		binding2.Clear();
	} else {
		const char *sep = strchr(str, ',');
		if (sep) {
			char buf[BUF_SIZE];
			const size_t len1 = sep - str;
			const size_t len2 = len - len1 - 1;
			memcpy(buf, str, len1);
			buf[len1] = '\0';
			binding1 = KeyBinding::FromString(buf);
			memcpy(buf, sep+1, len2);
			buf[len2] = '\0';
			binding2 = KeyBinding::FromString(buf);
		} else {
			binding1 = KeyBinding::FromString(str);
			binding2.Clear();
		}
	}
}

std::string ActionBinding::ToString() const
{
	std::ostringstream oss;
	if (binding1.Enabled() && binding2.Enabled()) {
		oss << binding1 << "," << binding2;
	} else if (binding1.Enabled()) {
		oss << binding1;
	} else if (binding2.Enabled()) {
		oss << binding2;
	} else {
		oss << "disabled";
	}
	return oss.str();
}

bool ActionBinding::IsActive() const {
	return binding1.IsActive() || binding2.IsActive();
}

bool ActionBinding::Matches(const SDL_Keysym *sym) const {
	return binding1.Matches(sym) || binding2.Matches(sym);
}

void ActionBinding::CheckSDLEventAndDispatch(const SDL_Event *event) {
	if (m_disableBindings) return;
	switch (event->type) {
		case SDL_KEYDOWN:
		case SDL_KEYUP:
		{
			if (Matches(&event->key.keysym)) {
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
			if (binding1.Matches(&event->jbutton) || binding2.Matches(&event->jbutton)) {
				if (event->jbutton.state == SDL_PRESSED)
					onPress.emit();
				else if (event->jbutton.state == SDL_RELEASED)
					onRelease.emit();
			}
			break;
		}
		case SDL_JOYHATMOTION:
		{
			if (binding1.Matches(&event->jhat) || binding2.Matches(&event->jhat)) {
				onPress.emit();
				// XXX to emit onRelease, we need to have access to the state of the joystick hat prior to this event,
				// so that we can detect the case of switching from a direction that matches the binding to some other direction
			}
			break;
		}
		default: break;
	}
}

bool JoyAxisBinding::IsActive() const
{
	// If the stick is within the deadzone, it's not active.
	return fabs(Pi::input.JoystickAxisState(joystick, axis)) > deadzone;
}

float JoyAxisBinding::GetValue() const
{
	if (!Enabled()) return 0.0f;

	const float o_val = Pi::input.JoystickAxisState(joystick, axis);

	// Deadzone with normalisation
	float value = fabs(o_val);
	if (value < deadzone) {
		return 0.0f;
	} else {
		// subtract deadzone and re-normalise to full range
		value = (value - deadzone) / (1.0f - deadzone);
	}

	// Apply sensitivity scaling and clamp.
	value = fmax(fmin(value * sensitivity, 1.0f), 0.0f);

	value = copysign(value, o_val);

	// Invert as necessary.
	return direction == POSITIVE ? value : 0.0f - value;
}

bool JoyAxisBinding::Matches(const SDL_Event *event) const
{
	if (event->type != SDL_JOYAXISMOTION) return false;
	return event->jaxis.which == joystick && event->jaxis.axis == axis;
}

std::string JoyAxisBinding::Description() const {
	if (!Enabled()) return std::string();

	const char *axis_names[] = {Lang::X, Lang::Y, Lang::Z};
	std::ostringstream ossaxisnum;
	ossaxisnum << int(axis);

	return stringf(Lang::JOY_AXIS,
		formatarg("sign", direction == KeyBindings::NEGATIVE ? "-" : ""), // no + sign if positive
		formatarg("signp", direction == KeyBindings::NEGATIVE ? "-" : "+"), // optional with + sign
		formatarg("joynum", joystick),
		formatarg("joyname", Pi::input.JoystickName(joystick)),
		formatarg("axis", axis >= 0 && axis < 3 ? axis_names[axis] : ossaxisnum.str())
	);
}

bool JoyAxisBinding::FromString(const char *str, JoyAxisBinding &ab) {
	if (strcmp(str, "disabled") == 0) {
		ab.Clear();
		return true;
	}

	const char *p = str;

	if (p[0] == '-') {
		ab.direction = NEGATIVE;
		p++;
	}
	else
		ab.direction = POSITIVE;

	if (strncmp(p, "Joy", 3) != 0)
		return false;
	p += 3;

	const int JoyUUIDLength = 33;
	char joyUUIDBuf[JoyUUIDLength];

	// read the UUID
	if (!ReadToTok('/', &p, joyUUIDBuf, JoyUUIDLength)) {
		return false;
	}
	// force terminate
	joyUUIDBuf[JoyUUIDLength-1] = '\0';
	// now, map the GUID to a joystick number
	const int joystick = Pi::input.JoystickFromGUIDString(joyUUIDBuf);
	if (joystick == -1) {
		return false;
	}
	// found a joystick
	assert(joystick < 256);
	ab.joystick = Uint8(joystick);

	if (strncmp(p, "Axis", 4) != 0)
		return false;

	p += 4;
	ab.axis = atoi(p);

	// Skip past the axis integer
	if (!(p = strstr(p, "/DZ")))
		return true; // deadzone is optional

	p += 3;
	ab.deadzone = atof(p);

	// Skip past the deadzone float
	if (!(p = strstr(p, "/E")))
		return true; // sensitivity is optional

	p += 2;
	ab.sensitivity = atof(p);

	return true;
}

JoyAxisBinding JoyAxisBinding::FromString(const char *str) {
	JoyAxisBinding ab;
	if (!JoyAxisBinding::FromString(str, ab))
		ab.Clear();
	return ab;
}

std::string JoyAxisBinding::ToString() const {
	std::ostringstream oss;
	if (Enabled()) {
		if (direction == NEGATIVE)
			oss << '-';

		oss << "Joy";
		oss << Pi::input.JoystickGUIDString(joystick);
		oss << "/Axis";
		oss << int(axis);
		oss << "/DZ" << deadzone;
		oss << "/E" << sensitivity;
	} else {
		oss << "disabled";
	}
	return oss.str();
}

void AxisBinding::SetFromString(const std::string str)
{
	size_t ofs = 0;
	size_t nextpos = str.find(',');
	if (nextpos == std::string::npos) return;

	if (str.substr(ofs, 8) != "disabled")
		axis = JoyAxisBinding::FromString(str.substr(0, nextpos).c_str());

	ofs = nextpos + 1;
	nextpos = str.find(',', ofs);
	if (str.substr(ofs, 8) != "disabled")
		positive = KeyBinding::FromString(str.substr(ofs, nextpos - ofs).c_str());

	ofs = nextpos + 1;
	if (str.substr(ofs, 8) != "disabled")
		negative = KeyBinding::FromString(str.substr(ofs).c_str());
}

std::string AxisBinding::ToString() const
{
	std::ostringstream oss;
	oss << axis.ToString() << ',';
	oss << positive.ToString() << ',';
	oss << negative.ToString();
	return oss.str();
}

bool AxisBinding::IsActive() const
{
	return axis.IsActive() || positive.IsActive() || negative.IsActive();
}

float AxisBinding::GetValue() const
{
	// Holding the positive and negative keys cancel each other out,
	float value = 0.0f;
	value += positive.IsActive() ? 1.0 : 0.0;
	value -= negative.IsActive() ? 1.0 : 0.0;

	// And input on the axis device supercedes both of them.
	return axis.IsActive() ? axis.GetValue() : value;
}

void AxisBinding::CheckSDLEventAndDispatch(const SDL_Event *event)
{
	if (m_disableBindings) return;
	float value = GetValue();
	switch (event->type) {
		case SDL_KEYDOWN:
		case SDL_KEYUP:
		{
			if (positive.Matches(&event->key.keysym) && negative.Matches(&event->key.keysym)) {
				onAxis.emit(value);
			}
			break;
		}
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
		{
			if (positive.Matches(&event->jbutton) || negative.Matches(&event->jbutton)) {
				onAxis.emit(value);
			}
			break;
		}
		case SDL_JOYHATMOTION:
		{
			if (positive.Matches(&event->jhat) || positive.Matches(&event->jhat)) {
				onAxis.emit(value);
				// XXX to emit onRelease, we need to have access to the state of the joystick hat prior to this event,
				// so that we can detect the case of switching from a direction that matches the binding to some other direction
			}
			break;
		}
		case SDL_JOYAXISMOTION:
		{
			if (axis.Matches(event)) onAxis.emit(value);
		}
		default: break;
	}
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
	for (ActionBinding * const *binding = s_KeyBindings; *binding; ++binding) {
		(*binding)->CheckSDLEventAndDispatch(event);
	}
}

void InitKeyBinding(ActionBinding &kb, const std::string &bindName, Uint32 defaultKey1, Uint32 defaultKey2) {
	std::string keyName = Pi::config->String(bindName.c_str());
	if (keyName.length() == 0) {
		if (defaultKey1 && defaultKey2) {
			keyName = stringf("Key%0{u},Key%1{u}", defaultKey1, defaultKey2);
		} else if (defaultKey1 || defaultKey2) {
			Uint32 k = (defaultKey1 | defaultKey2); // only one of them is non-zero, so this gets the non-zero value
			keyName = stringf("Key%0{u}", k);
		}
		Pi::config->SetString(bindName.c_str(), keyName.c_str());
	}

	// set the binding from the configured or default value
	kb.SetFromString(keyName.c_str());
}

void InitAxisBinding(JoyAxisBinding &ab, const std::string &bindName, const std::string &defaultAxis) {
	std::string axisName = Pi::config->String(bindName.c_str());
	if (axisName.length() == 0) {
		axisName = defaultAxis;
		Pi::config->SetString(bindName.c_str(), axisName.c_str());
	}

	// set the binding from the configured or default value
	if (!JoyAxisBinding::FromString(axisName.c_str(), ab)) {
		Output("invalid axis binding '%s' in config file for %s\n", axisName.c_str(), bindName.c_str());
		ab.Clear();
	}
}

void UpdateBindings()
{
#define KEY_BINDING(name, config_name, b, default_value_1, default_value_2) \
	InitKeyBinding(KeyBindings::name, config_name, default_value_1, default_value_2);
#define AXIS_BINDING(name, config_name, b, default_value) \
	InitAxisBinding(KeyBindings::name, config_name, default_value);
#include "KeyBindings.inc.h"
}

void InitBindings()
{
	UpdateBindings();
	Pi::config->Save();
}

void EnableBindings()
{
	m_disableBindings = 0;
}

void DisableBindings()
{
	m_disableBindings = 1;
}

}
