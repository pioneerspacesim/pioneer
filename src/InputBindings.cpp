// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "InputBindings.h"
#include "Input.h"
#include "SDL_events.h"
#include "SDL_joystick.h"
#include "utils.h"

#include <ostream>
#include <regex>

using namespace InputBindings;

namespace Input {
	int JoystickFromGUIDString(const std::string &guid);
	std::string JoystickGUIDString(int joystickID);
} // namespace Input

// Helper function to early-out in Matches()
Response MatchType(const KeyBinding &k, const SDL_Event &ev)
{
	using Type = KeyBinding::Type;

	bool p = false;
	bool r = false;

	switch (k.type) {
	case Type::Disabled:
		return Response::Ignored;
	case Type::KeyboardKey:
		p = ev.type == SDL_KEYDOWN;
		r = ev.type == SDL_KEYUP;
		break;
	case Type::JoystickButton:
		p = ev.type == SDL_JOYBUTTONDOWN;
		r = ev.type == SDL_JOYBUTTONUP;
		break;
	case Type::JoystickHat:
		if (ev.type == SDL_JOYHATMOTION) {
			p = (ev.jhat.value & k.joystick.button) == k.joystick.button;
			r = !p;
		}
		break;
	case Type::MouseButton:
		p = ev.type == SDL_MOUSEBUTTONDOWN;
		r = ev.type == SDL_MOUSEBUTTONUP;
		break;
	}

	return p ? Response::Pressed : r ? Response::Released : Response::Ignored;
}

Response KeyBinding::Matches(const SDL_Event &ev) const
{
	Response r = MatchType(*this, ev);
	if (r == Response::Ignored)
		return r;

	if (type == Type::KeyboardKey) {
		return (ev.key.keysym.sym == keycode) ? r : Response::Ignored;
	} else if (type == Type::JoystickButton) {
		bool cond = (joystick.id == Input::JoystickFromID(ev.jbutton.which) && joystick.button == ev.jbutton.button);
		return cond ? r : Response::Ignored;
	} else if (type == Type::JoystickHat) {
		bool cond = (joystick.id == Input::JoystickFromID(ev.jhat.which) && joystick.hat == ev.jhat.hat);
		return cond ? r : Response::Ignored;
	} else if (type == Type::MouseButton) {
		return (mouse.button == ev.button.button) ? r : Response::Ignored;
	}

	return Response::Ignored;
}

bool KeyBinding::operator==(const KeyBinding &rhs) const
{
	if (type != rhs.type)
		return false;

	if (type == Type::KeyboardKey)
		return keycode == rhs.keycode;
	else if (type == Type::JoystickButton)
		return joystick.id == rhs.joystick.id && joystick.button == rhs.joystick.button;
	else if (type == Type::JoystickHat)
		return joystick.id == rhs.joystick.id && joystick.hat == rhs.joystick.hat && joystick.button == rhs.joystick.button;
	else if (type == Type::MouseButton)
		return mouse.button == rhs.mouse.button;
	else
		return true;
}

#define ret_if_different(val) \
	if (val != rhs.val) return val < rhs.val
bool KeyBinding::operator<(const KeyBinding &rhs) const
{
	ret_if_different(type);

	switch (type) {
	case Type::KeyboardKey:
		ret_if_different(keycode);
		break;
	case Type::JoystickButton:
		ret_if_different(joystick.id);
		ret_if_different(joystick.button);
		break;
	case Type::JoystickHat:
		ret_if_different(joystick.id);
		ret_if_different(joystick.hat);
		ret_if_different(joystick.button);
		break;
	case Type::MouseButton:
		ret_if_different(mouse.button);
		break;
	default:
		break;
	}
	// apparently they are the same
	return false;
}
#undef ret_if_different

Action &Action::operator=(const Action &rhs)
{
	binding = rhs.binding;
	binding2 = rhs.binding2;
	return *this;
}

Axis &Axis::operator=(const Axis &rhs)
{
	axis = rhs.axis;
	positive = rhs.positive;
	negative = rhs.negative;
	return *this;
}

// ============================================================================
//
// Config Loading
//
// ============================================================================

using smatch = std::match_results<std::string_view::const_iterator>;
static std::regex disabled_matcher("^disabled", std::regex::icase);

// Handle the fiddly bits of matching a regex and advancing the beginning of a string
bool consumeMatch(std::string_view &str, smatch &match_results, std::regex &reg)
{
	if (!std::regex_search(str.cbegin(), str.cend(), match_results, reg))
		return false;

	str.remove_prefix(std::distance(str.cbegin(), match_results[0].second));
	return true;
}

