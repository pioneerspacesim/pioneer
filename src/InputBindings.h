// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "DeleteEmitter.h"

#include <SDL_events.h>
#include <SDL_keycode.h>
#include <sigc++/sigc++.h>

#include <cstdint>
#include <iosfwd>
#include <string_view>

namespace InputBindings {
	enum class Response {
		Ignored = 0,
		Pressed,
		Released
	};

	struct KeyBinding {
		enum class Type : uint8_t {
			Disabled,
			KeyboardKey,
			JoystickButton,
			JoystickHat,
			MouseButton
		};

		Type type = Type::Disabled;
		union {
			SDL_Keycode keycode = 0;
			struct {
				// 65536 possible IDs should be more than plenty, even with lots of hot-plug noise.
				uint16_t id;
				// if type = JoystickHat, this is the hat direction; otherwise it's the button index
				uint8_t button;
				uint8_t hat;
			} joystick;
			struct {
				uint8_t button;
			} mouse;
		};

		KeyBinding() = default;

		KeyBinding(SDL_Keycode k) :
			type(Type::KeyboardKey), keycode(k) {}

		static KeyBinding JoystickButton(uint16_t joystickID, uint8_t button)
		{
			KeyBinding t;
			t.type = Type::JoystickButton;
			t.joystick = { joystickID, button, 0 };
			return t;
		}

		static KeyBinding JoystickHat(uint16_t joystickID, uint8_t hat, uint8_t direction)
		{
			KeyBinding t;
			t.type = Type::JoystickHat;
			t.joystick = { joystickID, direction, hat };
			return t;
		}

		static KeyBinding MouseButton(uint8_t mouseButton)
		{
			KeyBinding t;
			t.type = Type::MouseButton;
			t.mouse.button = mouseButton;
			return t;
		}

		static KeyBinding FromEvent(SDL_Event &event);

		bool Enabled() const { return type != Type::Disabled; }
		Response Matches(const SDL_Event &ev) const;

		bool operator==(const KeyBinding &rhs) const;
		bool operator<(const KeyBinding &rhs) const;

		// serialization
		friend std::string_view &operator>>(std::string_view &, KeyBinding &);
		friend std::ostream &operator<<(std::ostream &, const KeyBinding &);
	};

	struct JoyAxis {
		uint16_t joystickId;
		uint8_t axis;
		int8_t direction; // if 0, the axis is disabled

		bool Enabled() const { return direction != 0; }

		bool operator==(const JoyAxis &rhs) const
		{
			if (!direction && !rhs.direction)
				return true;

			return joystickId == rhs.joystickId && axis == rhs.axis && direction == rhs.direction;
		}

		// serialization
		friend std::string_view &operator>>(std::string_view &, JoyAxis &);
		friend std::ostream &operator<<(std::ostream &, const JoyAxis &);
	};

	struct KeyChord {
		KeyBinding activator;
		KeyBinding modifier1;
		KeyBinding modifier2;

		KeyChord() = default;
		KeyChord(KeyBinding a, KeyBinding m1 = {}, KeyBinding m2 = {}) :
			activator(a),
			modifier1(m1),
			modifier2(m2)
		{}

		bool IsActive() const { return Enabled() && m_active; }
		bool Enabled() const { return activator.Enabled(); }

		bool operator==(const KeyChord &rhs) const
		{
			return activator == rhs.activator && modifier1 == rhs.modifier1 && modifier2 == rhs.modifier2;
		}
		bool operator!=(const KeyChord &rhs) const { return !(*this == rhs); }

		// Groups chords by number of modifiers in descending order
		bool operator<(const KeyChord &rhs) const
		{
			return (modifier2.Enabled() && !rhs.modifier2.Enabled()) || (modifier1.Enabled() && !rhs.modifier1.Enabled());
		}

		bool m_active = false;
		uint8_t m_queuedEvents = 0;

		// serialization
		friend std::string_view &operator>>(std::string_view &, KeyChord &);
		friend std::ostream &operator<<(std::ostream &, const KeyChord &);
	};

	struct Action : public DeleteEmitter {
		KeyChord binding;
		KeyChord binding2;
		bool m_active = false;
		sigc::signal<void> onPressed;
		sigc::signal<void> onReleased;

		Action() = default;
		Action(KeyChord b1, KeyChord b2 = {}) :
			binding(b1),
			binding2(b2)
		{}

		// NOTE: sigc::signals cannot be copied, this function is for convenience to copy bindings only
		Action &operator=(const Action &rhs);

		bool IsActive() { return m_active; }
		bool Enabled() { return binding.Enabled() || binding2.Enabled(); }

		// serialization
		friend std::string_view &operator>>(std::string_view &, Action &);
		friend std::ostream &operator<<(std::ostream &, const Action &);
	};

	struct Axis : public DeleteEmitter {
		JoyAxis axis;
		KeyChord positive;
		KeyChord negative;
		float m_value;
		float m_manualValue;
		sigc::signal<void, float> onAxisValue;

		Axis() = default;
		Axis(JoyAxis a, KeyChord p = {}, KeyChord n = {}) :
			axis(a),
			positive(p),
			negative(n)
		{}

		Axis(KeyChord p, KeyChord n = {}) :
			axis{},
			positive(p),
			negative(n)
		{}

		// NOTE: sigc::signals cannot be copied, this function is for convenience to copy bindings only
		Axis &operator=(const Axis &rhs);

		bool IsActive() { return m_value != 0.0; }
		float GetValue() { return m_value; }
		// if we want to set the value of the axis, for example from the UI slider
		// must be remembered separately, because m_value is overwritten by data from the joystick
		void SetValue(float value) { m_manualValue = value; }
		bool Enabled() { return axis.Enabled() || positive.Enabled() || negative.Enabled(); }

		// serialization
		friend std::string_view &operator>>(std::string_view &, Axis &);
		friend std::ostream &operator<<(std::ostream &, const Axis &);
	};

	std::string_view &operator>>(std::string_view &, KeyBinding &);
	std::ostream &operator<<(std::ostream &, const KeyBinding &);

	std::string_view &operator>>(std::string_view &, JoyAxis &);
	std::ostream &operator<<(std::ostream &, const JoyAxis &);

	std::string_view &operator>>(std::string_view &, KeyChord &);
	std::ostream &operator<<(std::ostream &, const KeyChord &);

	std::string_view &operator>>(std::string_view &, Action &);
	std::ostream &operator<<(std::ostream &, const Action &);

	std::string_view &operator>>(std::string_view &, Axis &);
	std::ostream &operator<<(std::ostream &, const Axis &);
}; // namespace InputBindings
