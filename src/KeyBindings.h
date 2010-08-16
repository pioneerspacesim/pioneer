#ifndef KEYBINDINGS_H
#define KEYBINDINGS_H

#include "libs.h"

namespace KeyBindings {

	/**
	 * Difference between KeyBinding and KeyModBinding is that if
	 * KeyModBinding::mod == 0 then no modifier keys may be pressed for
	 * IsActive() to be true. With KeyBinding, if mod == 0 then modifier
	 * keys are simply ignored.
	 *
	 * Things like the F1 vs shift-F1 functionality (switch external view
	 * vs set time accel) should be KeyModBinding, since mod key state is
	 * important. Whereas ship thrust and rotation controls like to ignore
	 * modifier keys.
	 */
	struct KeyBinding {
		SDLKey key;
		SDLMod mod;
		bool IsActive();
	};

	struct KeyModBinding {
		SDLKey key;
		SDLMod mod;
		bool IsActive();
	};

	struct KeyBindingPrototype {
		const char *label, *function;
		SDLKey defaultKey;
		SDLMod defaultMod;
	};
	extern const KeyBindingPrototype bindingProtos[];

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
};

#endif /* KEYBINDINGS_H */
