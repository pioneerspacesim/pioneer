#include "KeyBindings.h"
#include "Pi.h"
#include "Lang.h"
#include <string>
#include <sstream>

namespace KeyBindings {

KeyAction pitchUp;
KeyAction pitchDown;
KeyAction yawLeft;
KeyAction yawRight;
KeyAction rollLeft;
KeyAction rollRight;
KeyAction thrustForward;
KeyAction thrustBackwards;
KeyAction thrustUp;
KeyAction thrustDown;
KeyAction thrustLeft;
KeyAction thrustRight;
KeyAction increaseSpeed;
KeyAction decreaseSpeed;
KeyAction fireLaser;
KeyAction fastRotate;
KeyAction targetObject;
KeyAction toggleLuaConsole;
KeyAction toggleScanMode;
KeyAction increaseScanRange;
KeyAction decreaseScanRange;

AxisBinding pitchAxis;
AxisBinding rollAxis;
AxisBinding yawAxis;

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
	std::ostringstream oss;

	if (direction == KeyBindings::NEGATIVE)
		oss << '-';

	oss << Lang::JOY << int(joystick) << ' ';

	if (0 <= axis && axis < 3)
		oss << axis_names[axis];
	else
		oss << int(axis);

	oss << Lang::AXIS;

	return oss.str();
}

const BindingPrototype bindingProtos[] = {
	{ Lang::WEAPONS, 0, 0, 0 },
	{ Lang::TARGET_OBJECT_IN_SIGHTS, "BindTargetObject", &targetObject, 0 },
	{ Lang::FIRE_LASER, "BindFireLaser", &fireLaser, 0 },
	{ Lang::SHIP_ORIENTATION, 0, 0, 0 },
	{ Lang::FAST_ROTATION_CONTROL, "BindFastRotate", &fastRotate, 0 },
	{ Lang::PITCH_UP, "BindPitchUp", &pitchUp, 0 },
	{ Lang::PITCH_DOWN, "BindPitchDown", &pitchDown, 0 },
	{ Lang::YAW_LEFT, "BindYawLeft", &yawLeft, 0 },
	{ Lang::YAW_RIGHT, "BindYawRight", &yawRight, 0 },
	{ Lang::ROLL_LEFT, "BindRollLeft", &rollLeft, 0 },
	{ Lang::ROLL_RIGHT, "BindRollRight", &rollRight, 0 },
	{ Lang::MANUAL_CONTROL_MODE, 0, 0, 0 },
	{ Lang::THRUSTER_MAIN, "BindThrustForward", &thrustForward, 0 },
	{ Lang::THRUSTER_RETRO, "BindThrustBackwards", &thrustBackwards, 0 },
	{ Lang::THRUSTER_VENTRAL, "BindThrustUp", &thrustUp, 0 },
	{ Lang::THRUSTER_DORSAL, "BindThrustDown", &thrustDown, 0 },
	{ Lang::THRUSTER_PORT, "BindThrustLeft", &thrustLeft, 0 },
	{ Lang::THRUSTER_STARBOARD, "BindThrustRight", &thrustRight, 0 },
	{ Lang::SPEED_CONTROL_MODE, 0, 0, 0 },
	{ Lang::INCREASE_SET_SPEED, "BindIncreaseSpeed", &increaseSpeed, 0 },
	{ Lang::DECREASE_SET_SPEED, "BindDecreaseSpeed", &decreaseSpeed, 0 },
	{ Lang::SCANNER_CONTROL, 0, 0, 0 },
	{ Lang::TOGGLE_SCAN_MODE, "BindToggleScanMode", &toggleScanMode, 0 },
	{ Lang::INCREASE_SCAN_RANGE, "BindIncreaseScanRange", &increaseScanRange, 0 },
	{ Lang::DECREASE_SCAN_RANGE, "BindDecreaseScanRange", &decreaseScanRange, 0 },
	{ Lang::TOGGLE_LUA_CONSOLE, "BindToggleLuaConsole", &toggleLuaConsole, 0 },
	{ 0, 0, 0, 0 },
};

