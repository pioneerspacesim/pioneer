// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "PiGuiLua.h"
#include "Face.h"
#include "Image.h"
#include "ModelSpinner.h"
#include "lua/LuaTable.h"

static std::vector<std::pair<std::string, int>> m_keycodes = {
	{ "left", SDLK_LEFT },
	{ "right", SDLK_RIGHT },
	{ "up", SDLK_UP },
	{ "down", SDLK_DOWN },
	{ "escape", SDLK_ESCAPE },
	{ "f1", SDLK_F1 },
	{ "f2", SDLK_F2 },
	{ "f3", SDLK_F3 },
	{ "f4", SDLK_F4 },
	{ "f5", SDLK_F5 },
	{ "f6", SDLK_F6 },
	{ "f7", SDLK_F7 },
	{ "f8", SDLK_F8 },
	{ "f9", SDLK_F9 },
	{ "f10", SDLK_F10 },
	{ "f11", SDLK_F11 },
	{ "f12", SDLK_F12 },
	{ "tab", SDLK_TAB },
};

static LuaRef m_handlers;
static LuaRef m_keys;

namespace PiGUI {

	namespace Lua {

		void Init()
		{
			LuaObject<PiGui::Instance>::RegisterClass();

			lua_State *l = ::Lua::manager->GetLuaState();
			lua_newtable(l);
			m_handlers = LuaRef(l, -1);

			lua_newtable(l);
			m_keys = LuaRef(l, -1);
			LuaTable keys(l, -1);
			for (auto p : m_keycodes) {
				keys.Set(p.first, p.second);
			}

			LuaObject<PiGUI::Image>::RegisterClass();
			LuaObject<PiGUI::Face>::RegisterClass();
			LuaObject<PiGUI::ModelSpinner>::RegisterClass();
			RegisterSandbox();
		}

		void Uninit()
		{
			m_handlers.Unref();
			m_keys.Unref();
		}

	} // namespace Lua

	LuaRef GetHandlers() { return m_handlers; }
	LuaRef GetKeys() { return m_keys; }
} // namespace PiGUI
