// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaEngine.h"
#include "LuaObject.h"
#include "LuaUtils.h"
#include "LuaConstants.h"
#include "EnumStrings.h"
#include "Random.h"
#include "Pi.h"
#include "utils.h"
#include "FloatComparison.h"
#include "FileSystem.h"
#include "ui/Context.h"
#include "graphics/Graphics.h"
#include "Sound.h"
#include "SoundMusic.h"
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

static int l_engine_get_video_mode_list(lua_State *l)
{
	LUA_DEBUG_START(l);

	const std::vector<Graphics::VideoMode> modes = Graphics::GetAvailableVideoModes();
	const int N = modes.size();
	lua_createtable(l, N, 0);
	for (int i = 0; i < N; ++i) {
		lua_createtable(l, 0, 2);
		lua_pushinteger(l, modes[i].width);
		lua_setfield(l, -2, "width");
		lua_pushinteger(l, modes[i].height);
		lua_setfield(l, -2, "height");

		lua_rawseti(l, -2, i+1);
	}

	LUA_DEBUG_END(l, 1);
	return 1;
}

static int l_engine_get_video_resolution(lua_State *l)
{
	lua_pushinteger(l, Graphics::GetScreenWidth());
	lua_pushinteger(l, Graphics::GetScreenHeight());
	return 2;
}

static int l_engine_set_video_resolution(lua_State *l)
{
	const int width = luaL_checkinteger(l, 1);
	const int height = luaL_checkinteger(l, 2);
	Pi::config->SetInt("ScrWidth", width);
	Pi::config->SetInt("ScrHeight", height);
	Pi::config->Save();
	return 0;
}

static int l_engine_get_fullscreen(lua_State *l)
{
	lua_pushboolean(l, Pi::config->Int("StartFullscreen") != 0);
	return 1;
}

static int l_engine_set_fullscreen(lua_State *l)
{
	if (lua_isnone(l, 1))
		return luaL_error(l, "SetFullscreen takes one boolean argument");
	const bool fullscreen = lua_toboolean(l, 1);
	Pi::config->SetInt("StartFullscreen", (fullscreen ? 1 : 0));
	Pi::config->Save();
	return 0;
}

static int l_engine_get_vsync_enabled(lua_State *l)
{
	lua_pushboolean(l, Pi::config->Int("VSync") != 0);
	return 1;
}

static int l_engine_set_vsync_enabled(lua_State *l)
{
	if (lua_isnone(l, 1))
		return luaL_error(l, "SetVSyncEnabled takes one boolean argument");
	const bool vsync = lua_toboolean(l, 1);
	Pi::config->SetInt("VSync", (vsync ? 1 : 0));
	Pi::config->Save();
	return 0;
}

static int l_engine_get_shaders_enabled(lua_State *l)
{
	lua_pushboolean(l, Pi::config->Int("DisableShaders") == 0);
	return 1;
}

static int l_engine_set_shaders_enabled(lua_State *l)
{
	if (lua_isnone(l, 1))
		return luaL_error(l, "SetShadersEnabled takes one boolean argument");
	const bool enabled = lua_toboolean(l, 1);
	Pi::config->SetInt("DisableShaders", (enabled ? 0 : 1));
	Pi::config->Save();
	return 0;
}

static int l_engine_get_texture_compression_enabled(lua_State *l)
{
	lua_pushboolean(l, Pi::config->Int("UseTextureCompression") != 0);
	return 1;
}

static int l_engine_set_texture_compression_enabled(lua_State *l)
{
	if (lua_isnone(l, 1))
		return luaL_error(l, "SetTextureCompressionEnabled takes one boolean argument");
	const bool enabled = lua_toboolean(l, 1);
	Pi::config->SetInt("UseTextureCompression", (enabled ? 1 : 0));
	Pi::config->Save();
	return 0;
}

static int l_engine_get_multisampling(lua_State *l)
{
	lua_pushinteger(l, Pi::config->Int("AntiAliasingMode"));
	return 1;
}

static int l_engine_set_multisampling(lua_State *l)
{
	const int samples = luaL_checkinteger(l, 1);
	Pi::config->SetInt("AntiAliasingMode", samples);
	Pi::config->Save();
	return 0;
}

static int l_engine_get_planet_detail_level(lua_State *l)
{
	lua_pushstring(l, EnumStrings::GetString("DetailLevel", Pi::detail.planets));
	return 1;
}

