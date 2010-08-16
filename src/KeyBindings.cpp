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
	// 0xfff filters out numlock, capslock and other shit
	if (mod) return Pi::KeyState(key) && ((Pi::KeyModState()&0xfff) == mod);
	return Pi::KeyState(key);
}

bool KeyModBinding::IsActive()
{
	// 0xfff filters out numlock, capslock and other shit
	return Pi::KeyState(key) && ((Pi::KeyModState()&0xfff) == mod);
}

const KeyBindingPrototype bindingProtos[] = {
	{ "Weapons", 0 },
	{ "Target object in crosshairs", "BindTargetObject", SDLK_TAB },
	{ "Fire laser", "BindFireLaser", SDLK_SPACE },
	{ "Ship orientation", 0 },
	{ "Fast rotational control", "BindFastRotate", SDLK_LSHIFT },
	{ "Pitch up", "BindPitchUp", SDLK_s },
	{ "Pitch down", "BindPitchDown", SDLK_w },
	{ "Yaw left", "BindYawLeft", SDLK_a },
	{ "Yaw right", "BindYawRight", SDLK_d },
	{ "Roll left", "BindRollLeft", SDLK_q },
	{ "Roll right", "BindRollRight", SDLK_e },
	{ "Manual control mode", 0 },
	{ "Thrust forward", "BindThrustForward", SDLK_i },
	{ "Thrust backwards", "BindThrustBackwards", SDLK_k },
	{ "Thrust up", "BindThrustUp", SDLK_u },
	{ "Thrust down", "BindThrustDown", SDLK_o },
	{ "Thrust left", "BindThrustLeft", SDLK_j },
	{ "Thrust right", "BindThrustRight", SDLK_l },
	{ "Speed control mode", 0 },
	{ "Increase set speed", "BindIncreaseSpeed", SDLK_RETURN },
	{ "Decrease set speed", "BindDecreaseSpeed", SDLK_RSHIFT },
	{ 0, 0 },
};

#define SET_KEY_BINDING(var,bindname) \
	(var).key = (SDLKey)Pi::config.Int(bindname "Key"); \
	(var).mod = (SDLMod)Pi::config.Int(bindname "Mod");

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

void SetDefaults() 
{
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
}

};