const BindingPrototype axisBindingProtos[] = {
	{ Lang::JOYSTICK_INPUT, 0, 0, 0 },
	{ Lang::PITCH, "BindAxisPitch", 0, &pitchAxis },
	{ Lang::ROLL, "BindAxisRoll", 0, &rollAxis },
	{ Lang::YAW, "BindAxisYaw", 0, &yawAxis },
	{ 0, 0, 0, 0 },
};

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
	for (int i = 0; bindingProtos[i].label; ++i) {
		KeyAction *kb = bindingProtos[i].kb;
		if (kb)
			kb->CheckSDLEventAndDispatch(event);
	}
}

void InitKeyBinding(KeyAction &kb, const std::string &bindName, SDLKey defaultKey) {
	std::string keyName = Pi::config->String(bindName.c_str());
	if (keyName.length() == 0) {
		keyName = stringf("Key%0{u}", uint32_t(defaultKey));
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
	InitKeyBinding(KeyBindings::pitchUp, "BindPitchUp", SDLK_s);
	InitKeyBinding(KeyBindings::pitchDown, "BindPitchDown", SDLK_w);
	InitKeyBinding(KeyBindings::yawLeft, "BindYawLeft", SDLK_a);
	InitKeyBinding(KeyBindings::yawRight, "BindYawRight", SDLK_d);
	InitKeyBinding(KeyBindings::rollLeft, "BindRollLeft", SDLK_q);
	InitKeyBinding(KeyBindings::rollRight, "BindRollRight", SDLK_e);
	InitKeyBinding(KeyBindings::thrustForward, "BindThrustForward", SDLK_i);
	InitKeyBinding(KeyBindings::thrustBackwards, "BindThrustBackwards", SDLK_k);
	InitKeyBinding(KeyBindings::thrustUp, "BindThrustUp", SDLK_u);
	InitKeyBinding(KeyBindings::thrustDown, "BindThrustDown", SDLK_o);
	InitKeyBinding(KeyBindings::thrustLeft, "BindThrustLeft", SDLK_j);
	InitKeyBinding(KeyBindings::thrustRight, "BindThrustRight", SDLK_l);
	InitKeyBinding(KeyBindings::increaseSpeed, "BindIncreaseSpeed", SDLK_RETURN);
	InitKeyBinding(KeyBindings::decreaseSpeed, "BindDecreaseSpeed", SDLK_RSHIFT);
	InitKeyBinding(KeyBindings::targetObject, "BindTargetObject", SDLK_TAB);
	InitKeyBinding(KeyBindings::fireLaser, "BindFireLaser", SDLK_SPACE);
	InitKeyBinding(KeyBindings::fastRotate, "BindFastRotate", SDLK_LSHIFT);
	InitKeyBinding(KeyBindings::toggleScanMode, "BindToggleScanMode", SDLK_BACKSLASH);
	InitKeyBinding(KeyBindings::increaseScanRange, "BindIncreaseScanRange", SDLK_RIGHTBRACKET);
	InitKeyBinding(KeyBindings::decreaseScanRange, "BindDecreaseScanRange", SDLK_LEFTBRACKET);
	InitKeyBinding(KeyBindings::toggleLuaConsole, "BindToggleLuaConsole", SDLK_BACKQUOTE);

	InitAxisBinding(KeyBindings::pitchAxis, "BindAxisPitch", "-Joy0Axis1");
	InitAxisBinding(KeyBindings::rollAxis, "BindAxisRoll", "Joy0Axis2");
	InitAxisBinding(KeyBindings::yawAxis, "BindAxisYaw", "Joy0Axis0");
}

void InitBindings()
{
	UpdateBindings();
	Pi::config->Save();
}

}
