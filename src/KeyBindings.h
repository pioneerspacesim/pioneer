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

		std::string Description() const;
		bool Matches(const SDL_keysym *sym) const;

		static KeyBinding keyboardBinding(SDLKey key, SDLMod mod);
	};

	struct KeyAction {
		KeyBinding binding;

		sigc::signal<void> onPress;
		sigc::signal<void> onRelease;

		bool IsActive() const;
		void CheckSDLEventAndDispatch(const SDL_Event *event);
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
		KeyAction *kb;
		AxisBinding *ab;
	};

	extern const BindingPrototype bindingProtos[];
	extern const BindingPrototype axisBindingProtos[];

	void InitBindings();
	void UpdateBindings();

	bool KeyBindingFromString(const std::string &str, KeyBinding *kb);
	KeyBinding KeyBindingFromString(const std::string &str);
	std::string KeyBindingToString(const KeyBinding &kb);
	bool AxisBindingFromString(const std::string &str, AxisBinding *ab);
	AxisBinding AxisBindingFromString(const std::string &str);
	std::string AxisBindingToString(const AxisBinding &ab);

	void DispatchSDLEvent(const SDL_Event *event);

	extern KeyAction pitchUp;
	extern KeyAction pitchDown;
	extern KeyAction yawLeft;
	extern KeyAction yawRight;
	extern KeyAction rollLeft;
	extern KeyAction rollRight;
	extern KeyAction thrustForward;
	extern KeyAction thrustBackwards;
	extern KeyAction thrustUp;
	extern KeyAction thrustDown;
	extern KeyAction thrustLeft;
	extern KeyAction thrustRight;
	extern KeyAction thrustLowPower;
	extern KeyAction increaseSpeed;
	extern KeyAction decreaseSpeed;
	extern KeyAction fireLaser;
	extern KeyAction targetObject;
	extern KeyAction toggleLuaConsole;
	extern KeyAction toggleScanMode;
	extern KeyAction increaseScanRange;
	extern KeyAction decreaseScanRange;
	extern KeyAction toggleHudMode;

	extern AxisBinding pitchAxis;
	extern AxisBinding rollAxis;
	extern AxisBinding yawAxis;
	extern AxisBinding thrustRightAxis;
	extern AxisBinding thrustUpAxis;
	extern AxisBinding thrustForwardAxis;
}

#endif /* KEYBINDINGS_H */