// Key54 | JoyGUID/B3 JoyGUID/H4/2 | Mouse5
// Less awful than iostreams, but still not elegant. That's C++ for you.
// TODO: save joystick id->GUID mapping separately in the config file and
// don't write them here to save space
std::string_view &InputBindings::operator>>(std::string_view &str, KeyBinding &out)
{
	static std::regex key_matcher("^Key(\\d+)");
	static std::regex joystick_matcher("^Joy([^/]{32})");
	static std::regex joystick_button("^/B(\\d+)");
	static std::regex joystick_hat("^/H(\\d+)/(\\d+)");
	static std::regex mouse_matcher("^Mouse(\\d+)");

	const auto begin = str.cbegin();
	const auto end = str.cend();

	// match_results[0].second should always point to the character after the
	// parsed key binding unless the value present could not be parsed.
	smatch match_results;
	if (std::regex_search(begin, end, match_results, key_matcher)) {
		out.type = KeyBinding::Type::KeyboardKey;
		out.keycode = std::stoi(match_results[1]);
	} else if (std::regex_search(begin, end, match_results, joystick_matcher)) {
		out.joystick.id = Input::JoystickFromGUIDString(match_results[1]);
		const auto start = match_results[0].second;
		if (std::regex_search(start, end, match_results, joystick_button)) {
			out.type = KeyBinding::Type::JoystickButton;
			out.joystick.button = std::stoi(match_results[1]);
			out.joystick.hat = 0;
		} else if (std::regex_search(start, end, match_results, joystick_hat)) {
			out.type = KeyBinding::Type::JoystickHat;
			out.joystick.hat = std::stoi(match_results[1]);
			out.joystick.button = std::stoi(match_results[2]);
		}
	} else if (std::regex_search(begin, end, match_results, mouse_matcher)) {
		out.type = KeyBinding::Type::MouseButton;
		out.mouse.button = std::stoi(match_results[1]);
	} else {
		out.type = KeyBinding::Type::Disabled;
		// consume the disabled text if present.
		std::regex_search(begin, end, match_results, disabled_matcher);
	}

	// return a string view containing the rest of the string
	if (!match_results.empty())
		str.remove_prefix(std::distance(begin, match_results[0].second));

	return str;
}

// Serialize a KeyBinding into the output stream.
// Writes nothing if the binding is disabled
std::ostream &InputBindings::operator<<(std::ostream &str, const KeyBinding &in)
{
	switch (in.type) {
	case KeyBinding::Type::KeyboardKey:
		return str << "Key" << in.keycode;
	case KeyBinding::Type::JoystickButton:
		return str << "Joy" << Input::JoystickGUIDString(in.joystick.id)
				   << "/B" << int(in.joystick.button);
	case KeyBinding::Type::JoystickHat:
		return str << "Joy" << Input::JoystickGUIDString(in.joystick.id)
				   << "/H" << int(in.joystick.hat) << "/" << int(in.joystick.button);
	case KeyBinding::Type::MouseButton:
		return str << "Mouse" << int(in.mouse.button);
	default:
		return str;
	}
}

// Match [-]JoyGUID/A4
std::string_view &InputBindings::operator>>(std::string_view &str, JoyAxis &out)
{
	static std::regex joy_matcher("^Joy([^/]{32})/A(\\d+)");
	auto begin = str.cbegin();

	bool reverse = !str.empty() && str[0] == '-';
	if (reverse)
		++begin;

	smatch match_results;
	if (std::regex_search(begin, str.cend(), match_results, joy_matcher)) {
		out.joystickId = Input::JoystickFromGUIDString(match_results[1]);
		out.axis = std::stoi(match_results[2]);
		out.direction = reverse ? -1 : 1;
	} else {
		std::regex_search(str.cbegin(), str.cend(), match_results, std::regex("^disabled"));
		out.direction = 0;
	}

	if (!match_results.empty())
		str.remove_prefix(std::distance(str.cbegin(), match_results[0].second));

	return str;
}

// [-]JoyGUID/A4
std::ostream &InputBindings::operator<<(std::ostream &str, const JoyAxis &in)
{
	if (!in.Enabled())
		return str << "disabled";

	return str << (in.direction < 0.0 ? "-Joy" : "Joy")
			   << Input::JoystickGUIDString(in.joystickId)
			   << "/A" << int(in.axis);
}

