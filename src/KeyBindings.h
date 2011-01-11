#ifndef KEYBINDINGS_H
#define KEYBINDINGS_H

#include "libs.h"

namespace KeyBindings {
	enum Type {
		KEYBOARD_KEY,
		JOYSTICK_BUTTON,
		JOYSTICK_HAT
	};

	struct KeyBinding {
		Type type;

		union {
			struct {
				SDLKey key;
				SDLMod mod;
			} keyboard;

			struct {
				Uint8 joystick;
				Uint8 button;
			} joystickButton;

			struct {
				Uint8 joystick;
				Uint8 hat;
				Uint8 direction;
			} joystickHat;
		} u;

		bool IsActive();
	};

	enum AxisDirection {
		POSITIVE,
		NEGATIVE
	};

	struct AxisBinding {
		Uint8 joystick;
		Uint8 axis;
		AxisDirection direction;

		double GetValue();
	};

	struct BindingPrototype {
		const char *label, *function;
	};

	extern const BindingPrototype bindingProtos[];

	void SetDefaults();
	void OnKeyBindingsChanged();

	extern KeyBinding pitchUp;
	extern KeyBinding pitchDown;
	extern KeyBinding yawLeft;
	extern KeyBinding yawRight;
	extern KeyBinding rollLeft;
	extern KeyBinding rollRight;
	extern KeyBinding thrustForward;
	extern KeyBinding thrustBackwards;
	extern KeyBinding thrustUp;
	extern KeyBinding thrustDown;
	extern KeyBinding thrustLeft;
	extern KeyBinding thrustRight;
	extern KeyBinding increaseSpeed;
	extern KeyBinding decreaseSpeed;
	extern KeyBinding fireLaser;
	extern KeyBinding fastRotate;
	extern KeyBinding targetObject;

	extern AxisBinding pitchAxis;
	extern AxisBinding rollAxis;
	extern AxisBinding yawAxis;
	extern AxisBinding thrustRightAxis;
	extern AxisBinding thrustUpAxis;
	extern AxisBinding thrustForwardAxis;
};

#endif /* KEYBINDINGS_H */
