// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef KEYBINDINGS_H
#define KEYBINDINGS_H

#include "libs.h"
#include <iosfwd>

namespace KeyBindings {
	enum Type {
		BINDING_DISABLED,
		KEYBOARD_KEY,
		JOYSTICK_BUTTON,
		JOYSTICK_HAT,
		MOUSE_BUTTON // TODO: implementme!
	};

	struct KeyBinding {
	public:
		// constructors
		static bool FromString(const char *str, KeyBinding &binding);
		static KeyBinding FromString(const char *str);
		static KeyBinding FromKeyMod(SDL_Keycode key, SDL_Keymod mod);
		static KeyBinding FromJoystickButton(Uint8 joystick, Uint8 button);
		static KeyBinding FromJoystickHat(Uint8 joystick, Uint8 hat, Uint8 direction);

		KeyBinding() :
			type(BINDING_DISABLED)
		{
			u.keyboard.key = SDLK_UNKNOWN;
			u.keyboard.mod = KMOD_NONE;
		}
		KeyBinding(SDL_Keycode key, SDL_Keymod mod = KMOD_NONE) :
			type(KEYBOARD_KEY)
		{
			u.keyboard.key = key;
			u.keyboard.mod = mod;
		}

		std::string ToString() const; // for serialisation
		std::string Description() const; // for display to the user

		bool IsActive() const;
		bool Matches(const SDL_Keysym *sym) const;
		bool Matches(const SDL_JoyButtonEvent *joy) const;
		bool Matches(const SDL_JoyHatEvent *joy) const;

		void Clear() { memset(this, 0, sizeof(*this)); }

		bool Enabled() const { return (type != BINDING_DISABLED); }

		friend std::ostream &operator<<(std::ostream &oss, const KeyBinding &kb);

	private:
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

			/* TODO: implement binding mouse buttons.
				struct {
					Uint8 button;
					// TODO: implement binding multiple clicks as their own action.
					Uint8 clicks;
				} mouseButton;
				*/
		} u;
	};

	struct ActionBinding {
		KeyBinding binding1;
		KeyBinding binding2;

		sigc::signal<void> onPress;
		sigc::signal<void> onRelease;

		ActionBinding() {}
		ActionBinding(KeyBinding b1, KeyBinding b2 = KeyBinding()) :
			binding1(b1),
			binding2(b2) {}
		// This constructor is just a programmer shortcut.
		ActionBinding(SDL_Keycode k1, SDL_Keycode k2 = SDLK_UNKNOWN)
		{
			binding1 = KeyBinding(k1);
			if (k2 != SDLK_UNKNOWN) binding2 = KeyBinding(k2);
		}

		void SetFromString(const char *str);
		void SetFromString(const std::string str) { return SetFromString(str.c_str()); }
		std::string ToString() const;

		bool IsActive() const;
		void CheckSDLEventAndDispatch(const SDL_Event *event);

		bool Matches(const SDL_Keysym *sym) const;
	};

	enum AxisDirection {
		POSITIVE,
		NEGATIVE
	};

	struct JoyAxisBinding {
	public:
		JoyAxisBinding() :
			joystick(JOYSTICK_DISABLED),
			axis(0),
			direction(POSITIVE),
			deadzone(0.0f),
			sensitivity(1.0f) {}
		JoyAxisBinding(Uint8 joystick_, Uint8 axis_, AxisDirection direction_, float deadzone_ = 0.0f, float sensitivity_ = 1.0f) :
			joystick(joystick_),
			axis(axis_),
			direction(direction_),
			deadzone(deadzone_),
			sensitivity(sensitivity_) {}

		float GetValue() const;
		std::string Description() const;

		void Clear()
		{
			joystick = JOYSTICK_DISABLED;
			axis = 0;
			direction = POSITIVE;
			deadzone = 0.0f;
			sensitivity = 1.0f;
		}

		bool Enabled() const { return (joystick != JOYSTICK_DISABLED); }

		static bool FromString(const char *str, JoyAxisBinding &binding);
		static JoyAxisBinding FromString(const char *str);
		std::string ToString() const;

		bool Matches(const SDL_Event *event) const;
		bool IsActive() const;

		bool IsInverted() { return direction == NEGATIVE; }
		AxisDirection GetDirection() { return direction; }
		void SetDirection(AxisDirection dir) { direction = dir; }

		float GetDeadzone() { return deadzone; }
		void SetDeadzone(float dz) { deadzone = dz; }

		float GetSensitivity() { return sensitivity; }
		void SetSensitivity(float sens) { sensitivity = sens; }

	private:
		enum { JOYSTICK_DISABLED = Uint8(-1) };
		Uint8 joystick;
		Uint8 axis;
		AxisDirection direction;
		float deadzone;
		float sensitivity;
	};

	struct AxisBinding {
		JoyAxisBinding axis;
		KeyBinding positive;
		KeyBinding negative;

		AxisBinding() {}
		AxisBinding(JoyAxisBinding ax, KeyBinding pos = KeyBinding(), KeyBinding neg = KeyBinding()) :
			axis(ax),
			positive(pos),
			negative(neg) {}
		// This constructor is just a programmer shortcut.
		AxisBinding(SDL_Keycode k1, SDL_Keycode k2) :
			positive(KeyBinding(k1)),
			negative(KeyBinding(k2)) {}

		sigc::signal<void, float> onAxis;

		void SetFromString(const char *str) { return SetFromString(std::string(str)); }
		void SetFromString(const std::string str);
		std::string ToString() const;

		bool IsActive() const;
		float GetValue() const;
		void CheckSDLEventAndDispatch(const SDL_Event *event);
	};

	struct BindingPrototype {
		const char *label, *function;
		ActionBinding *kb;
		JoyAxisBinding *ab;
	};

	void InitBindings();
	void UpdateBindings();
	void EnableBindings();
	void DisableBindings();

	void DispatchSDLEvent(const SDL_Event *event);

#define KEY_BINDING(name, a, b, c, d) extern ActionBinding name;
#define AXIS_BINDING(name, a, b, c) extern JoyAxisBinding name;
#include "KeyBindings.inc.h"

#define BINDING_PAGE(name) extern const BindingPrototype BINDING_PROTOS_##name[];
#include "KeyBindings.inc.h"

} // namespace KeyBindings

#endif /* KEYBINDINGS_H */