// find a close paren, copy str into ret str, and return retstr
// (for one-line failure case returns)
std::string_view &findCloseParen(std::string_view &str, std::string_view &retstr, smatch &match_results)
{
	if (std::regex_search(str.cbegin(), str.cend(), match_results, std::regex("\\)")))
		str.remove_prefix(std::distance(str.cbegin(), match_results[0].second));

	retstr = str;
	return retstr;
}

// Parse KeyChord(Key53 + JoyGUID/B3 + Mouse1) | KeyChord(Mouse5)
std::string_view &InputBindings::operator>>(std::string_view &str, KeyChord &out)
{
	static std::regex key_chord("^KeyChord\\(\\s*");
	static std::regex plus_sign("^\\s*\\+\\s*");
	smatch match_results;

	// Early-out for disabled key chord
	if (consumeMatch(str, match_results, disabled_matcher)) {
		// erase hardcoded defaults
		// (since the binding is present in the configuration file)
		out = KeyBinding{};
		return str;
	}

	// make a copy of the string view so we can nondestructively consume matches.
	std::string_view iterstr = str;

	// ensure we read the KeyChord( opening
	if (!consumeMatch(iterstr, match_results, key_chord))
		return str;

	// read the activator KeyBinding
	iterstr >> out.activator;

	// if the activator is disabled, early-out here.
	// if we don't have a following plus sign, discard everything to the next close-paren
	if (!out.activator.Enabled() || !consumeMatch(iterstr, match_results, plus_sign))
		return findCloseParen(iterstr, str, match_results);

	// read the first modifier
	iterstr >> out.modifier1;

	// ditto for the second modifier
	if (!out.modifier1.Enabled() || !consumeMatch(iterstr, match_results, plus_sign))
		return findCloseParen(iterstr, str, match_results);

	iterstr >> out.modifier2;

	return findCloseParen(iterstr, str, match_results);
}

// KeyChord(Key54 + JoyGUID/B4 + JoyGUID/H1/3)
std::ostream &InputBindings::operator<<(std::ostream &str, const KeyChord &in)
{
	if (!in.Enabled())
		return str << "disabled";

	str << "KeyChord(" << in.activator;
	if (in.modifier1.Enabled()) {
		str << " + " << in.modifier1;
		if (in.modifier2.Enabled())
			str << " + " << in.modifier2;
	}
	str << ")";

	return str;
}

std::string_view &InputBindings::operator>>(std::string_view &str, Axis &out)
{
	static std::regex input_axis("^InputAxis\\(\\s*");
	static std::regex comma_sep("^\\s*,\\s*");
	smatch match_results;

	auto iterstr = str;
	if (!consumeMatch(iterstr, match_results, input_axis))
		return str;
	iterstr >> out.axis;

	if (!consumeMatch(iterstr, match_results, comma_sep))
		return findCloseParen(iterstr, str, match_results);

	iterstr >> out.negative;

	if (!consumeMatch(iterstr, match_results, comma_sep))
		return findCloseParen(iterstr, str, match_results);
	iterstr >> out.positive;

	return findCloseParen(iterstr, str, match_results);
}

// InputAxis(-JoyGUID/A3, KeyChord(Key32), KeyChord(Mouse5 + JoyGUID/H1/1))
// InputAxis(disabled, disabled, disabled)
std::ostream &InputBindings::operator<<(std::ostream &str, const Axis &in)
{
	return str << "InputAxis(" << in.axis << ", " << in.negative << ", " << in.positive << ")";
}

std::string_view &InputBindings::operator>>(std::string_view &str, Action &out)
{
	static std::regex input_action("^InputAction\\(\\s*");
	static std::regex comma_sep("^\\s*,\\s*");
	smatch match_results;

	auto iterstr = str;
	if (!consumeMatch(iterstr, match_results, input_action))
		return str;
	iterstr >> out.binding;

	if (!consumeMatch(iterstr, match_results, comma_sep))
		return findCloseParen(iterstr, str, match_results);
	iterstr >> out.binding2;

	return findCloseParen(iterstr, str, match_results);
}

// InputAction(KeyChord(Key53 + Mouse4), KeyChord(Mouse5 + JoyGUID/H1/1))
// InputAction(disabled, disabled)
std::ostream &InputBindings::operator<<(std::ostream &str, const Action &in)
{
	return str << "InputAction(" << in.binding << ", " << in.binding2 << ")";
}
