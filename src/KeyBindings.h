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
		std::string Description() const;

		static KeyBinding keyboardBinding(SDLKey key, SDLMod mod);
	};

	enum AxisDirection {
		POSITIVE,
		NEGATIVE
	};

	struct AxisBinding {
		Uint8 joystick;
		Uint8 axis;
		AxisDirection direction;

		AxisBinding();
		AxisBinding(Uint8 joystick, Uint8 axis, AxisDirection direction);
		float GetValue();
		std::string Description() const;
	};

	struct BindingPrototype {
		const char *label, *function;
	};

	extern const BindingPrototype bindingProtos[];
	extern const BindingPrototype axisBindingProtos[];

	void SetDefaults();
	void OnKeyBindingsChanged();

	bool KeyBindingFromString(const std::string &str, KeyBinding *kb);
	KeyBinding KeyBindingFromString(const std::string &str);
	std::string KeyBindingToString(const KeyBinding &kb);
	bool AxisBindingFromString(const std::string &str, AxisBinding *ab);
	AxisBinding AxisBindingFromString(const std::string &str);
	std::string AxisBindingToString(const AxisBinding &ab);

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
}

#endif /* KEYBINDINGS_H */