static int l_engine_set_planet_detail_level(lua_State *l)
{
	const int level = LuaConstants::GetConstantFromArg(l, "DetailLevel", 1);
	if (level != Pi::detail.planets) {
		Pi::detail.planets = level;
		Pi::config->SetInt("DetailPlanets", level);
		Pi::config->Save();
		Pi::OnChangeDetailLevel();
	}
	return 0;
}

static int l_engine_get_city_detail_level(lua_State *l)
{
	lua_pushstring(l, EnumStrings::GetString("DetailLevel", Pi::detail.cities));
	return 1;
}

static int l_engine_set_city_detail_level(lua_State *l)
{
	const int level = LuaConstants::GetConstantFromArg(l, "DetailLevel", 1);
	if (level != Pi::detail.cities) {
		Pi::detail.cities = level;
		Pi::config->SetInt("DetailCities", level);
		Pi::config->Save();
		Pi::OnChangeDetailLevel();
	}
	return 0;
}

static int l_engine_get_fractal_detail_level(lua_State *l)
{
	lua_pushstring(l, EnumStrings::GetString("DetailLevel", Pi::detail.fracmult));
	return 1;
}

static int l_engine_set_fractal_detail_level(lua_State *l)
{
	const int level = LuaConstants::GetConstantFromArg(l, "DetailLevel", 1);
	if (level != Pi::detail.fracmult) {
		Pi::detail.cities = level;
		Pi::config->SetInt("FractalMultiple", level);
		Pi::config->Save();
		Pi::OnChangeDetailLevel();
	}
	return 0;
}

static int l_engine_get_planet_fractal_colour_enabled(lua_State *l)
{
	lua_pushboolean(l, Pi::detail.textures);
	return 1;
}

static int l_engine_set_planet_fractal_colour_enabled(lua_State *l)
{
	if (lua_isnone(l, 1))
		return luaL_error(l, "SetPlanetFractalColourEnabled takes one boolean argument");
	const int level = (lua_toboolean(l, 1) ? 1 : 0);
	if (level != Pi::detail.textures) {
		Pi::detail.textures = level;
		Pi::config->SetInt("Textures", level);
		Pi::config->Save();
		Pi::OnChangeDetailLevel();
	}
	return 0;
}

static void set_master_volume(const bool muted, const float volume)
{
	Sound::Pause(muted || is_zero_exact(volume));
	Sound::SetMasterVolume(volume);
	Pi::config->SetFloat("MasterVolume", volume);
	Pi::config->SetInt("MasterMuted", muted ? 1 : 0);
	Pi::config->Save();
}

static void set_effects_volume(const bool muted, const float volume)
{
	Sound::SetSfxVolume(muted ? 0.0f : volume);
	Pi::config->SetFloat("SfxVolume", volume);
	Pi::config->SetInt("SfxMuted", muted ? 1 : 0);
	Pi::config->Save();
}

static void set_music_volume(const bool muted, const float volume)
{
	Pi::GetMusicPlayer().SetEnabled(!(muted || is_zero_exact(volume)));
	Pi::GetMusicPlayer().SetVolume(volume);
	Pi::config->SetFloat("MusicVolume", volume);
	Pi::config->SetInt("MusicMuted", muted ? 1 : 0);
	Pi::config->Save();
}

static int l_engine_get_master_muted(lua_State *l)
{
	lua_pushboolean(l, Pi::config->Int("MasterMuted") != 0);
	return 1;
}

static int l_engine_set_master_muted(lua_State *l)
{
	if (lua_isnone(l, 1))
		return luaL_error(l, "SetMasterMuted takes one boolean argument");
	const bool muted = lua_toboolean(l, 1);
	set_master_volume(muted, Sound::GetMasterVolume());
	return 0;
}

static int l_engine_get_master_volume(lua_State *l)
{
	lua_pushnumber(l, Pi::config->Float("MasterVolume"));
	return 1;
}

static int l_engine_set_master_volume(lua_State *l)
{
	const float volume = Clamp(luaL_checknumber(l, 1), 0.0, 1.0);
	set_master_volume(Pi::config->Int("MasterMuted") != 0, volume);
	return 0;
}

static int l_engine_get_effects_muted(lua_State *l)
{
	lua_pushboolean(l, Pi::config->Int("SfxMuted") != 0);
	return 1;
}

