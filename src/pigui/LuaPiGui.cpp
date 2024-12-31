// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaPiGui.h"
#include "Face.h"
#include "Image.h"
#include "ModelSpinner.h"
#include "Radar.h"
#include "lua/LuaPiGuiInternal.h"
#include "lua/LuaTable.h"

#include <profiler/Profiler.h>

static LuaRef m_handlers;
static LuaRef m_themes;
static LuaRef m_eventQueue;

namespace PiGui {

	namespace Lua {

		void Init()
		{
			LuaObject<PiGui::Instance>::RegisterClass();
			lua_State *l = ::Lua::manager->GetLuaState();

			LUA_DEBUG_START(l);

			lua_newtable(l);
			m_handlers = LuaRef(l, -1);
			lua_pop(l, 1);

			lua_newtable(l);
			m_themes = LuaRef(l, -1);
			lua_pop(l, 1);

			pi_lua_import(l, "Event");

			// Create a new event table and store it for UI use
			m_eventQueue = LuaTable(l, -1).Call<LuaRef>("New");
			lua_pop(l, 1);

			LuaObject<PiGui::Image>::RegisterClass();
			LuaObject<PiGui::Face>::RegisterClass();
			LuaObject<PiGui::ModelSpinner>::RegisterClass();
			LuaObject<PiGui::RadarWidget>::RegisterClass();
			RegisterSandbox();

			LUA_DEBUG_END(l, 0);
		}

		void Uninit()
		{
			m_handlers.Unref();
			m_themes.Unref();
			m_eventQueue.Unref();
		}

	} // namespace Lua

	LuaRef GetHandlers() { return m_handlers; }
	LuaRef GetThemes() { return m_themes; }
	LuaRef GetEventQueue() { return m_eventQueue; }

	void EmitEvents()
	{
		PROFILE_SCOPED()

		ScopedTable queue(GetEventQueue());
		queue.Call("_Emit");
	}

	void RunHandler(double delta, const std::string &handler)
	{
		PROFILE_SCOPED()
		ScopedTable t(GetHandlers());
		if (t.Get<bool>(handler)) {
			t.Call<bool>(handler, delta);
			Pi::renderer->CheckRenderErrors(__FUNCTION__, __LINE__);
		}
	}

	void LoadTheme(ImGuiStyle &style, const std::string &theme)
	{
		PROFILE_SCOPED();
		ScopedTable t(GetThemes());
		if (t.Get<bool>(theme)) {
			ScopedTable theme_tab = t.Sub(theme);
			load_theme_from_table(theme_tab, style);
		} else {
			Output("Unable to load theme %s from lua!\n", theme.c_str());
		}
	}

	void LoadThemeFromDisk(const std::string &theme)
	{
		PROFILE_SCOPED();
		GetThemes().PushCopyToStack();

		pi_lua_import(GetThemes().GetLua(), "pigui.themes." + theme);
		lua_setfield(GetThemes().GetLua(), -2, theme.c_str());

		lua_pop(GetThemes().GetLua(), 1);
	}
} // namespace PiGui
