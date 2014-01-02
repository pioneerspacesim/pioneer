// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef KEYBINDINGS_H
#define KEYBINDINGS_H

#include "libs.h"

namespace KeyBindings {
	enum Type {
		BINDING_DISABLED,
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
		bool IsActive() const;
		bool Matches(const SDL_Keysym *sym) const;
		bool Matches(const SDL_JoyButtonEvent *joy) const;
		bool Matches(const SDL_JoyHatEvent *joy) const;

		void Clear() {
			memset(this, 0, sizeof(*this));
		}

		static bool FromString(const char *str, KeyBinding &binding);
		static KeyBinding FromString(const char *str);
		std::string ToString() const;

		static KeyBinding keyboardBinding(SDL_Keycode key, SDL_Keymod mod);
	};

	struct KeyAction {
		KeyBinding binding1;
		KeyBinding binding2;

		sigc::signal<void> onPress;
		sigc::signal<void> onRelease;

		void SetFromString(const char *str);
		std::string ToString() const;

		bool IsActive() const;
		void CheckSDLEventAndDispatch(const SDL_Event *event);

		bool Matches(const SDL_Keysym *sym) const;
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

		void Clear() {
			memset(this, 0, sizeof(*this));
		}

		static bool FromString(const char *str, AxisBinding &binding);
		static AxisBinding FromString(const char *str);
		std::string ToString() const;
	};

	struct BindingPrototype {
		const char *label, *function;
		KeyAction *kb;
		AxisBinding *ab;
	};

	void InitBindings();
	void UpdateBindings();

	void DispatchSDLEvent(const SDL_Event *event);

#define KEY_BINDING(name,a,b,c,d) extern KeyAction name;
#define AXIS_BINDING(name,a,b,c) extern AxisBinding name;
#include "KeyBindings.inc.h"

#define BINDING_PAGE(name) extern const BindingPrototype BINDING_PROTOS_ ## name[];
#include "KeyBindings.inc.h"

} // namespace KeyBindings

#endif /* KEYBINDINGS_H */