static int l_engine_set_effects_muted(lua_State *l)
{
	if (lua_isnone(l, 1))
		return luaL_error(l, "SetEffectsMuted takes one boolean argument");
	const bool muted = lua_toboolean(l, 1);
	set_effects_volume(muted, Sound::GetSfxVolume());
	return 0;
}

static int l_engine_get_effects_volume(lua_State *l)
{
	lua_pushnumber(l, Pi::config->Float("SfxVolume"));
	return 1;
}

static int l_engine_set_effects_volume(lua_State *l)
{
	const float volume = Clamp(luaL_checknumber(l, 1), 0.0, 1.0);
	set_effects_volume(Pi::config->Int("SfxMuted") != 0, volume);
	return 0;
}

static int l_engine_get_music_muted(lua_State *l)
{
	lua_pushboolean(l, Pi::config->Int("MusicMuted") != 0);
	return 1;
}

static int l_engine_set_music_muted(lua_State *l)
{
	if (lua_isnone(l, 1))
		return luaL_error(l, "SetMusicMuted takes one boolean argument");
	const bool muted = lua_toboolean(l, 1);
	set_music_volume(muted, Pi::GetMusicPlayer().GetVolume());
	return 0;
}

static int l_engine_get_music_volume(lua_State *l)
{
	lua_pushnumber(l, Pi::config->Float("MusicVolume"));
	return 1;
}

static int l_engine_set_music_volume(lua_State *l)
{
	const float volume = Clamp(luaL_checknumber(l, 1), 0.0, 1.0);
	set_music_volume(Pi::config->Int("MusicMuted") != 0, volume);
	return 0;
}

void LuaEngine::Register()
{
	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);

	static const luaL_Reg l_methods[] = {
		{ "Quit", l_engine_quit },
		{ "SettingsView", l_engine_settings_view },

		{ "GetVideoModeList", l_engine_get_video_mode_list },
		{ "GetVideoResolution", l_engine_get_video_resolution },
		{ "SetVideoResolution", l_engine_set_video_resolution },
		{ "GetFullscreen", l_engine_get_fullscreen },
		{ "SetFullscreen", l_engine_set_fullscreen },
		{ "GetVSyncEnabled", l_engine_get_vsync_enabled },
		{ "SetVSyncEnabled", l_engine_set_vsync_enabled },
		{ "GetShadersEnabled", l_engine_get_shaders_enabled },
		{ "SetShadersEnabled", l_engine_set_shaders_enabled },
		{ "GetTextureCompressionEnabled", l_engine_get_texture_compression_enabled },
		{ "SetTextureCompressionEnabled", l_engine_set_texture_compression_enabled },
		{ "GetMultisampling", l_engine_get_multisampling },
		{ "SetMultisampling", l_engine_set_multisampling },

		{ "GetPlanetDetailLevel", l_engine_get_planet_detail_level },
		{ "SetPlanetDetailLevel", l_engine_set_planet_detail_level },
		{ "GetCityDetailLevel", l_engine_get_city_detail_level },
		{ "SetCityDetailLevel", l_engine_set_city_detail_level },
		{ "GetFractalDetailLevel", l_engine_get_fractal_detail_level },
		{ "SetFractalDetailLevel", l_engine_set_fractal_detail_level },
		{ "GetPlanetFractalColourEnabled", l_engine_get_planet_fractal_colour_enabled },
		{ "SetPlanetFractalColourEnabled", l_engine_set_planet_fractal_colour_enabled },

		{ "GetMasterMuted", l_engine_get_master_muted },
		{ "SetMasterMuted", l_engine_set_master_muted },
		{ "GetMasterVolume", l_engine_get_master_volume },
		{ "SetMasterVolume", l_engine_set_master_volume },
		{ "GetEffectsMuted", l_engine_get_effects_muted },
		{ "SetEffectsMuted", l_engine_set_effects_muted },
		{ "GetEffectsVolume", l_engine_get_effects_volume },
		{ "SetEffectsVolume", l_engine_set_effects_volume },
		{ "GetMusicMuted", l_engine_get_music_muted },
		{ "SetMusicMuted", l_engine_set_music_muted },
		{ "GetMusicVolume", l_engine_get_music_volume },
		{ "SetMusicVolume", l_engine_set_music_volume },

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
