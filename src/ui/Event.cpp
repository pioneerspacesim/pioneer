// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Event.h"
#include "LuaObject.h"
#include "EnumStrings.h"

namespace UI {

struct KeyMap {
	const char  *name;
	SDL_Keycode sym;
};
static const KeyMap keymap[] = {
	{ "backspace",   SDLK_BACKSPACE },
	{ "tab",         SDLK_TAB },
	{ "enter",       SDLK_RETURN },
	{ "pause",       SDLK_PAUSE },
	{ "esc",         SDLK_ESCAPE },
	{ "space",       SDLK_SPACE },
	{ "'",           SDLK_QUOTE },
	{ ",",           SDLK_COMMA },
	{ "-",           SDLK_MINUS },
	{ ".",           SDLK_PERIOD },
	{ "/",           SDLK_SLASH },
	{ "0",           SDLK_0 },
	{ "1",           SDLK_1 },
	{ "2",           SDLK_2 },
	{ "3",           SDLK_3 },
	{ "4",           SDLK_4 },
	{ "5",           SDLK_5 },
	{ "6",           SDLK_6 },
	{ "7",           SDLK_7 },
	{ "8",           SDLK_8 },
	{ "9",           SDLK_9 },
	{ ";",           SDLK_SEMICOLON },
	{ "=",           SDLK_EQUALS },
	{ "[",           SDLK_LEFTBRACKET },
	{ "\\",          SDLK_BACKSLASH },
	{ "]",           SDLK_RIGHTBRACKET },
	{ "`",           SDLK_BACKQUOTE },
	{ "a",           SDLK_a },
	{ "b",           SDLK_b },
	{ "c",           SDLK_c },
	{ "d",           SDLK_d },
	{ "e",           SDLK_e },
	{ "f",           SDLK_f },
	{ "g",           SDLK_g },
	{ "h",           SDLK_h },
	{ "i",           SDLK_i },
	{ "j",           SDLK_j },
	{ "k",           SDLK_k },
	{ "l",           SDLK_l },
	{ "m",           SDLK_m },
	{ "n",           SDLK_n },
	{ "o",           SDLK_o },
	{ "p",           SDLK_p },
	{ "q",           SDLK_q },
	{ "r",           SDLK_r },
	{ "s",           SDLK_s },
	{ "t",           SDLK_t },
	{ "u",           SDLK_u },
	{ "v",           SDLK_v },
	{ "w",           SDLK_w },
	{ "x",           SDLK_x },
	{ "y",           SDLK_y },
	{ "z",           SDLK_z },
	{ "delete",      SDLK_DELETE },
	{ "kp0",         SDLK_KP_0 },
	{ "kp1",         SDLK_KP_1 },
	{ "kp2",         SDLK_KP_2 },
	{ "kp3",         SDLK_KP_3 },
	{ "kp4",         SDLK_KP_4 },
	{ "kp5",         SDLK_KP_5 },
	{ "kp6",         SDLK_KP_6 },
	{ "kp7",         SDLK_KP_7 },
	{ "kp8",         SDLK_KP_8 },
	{ "kp9",         SDLK_KP_9 },
	{ "kp.",         SDLK_KP_PERIOD },
	{ "kp/",         SDLK_KP_DIVIDE },
	{ "kp*",         SDLK_KP_MULTIPLY },
	{ "kp-",         SDLK_KP_MINUS },
	{ "kp+",         SDLK_KP_PLUS },
	{ "kpenter",     SDLK_KP_ENTER },
	{ "up",          SDLK_UP },
	{ "down",        SDLK_DOWN },
	{ "right",       SDLK_RIGHT },
	{ "left",        SDLK_LEFT },
	{ "insert",      SDLK_INSERT },
	{ "home",        SDLK_HOME },
	{ "end",         SDLK_END },
	{ "pageup",      SDLK_PAGEUP },
	{ "pagedown",    SDLK_PAGEDOWN },
	{ "f1",          SDLK_F1 },
	{ "f2",          SDLK_F2 },
	{ "f3",          SDLK_F3 },
	{ "f4",          SDLK_F4 },
	{ "f5",          SDLK_F5 },
	{ "f6",          SDLK_F6 },
	{ "f7",          SDLK_F7 },
	{ "f8",          SDLK_F8 },
	{ "f9",          SDLK_F9 },
	{ "f10",         SDLK_F10 },
	{ "f11",         SDLK_F11 },
	{ "f12",         SDLK_F12 },
	{ "f13",         SDLK_F13 },
	{ "f14",         SDLK_F14 },
	{ "f15",         SDLK_F15 },
	{ "help",        SDLK_HELP },
	{ "printscreen", SDLK_PRINTSCREEN },
	{ 0, 0 }
};

KeySym KeySym::FromString(const std::string &spec)
{
	static const std::string delim("+");

	SDL_Keycode sym = SDLK_UNKNOWN;
	Uint32 mod = KMOD_NONE;

	size_t start = 0, end = 0;
	while (end != std::string::npos) {
		// get to the first non-delim char
		start = spec.find_first_not_of(delim, end);

		// read the end, no more to do
		if (start == std::string::npos)
			break;

		// find the end - next delim or end of string
		end = spec.find_first_of(delim, start);

		// extract the fragment
		const std::string token(spec.substr(start, (end == std::string::npos) ? std::string::npos : end - start));

		if (token == "ctrl")
			mod |= KMOD_CTRL;
		else if (token == "shift")
			mod |= KMOD_SHIFT;
		else if (token == "alt")
			mod |= KMOD_ALT;
		else if (token == "meta")
			mod |= KMOD_GUI;

		else {
			if (sym != SDLK_UNKNOWN)
				Output("key spec '%s' has multiple keys, ignoring '%s'\n", spec.c_str(), token.c_str());
			else {
				for (const KeyMap *map = keymap; map->name; map++)
					if (token == map->name)
						sym = map->sym;
				if (sym == SDLK_UNKNOWN)
					Output("key spec '%s' has unkown token '%s', ignoring it\n", spec.c_str(), token.c_str());
			}
		}
	}

	return KeySym(sym, SDL_Keymod(mod));
}

static void _settable(lua_State *l, const char *key, const Point &value)
{
	lua_pushstring(l, key);

	lua_newtable(l);
	pi_lua_settable(l, "x", value.x);
	pi_lua_settable(l, "y", value.y);

	lua_rawset(l, -3);
}

void KeyboardEvent::ToLuaTable(lua_State *l) const
{
	lua_newtable(l);
	pi_lua_settable(l, "type", EnumStrings::GetString("UIEventType", type));
	pi_lua_settable(l, "action", EnumStrings::GetString("UIKeyboardAction", action));
	pi_lua_settable(l, "repeat", repeat);

	// XXX expose sym and mod constants
}

void TextInputEvent::ToLuaTable(lua_State *l) const
{
	lua_newtable(l);
	pi_lua_settable(l, "type", EnumStrings::GetString("UIEventType", type));

	lua_pushvalue(l, unicode);
	lua_setfield(l, -2, "unicode");
}

void MouseButtonEvent::ToLuaTable(lua_State *l) const
{
	lua_newtable(l);
	pi_lua_settable(l, "type", EnumStrings::GetString("UIEventType", type));

	_settable(l, "pos", pos);

	pi_lua_settable(l, "action", EnumStrings::GetString("UIMouseButtonAction", action));
	pi_lua_settable(l, "button", EnumStrings::GetString("UIMouseButtonType", button));
}

void MouseMotionEvent::ToLuaTable(lua_State *l) const
{
	lua_newtable(l);
	pi_lua_settable(l, "type", EnumStrings::GetString("UIEventType", type));

	_settable(l, "pos", pos);
	_settable(l, "rel", rel);
}

void MouseWheelEvent::ToLuaTable(lua_State *l) const
{
	lua_newtable(l);
	pi_lua_settable(l, "type", EnumStrings::GetString("UIEventType", type));

	_settable(l, "pos", pos);

	pi_lua_settable(l, "direction", EnumStrings::GetString("UIMouseWheelDirection", direction));
}

void JoystickAxisMotionEvent::ToLuaTable(lua_State *l) const
{
	lua_newtable(l);
	pi_lua_settable(l, "type", EnumStrings::GetString("UIEventType", type));
	pi_lua_settable(l, "joystick", joystick);
	pi_lua_settable(l, "value", value);
	pi_lua_settable(l, "axis", axis);
}

void JoystickHatMotionEvent::ToLuaTable(lua_State *l) const
{
	lua_newtable(l);
	pi_lua_settable(l, "type", EnumStrings::GetString("UIEventType", type));
	pi_lua_settable(l, "joystick", joystick);
	pi_lua_settable(l, "direction", EnumStrings::GetString("UIJoystickHatDirection", direction));
	pi_lua_settable(l, "hat", hat);
}

void JoystickButtonEvent::ToLuaTable(lua_State *l) const
{
	lua_newtable(l);
	pi_lua_settable(l, "type", EnumStrings::GetString("UIEventType", type));
	pi_lua_settable(l, "joystick", joystick);
	pi_lua_settable(l, "action", EnumStrings::GetString("UIJoystickButtonAction", action));
	pi_lua_settable(l, "button", button);
}

}
