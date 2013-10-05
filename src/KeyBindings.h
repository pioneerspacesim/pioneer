// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

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
				SDL_Keycode key;
				SDL_Keymod mod;
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
		bool Matches(const SDL_Keysym *sym) const;

		static KeyBinding keyboardBinding(SDL_Keycode key, SDL_Keymod mod);
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

	void InitBindings();
	void UpdateBindings();

	bool KeyBindingFromString(const std::string &str, KeyBinding *kb);
	KeyBinding KeyBindingFromString(const std::string &str);
	std::string KeyBindingToString(const KeyBinding &kb);
	bool AxisBindingFromString(const std::string &str, AxisBinding *ab);
	AxisBinding AxisBindingFromString(const std::string &str);
	std::string AxisBindingToString(const AxisBinding &ab);

	void DispatchSDLEvent(const SDL_Event *event);

#define KEY_BINDING(name,a,b,c) extern KeyAction name;
#define AXIS_BINDING(name,a,b,c) extern AxisBinding name;
#include "KeyBindings.inc.h"

#define BINDING_PAGE(name) extern const BindingPrototype BINDING_PROTOS_ ## name[];
#include "KeyBindings.inc.h"

} // namespace KeyBindings

#endif /* KEYBINDINGS_H */
