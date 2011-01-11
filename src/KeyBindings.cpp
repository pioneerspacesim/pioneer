#include "KeyBindings.h"
#include "Pi.h"

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

bool KeyBinding::IsActive()
{
	if (type == KEYBOARD_KEY) {
		// 0xfff filters out numlock, capslock and other shit
		if (u.keyboard.mod != 0)
			return Pi::KeyState(u.keyboard.key) && ((Pi::KeyModState()&0xfff) == u.keyboard.mod);

		return Pi::KeyState(u.keyboard.key);

	} else if (type == JOYSTICK_BUTTON) {
	} else if (type == JOYSTICK_HAT) {
	} else
		abort();

	return false;
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

/**
 * Exampe strings:
 *   Key55
 *   Joy0Button2
 *   Joy0Hat0Dir3
 */
static bool KeyBindingFromString(const char *str, KeyBinding *kb)
{
	const char *digits = "1234567890";

	if (strncmp(str, "Key", 3) == 0) {
		kb->type = KEYBOARD_KEY;
		str += 3;

		kb->u.keyboard.key = (SDLKey) atoi(str);
		str += strspn(str, digits);

		if (strncmp(str, "Mod", 3) == 0) {
			str += 3;
			kb->u.keyboard.mod = (SDLMod) atoi(str);
		}

		return true;

	} else if (strncmp(str, "Joy", 3) == 0) {
		str += 3;

		int joy = atoi(str);
		str += strspn(str, digits);

		if (strncmp(str, "Button", 6) == 0) {
			str += 6;
			kb->type = JOYSTICK_BUTTON;
			kb->u.joystickButton.joystick = joy;
			kb->u.joystickButton.button = atoi(str);
			return true;
		} else if (strncmp(str, "Hat", 3) == 0) {
			str += 3;
			kb->type = JOYSTICK_HAT;
			kb->u.joystickHat.joystick = joy;
			kb->u.joystickHat.hat = atoi(str);
			str += strspn(str, digits);

			if (strncmp(str, "Dir", 3) != 0)
				return false;

			str += 3;
			kb->u.joystickHat.direction = atoi(str);
			return true;
		}

		return false;
	}

	return false;
}

#define SET_KEY_BINDING(var,bindname) \
	KeyBindingFromString(Pi::config.String(bindname).c_str(), &(var));

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
}

static void SetSDLKeyboardBinding(const char *name, SDLKey key) {
	char buffer[64];
	snprintf(buffer, sizeof(buffer), "Key%i", (int) key);
	Pi::config.SetString(name, buffer);
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
#if 0
	for (int i=0; bindingProtos[i].label; i++) {
		// skip group labels
		if (bindingProtos[i].function == 0) continue;
		char buf[128];
		snprintf(buf, sizeof(buf), "%sKey", bindingProtos[i].function);
		Pi::config.SetInt(buf, bindingProtos[i].defaultKey);
		snprintf(buf, sizeof(buf), "%sMod", bindingProtos[i].function);
		Pi::config.SetInt(buf, bindingProtos[i].defaultMod);
	}
	OnKeyBindingsChanged();
#endif
}

};
