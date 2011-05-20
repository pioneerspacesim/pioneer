#include "KeyBindings.h"
#include "Pi.h"

#include <sstream>

namespace KeyBindings {

KeyBinding pitchUp;
KeyBinding pitchDown;
KeyBinding yawLeft;
KeyBinding yawRight;
KeyBinding rollLeft;
KeyBinding rollRight;
KeyBinding thrustForward;
KeyBinding thrustBackwards;
KeyBinding thrustUp;
KeyBinding thrustDown;
KeyBinding thrustLeft;
KeyBinding thrustRight;
KeyBinding increaseSpeed;
KeyBinding decreaseSpeed;
KeyBinding fireLaser;
KeyBinding fastRotate;
KeyBinding targetObject;

AxisBinding pitchAxis;
AxisBinding rollAxis;
AxisBinding yawAxis;

KeyBinding KeyBinding::keyboardBinding(SDLKey key, SDLMod mod) {
	KeyBinding kb;

	kb.type = KEYBOARD_KEY;
	kb.u.keyboard.key  = key;
	kb.u.keyboard.mod  = mod;

	return kb;
}

bool KeyBinding::IsActive()
{
	if (type == KEYBOARD_KEY) {
		// 0xfff filters out numlock, capslock and other shit
		if (u.keyboard.mod != 0)
			return Pi::KeyState(u.keyboard.key) && ((Pi::KeyModState()&0xfff) == u.keyboard.mod);

		return Pi::KeyState(u.keyboard.key) != 0;

	} else if (type == JOYSTICK_BUTTON) {
		return Pi::JoystickButtonState(u.joystickButton.joystick, u.joystickButton.button) != 0;
	} else if (type == JOYSTICK_HAT) {
		return Pi::JoystickHatState(u.joystickHat.joystick, u.joystickHat.hat) == u.joystickHat.direction;
	} else
		abort();

	return false;
}

std::string KeyBinding::Description() const {
	std::ostringstream oss;

	if (type == KEYBOARD_KEY) {
		if (u.keyboard.mod & KMOD_SHIFT) oss << "shift ";
		if (u.keyboard.mod & KMOD_CTRL) oss << "ctrl ";
		if (u.keyboard.mod & KMOD_ALT) oss << "alt ";
		if (u.keyboard.mod & KMOD_META) oss << "meta ";
		oss << SDL_GetKeyName(u.keyboard.key);
	} else if (type == JOYSTICK_BUTTON) {
		oss << "Joy" << int(u.joystickButton.joystick);
		oss << " Button " << int(u.joystickButton.button);
	} else if (type == JOYSTICK_HAT) {
		oss << "Joy" << int(u.joystickHat.joystick);
		oss << " Hat" << int(u.joystickHat.hat);
		oss << " Dir " << int(u.joystickHat.direction);
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
	const char *axis_names[] = {"X", "Y", "Z"};
	std::ostringstream oss;

	if (direction == KeyBindings::NEGATIVE)
		oss << '-';

	oss << "Joy" << int(joystick) << ' ';

	if (0 <= axis && axis < 3)
		oss << axis_names[axis];
	else
		oss << int(axis);

	oss << " Axis";

	return oss.str();
}

const BindingPrototype bindingProtos[] = {
	{ "Weapons", 0 },
	{ "Target object in crosshairs", "BindTargetObject" },
	{ "Fire laser", "BindFireLaser" },
	{ "Ship orientation", 0 },
	{ "Fast rotational control", "BindFastRotate" },
	{ "Pitch up", "BindPitchUp" },
	{ "Pitch down", "BindPitchDown" },
	{ "Yaw left", "BindYawLeft" },
	{ "Yaw right", "BindYawRight" },
	{ "Roll left", "BindRollLeft" },
	{ "Roll right", "BindRollRight" },
	{ "Manual control mode", 0 },
	{ "Thrust forward", "BindThrustForward" },
	{ "Thrust backwards", "BindThrustBackwards" },
	{ "Thrust up", "BindThrustUp" },
	{ "Thrust down", "BindThrustDown" },
	{ "Thrust left", "BindThrustLeft" },
	{ "Thrust right", "BindThrustRight" },
	{ "Speed control mode", 0 },
	{ "Increase set speed", "BindIncreaseSpeed" },
	{ "Decrease set speed", "BindDecreaseSpeed" },
	{ 0, 0 },
};

const BindingPrototype axisBindingProtos[] = {
	{ "Joystick input", 0 },
	{ "Pitch", "BindAxisPitch" },
	{ "Roll", "BindAxisRoll" },
	{ "Yaw", "BindAxisYaw" },
	{ 0, 0 },
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

#define SET_KEY_BINDING(var,bindname) \
	KeyBindingFromString(Pi::config.String(bindname).c_str(), &(var));

#define SET_AXIS_BINDING(var, bindname) \
	AxisBindingFromString(Pi::config.String(bindname).c_str(), &(var));

void OnKeyBindingsChanged()
{
	SET_KEY_BINDING(pitchUp, "BindPitchUp");
	SET_KEY_BINDING(pitchDown, "BindPitchDown");
	SET_KEY_BINDING(yawLeft, "BindYawLeft");
	SET_KEY_BINDING(yawRight, "BindYawRight");
	SET_KEY_BINDING(rollLeft, "BindRollLeft");
	SET_KEY_BINDING(rollRight, "BindRollRight");
	SET_KEY_BINDING(thrustForward, "BindThrustForward");
	SET_KEY_BINDING(thrustBackwards, "BindThrustBackwards");
	SET_KEY_BINDING(thrustUp, "BindThrustUp");
	SET_KEY_BINDING(thrustDown, "BindThrustDown");
	SET_KEY_BINDING(thrustLeft, "BindThrustLeft");
	SET_KEY_BINDING(thrustRight, "BindThrustRight");
	SET_KEY_BINDING(increaseSpeed, "BindIncreaseSpeed");
	SET_KEY_BINDING(decreaseSpeed, "BindDecreaseSpeed");
	SET_KEY_BINDING(fireLaser, "BindFireLaser");
	SET_KEY_BINDING(fastRotate, "BindFastRotate");
	SET_KEY_BINDING(targetObject, "BindTargetObject");
	//SET_KEY_BINDING(key, "Bind");

	SET_AXIS_BINDING(pitchAxis, "BindAxisPitch");
	SET_AXIS_BINDING(rollAxis, "BindAxisRoll");
	SET_AXIS_BINDING(yawAxis, "BindAxisYaw");
}

static void SetSDLKeyboardBinding(const char *name, SDLKey key) {
	char buffer[64];
	snprintf(buffer, sizeof(buffer), "Key%i", int(key));
	Pi::config.SetString(name, buffer);
}

static void SetAxisBinding(const char *function, const AxisBinding &ab) {
	Pi::config.SetString(function, AxisBindingToString(ab).c_str());
}

void SetDefaults() 
{
	SetSDLKeyboardBinding("BindTargetObject", SDLK_TAB);
	SetSDLKeyboardBinding("BindFireLaser", SDLK_SPACE);
	SetSDLKeyboardBinding("BindFastRotate", SDLK_LSHIFT);
	SetSDLKeyboardBinding("BindPitchUp", SDLK_s);
	SetSDLKeyboardBinding("BindPitchDown", SDLK_w);
	SetSDLKeyboardBinding("BindYawLeft", SDLK_a);
	SetSDLKeyboardBinding("BindYawRight", SDLK_d);
	SetSDLKeyboardBinding("BindRollLeft", SDLK_q);
	SetSDLKeyboardBinding("BindRollRight", SDLK_e);
	SetSDLKeyboardBinding("BindThrustForward", SDLK_i);
	SetSDLKeyboardBinding("BindThrustBackwards", SDLK_k);
	SetSDLKeyboardBinding("BindThrustUp", SDLK_u);
	SetSDLKeyboardBinding("BindThrustDown", SDLK_o);
	SetSDLKeyboardBinding("BindThrustLeft", SDLK_j);
	SetSDLKeyboardBinding("BindThrustRight", SDLK_l);
	SetSDLKeyboardBinding("BindIncreaseSpeed", SDLK_RETURN);
	SetSDLKeyboardBinding("BindDecreaseSpeed", SDLK_RSHIFT);

	SetAxisBinding("BindAxisPitch", AxisBindingFromString("-Joy0Axis1"));
	SetAxisBinding("BindAxisRoll", AxisBindingFromString("Joy0Axis2"));
	SetAxisBinding("BindAxisYaw", AxisBindingFromString("Joy0Axis0"));

	OnKeyBindingsChanged();
}

}
