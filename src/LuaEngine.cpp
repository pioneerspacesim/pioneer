// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaEngine.h"
#include "LuaObject.h"
#include "LuaUtils.h"
#include "Random.h"
#include "Pi.h"
#include "utils.h"
#include "FileSystem.h"
#include "ui/Context.h"
#include "GameMenuView.h"

/*
 * Interface: Engine
 *
 * A global table that exposes a number of non-game-specific values from the
 * game engine.
 *
 */

/*
 * Attribute: rand
 *
 * The global <Rand> object. Its stream of values will be different across
 * multiple Pioneer runs. Use this when you just need a random number and
 * don't care about the seed.
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  stable
 */
static int l_engine_attr_rand(lua_State *l)
{
	LuaObject<Random>::PushToLua(&Pi::rng);
	return 1;
}

/*
 * Attribute: ticks
 *
 * Number of milliseconds since Pioneer was started. This should be used for
 * debugging purposes only (eg timing) and should never be used for game logic
 * of any kind.
 *
 * Availability:
 *
 *   alpha 26
 *
 * Status:
 *
 *   debug
 */
static int l_engine_attr_ticks(lua_State *l)
{
	lua_pushinteger(l, SDL_GetTicks());
	return 1;
}

/*
 * Attribute: ui
 *
 * The global <UI.Context> object. New UI widgets are created through this
 * object.
 *
 * Availability:
 *
 *   alpha 25
 *
 * Status:
 *
 *   experimental
 */
static int l_engine_attr_ui(lua_State *l)
{
	LuaObject<UI::Context>::PushToLua(Pi::ui.Get());
	return 1;
}

/*
 * Attribute: version
 *
 * String describing the version of Pioneer
 *
 * Availability:
 *
 *   alpha 25
 *
 * Status:
 *
 *   experimental
 */
static int l_engine_attr_version(lua_State *l)
{
	std::string version(PIONEER_VERSION);
	if (strlen(PIONEER_EXTRAVERSION)) version += " (" PIONEER_EXTRAVERSION ")";
    lua_pushlstring(l, version.c_str(), version.size());
    return 1;
}

/*
 * Function: Quit
 *
 * Exit the program. If there is a game running it ends the game first.
 *
 * > Engine.Quit()
 *
 * Availability:
 *
 *   alpha 28
 *
 * Status:
 *
 *   experimental
 */
static int l_engine_quit(lua_State *l)
{
	if (Pi::game)
		Pi::EndGame();
	Pi::Quit();
	return 0;
}

// XXX hack to allow the new UI to activate the old settings view
//     remove once its been converted
static int l_engine_settings_view(lua_State *l)
{
	if (Pi::game || Pi::GetView() == Pi::gameMenuView)
		return 0;
	Pi::SetView(Pi::gameMenuView);
	while (Pi::GetView() == Pi::gameMenuView) Gui::MainLoopIteration();
	Pi::SetView(0);
	return 0;
}

void LuaEngine::Register()
{
	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);

	static const luaL_Reg l_methods[] = {
		{ "Quit", l_engine_quit },
		{ "SettingsView", l_engine_settings_view },
		{ 0, 0 }
	};

	static const luaL_Reg l_attrs[] = {
		{ "rand",    l_engine_attr_rand    },
		{ "ticks",   l_engine_attr_ticks   },
		{ "ui",      l_engine_attr_ui      },
		{ "version", l_engine_attr_version },
		{ 0, 0 }
	};

	lua_getfield(l, LUA_REGISTRYINDEX, "CoreImports");
	LuaObjectBase::CreateObject(l_methods, l_attrs, 0);
	lua_setfield(l, -2, "Engine");
	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);
}
