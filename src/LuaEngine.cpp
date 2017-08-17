// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaEngine.h"
#include "LuaObject.h"
#include "LuaUtils.h"
#include "LuaConstants.h"
#include "EnumStrings.h"
#include "Random.h"
#include "OS.h"
#include "Pi.h"
#include "PiGui.h"
#include "utils.h"
#include "FloatComparison.h"
#include "FileSystem.h"
#include "ui/Context.h"
#include "graphics/Graphics.h"
#include "Sound.h"
#include "SoundMusic.h"
#include "KeyBindings.h"
#include "Lang.h"
#include "Player.h"
#include "Game.h"
#include "scenegraph/Model.h"
#include "LuaPiGui.h"

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
 * Attribute: pigui
 *
 * The global PiGui object. It provides an interface to ImGui functions
 *
 * Availability:
 *
 *   2016-10-06
 *
 * Status:
 *
 *   experimental
 */
static int l_engine_attr_pigui(lua_State *l)
{
	LuaObject<PiGui>::PushToLua(Pi::pigui.Get());
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

/*
 * Method: GetVideoModeList
 *
 * Get the available video modes
 *
 * > Engine.GetVideoModeList()
 *
 * Availability:
 *
 *   2017-04
 *
 * Status:
 *
 *   stable
 */

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

/*
 * Method: GetVideoResolution
 *
 * Get the current video resolution width and height
 *
 * > width,height = Engine.GetVideoResolution()
 *
 * Availability:
 *
 *   2017-04
 *
 * Status:
 *
 *   stable
 */

static int l_engine_get_video_resolution(lua_State *l)
{
	lua_pushinteger(l, Graphics::GetScreenWidth());
	lua_pushinteger(l, Graphics::GetScreenHeight());
	return 2;
}

/*
 * Method: SetVideoResolution
 *
 * Set the current video resolution width and height
 *
 * > Engine.SetVideoResolution(width, height)
 *
 * Parameters:
 *
 *   width - the new width in pixels
 *   height - the new height in pixels
 *
 * Availability:
 *
 *   2017-04
 *
 * Status:
 *
 *   stable
 */

static int l_engine_set_video_resolution(lua_State *l)
{
	const int width = luaL_checkinteger(l, 1);
	const int height = luaL_checkinteger(l, 2);
	Pi::config->SetInt("ScrWidth", width);
	Pi::config->SetInt("ScrHeight", height);
	Pi::config->Save();
	return 0;
}

/*
 * Method: GetFullscreen
 *
 * Return true if fullscreen is enabled
 *
 * > fullscreen = Engine.GetFullscreen()
 *
 * Availability:
 *
 *   2017-04
 *
 * Status:
 *
 *   stable
 */

static int l_engine_get_fullscreen(lua_State *l)
{
	lua_pushboolean(l, Pi::config->Int("StartFullscreen") != 0);
	return 1;
}

/*
 * Method: SetFullscreen
 *
 * Turn fullscreen on or off
 *
 * > Engine.SetFullscreen(true)
 *
 * Parameters:
 *
 *   fullscreen - true to turn on
 *
 * Availability:
 *
 *   2017-04
 *
 * Status:
 *
 *   stable
 */

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
		Pi::detail.fracmult = level;
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

static int l_engine_get_display_nav_tunnels(lua_State *l)
{
	lua_pushboolean(l, Pi::config->Int("DisplayNavTunnel") != 0);
	return 1;
}

static int l_engine_set_display_nav_tunnels(lua_State *l)
{
	if (lua_isnone(l, 1))
		return luaL_error(l, "SetDisplayNavTunnels takes one boolean argument");
	const bool enabled = lua_toboolean(l, 1);
	Pi::config->SetInt("DisplayNavTunnel", (enabled ? 1 : 0));
	Pi::config->Save();
	Pi::SetNavTunnelDisplayed(enabled);
	return 0;
}

static int l_engine_get_display_speed_lines(lua_State *l)
{
	lua_pushboolean(l, Pi::config->Int("SpeedLines") != 0);
	return 1;
}

static int l_engine_set_display_speed_lines(lua_State *l)
{
	if (lua_isnone(l, 1))
		return luaL_error(l, "SetDisplaySpeedLines takes one boolean argument");
	const bool enabled = lua_toboolean(l, 1);
	Pi::config->SetInt("SpeedLines", (enabled ? 1 : 0));
	Pi::config->Save();
	Pi::SetSpeedLinesDisplayed(enabled);
	return 0;
}

static int l_engine_get_cockpit_enabled(lua_State *l)
{
	lua_pushboolean(l, Pi::config->Int("EnableCockpit") != 0);
	return 1;
}

static int l_engine_set_cockpit_enabled(lua_State *l)
{
	if (lua_isnone(l, 1))
		return luaL_error(l, "SetCockpitEnabled takes one boolean argument");
	const bool enabled = lua_toboolean(l, 1);
	Pi::config->SetInt("EnableCockpit", (enabled ? 1 : 0));
	Pi::config->Save();
	if (Pi::player) {
		Pi::player->InitCockpit();
		if (enabled) Pi::player->OnCockpitActivated();
	}
	return 0;
}

static int l_engine_get_aniso_enabled(lua_State *l)
{
	lua_pushboolean(l, Pi::config->Int("UseAnisotropicFiltering") != 0);
	return 1;
}

static int l_engine_set_aniso_enabled(lua_State *l)
{
	if (lua_isnone(l, 1))
		return luaL_error(l, "SetAnisoEnabled takes one boolean argument");
	const bool enabled = lua_toboolean(l, 1);
	Pi::config->SetInt("UseAnisotropicFiltering", (enabled ? 1 : 0));
	Pi::config->Save();
	return 0;
}

static int l_engine_get_autosave_enabled(lua_State *l)
{
	lua_pushboolean(l, Pi::config->Int("EnableAutosave") != 0);
	return 1;
}

static int l_engine_set_autosave_enabled(lua_State *l)
{
	if (lua_isnone(l, 1))
		return luaL_error(l, "SetAutopilotEnabled takes one boolean argument");
	const bool enabled = lua_toboolean(l, 1);
	Pi::config->SetInt("EnableAutosave", (enabled ? 1 : 0));
	Pi::config->Save();
	return 0;
}

static int l_engine_get_display_hud_trails(lua_State *l)
{
	lua_pushboolean(l, Pi::config->Int("HudTrails") != 0);
	return 1;
}

static int l_engine_set_display_hud_trails(lua_State *l)
{
	if (lua_isnone(l, 1))
		return luaL_error(l, "SetDisplayHudTrails takes one boolean argument");
	const bool enabled = lua_toboolean(l, 1);
	Pi::config->SetInt("HudTrails", (enabled ? 1 : 0));
	Pi::config->Save();
	Pi::SetHudTrailsDisplayed(enabled);
	return 0;
}

static int l_engine_set_amount_stars(lua_State *l)
{
	const float amount = Clamp(luaL_checknumber(l, 1), 0.01, 1.0);
	Pi::config->SetFloat("AmountOfBackgroundStars", amount);
	Pi::config->Save();
	Pi::SetAmountBackgroundStars(amount);
	return 0;
}

static int l_engine_get_amount_stars(lua_State *l)
{
	lua_pushnumber(l, Pi::config->Float("AmountOfBackgroundStars"));
	return 1;
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

static int l_engine_get_gpu_jobs_enabled(lua_State *l)
{
	lua_pushboolean(l, Pi::config->Int("EnableGPUJobs") != 0);
	return 1;
}

static int l_engine_set_gpu_jobs_enabled(lua_State *l)
{
	if (lua_isnone(l, 1))
		return luaL_error(l, "SetGpuJobsEnabled takes one boolean argument");
	const bool enabled = lua_toboolean(l, 1);
	Pi::config->SetInt("EnableGPUJobs", (enabled ? 1 : 0));
	Pi::config->Save();
	return 0;
}

static void push_bindings(lua_State *l, const KeyBindings::BindingPrototype *protos) {
	LUA_DEBUG_START(l);

	lua_newtable(l); // [-1] bindings
	lua_pushnil(l); // [-2] bindings, [-1] group (no current group)

	assert(!protos[0].function); // first entry should be a group header

	int group_idx = 1;
	int binding_idx = 1;
	for (const KeyBindings::BindingPrototype *proto = protos; proto->label; ++proto) {
		if (! proto->function) {
			// start a new named binding group

			// [-2] bindings, [-1] group
			lua_pop(l, 1);
			// [-1] bindings
			lua_newtable(l);
			lua_pushstring(l, proto->label);
			lua_setfield(l, -2, "label");
			// [-2] bindings, [-1] group
			lua_pushvalue(l, -1);
			// [-3] bindings, [-2] group, [-1] group copy
			lua_rawseti(l, -3, group_idx);
			++group_idx;

			binding_idx = 1;
		} else {
			// key or axis binding prototype

			// [-2] bindings, [-1] group
			lua_createtable(l, 0, 5);
			// [-3] bindings, [-2] group, [-1] binding

			// fields are: type ('KEY' or 'AXIS'), id ('BindIncreaseSpeed'), label ('Increase Speed'), binding ('Key13'), bindingDescription ('')
			lua_pushstring(l, (proto->kb ? "KEY" : "AXIS"));
			lua_setfield(l, -2, "type");
			lua_pushstring(l, proto->function);
			lua_setfield(l, -2, "id");
			lua_pushstring(l, proto->label);
			lua_setfield(l, -2, "label");
			if (proto->kb) {
				const KeyBindings::KeyBinding kb1 = proto->kb->binding1;
				if (kb1.Enabled()) {
					lua_pushstring(l, kb1.ToString().c_str());
					lua_setfield(l, -2, "binding1");
					lua_pushstring(l, kb1.Description().c_str());
					lua_setfield(l, -2, "bindingDescription1");
				}
				const KeyBindings::KeyBinding kb2 = proto->kb->binding2;
				if (kb2.Enabled()) {
					lua_pushstring(l, kb2.ToString().c_str());
					lua_setfield(l, -2, "binding2");
					lua_pushstring(l, kb2.Description().c_str());
					lua_setfield(l, -2, "bindingDescription2");
				}
			} else if (proto->ab) {
				const KeyBindings::AxisBinding &ab = *proto->ab;
				lua_pushstring(l, ab.ToString().c_str());
				lua_setfield(l, -2, "binding1");
				lua_pushstring(l, ab.Description().c_str());
				lua_setfield(l, -2, "bindingDescription1");
			} else {
				assert(0); // invalid prototype binding
			}

			// [-3] bindings, [-2] group, [-1] binding
			lua_rawseti(l, -2, binding_idx);
			++binding_idx;
		}

		LUA_DEBUG_CHECK(l, 2); // [-2] bindings, [-1] group
	}

	// pop the group table (which should already have been put in the bindings table)
	lua_pop(l, 1);

	LUA_DEBUG_END(l, 1);
}

/*
 * Function: GetKeyBindings
 *
 * Get a table listing all the current key and axis bindings.
 *
 * > bindings = Engine.GetKeyBindings()
 *
 * Returns:
 *
 *   bindings - A table containing all the key and axis bindings.
 *
 * The bindings table has the following structure (in Lua syntax):
 *
 * > bindings = {
 * >   { -- a page
 * >      label = 'CONTROLS', -- the (translated) name of the page
 * >      { -- a group
 * >          label = 'Miscellaneous', -- the (translated) name of the group
 * >          { -- a binding
 * >              type = 'KEY', -- the type of binding; can be 'KEY' or 'AXIS'
 * >              id = 'BindToggleLuaConsole', -- the internal ID of the binding; pass this to Engine.SetKeyBinding
 * >              label = 'Toggle Lua console', -- the (translated) label for the binding
 * >              binding1 = 'Key96', -- the first bound key or axis (value stored in config file)
 * >              bindingDescription1 = '`', -- display text for the first bound key or axis
 * >              binding2 = 'Key96', -- the second bound key or axis (value stored in config file)
 * >              bindingDescription2 = '`', -- display text for the second bound key or axis
 * >          },
 * >          -- ... more bindings
 * >      },
 * >      -- ... more groups
 * >   },
 * >   -- ... more pages
 * > }
 *
 * Availability:
 *
 *   October 2013
 *
 * Status:
 *
 *   temporary
 */
static int l_engine_get_key_bindings(lua_State *l)
{
	// XXX maybe this key-bindings table should be cached in the Lua registry?

	int idx = 1;
	lua_newtable(l);

#define BINDING_PAGE(name) \
	push_bindings(l, KeyBindings :: BINDING_PROTOS_ ## name); \
	lua_pushstring(l, Lang :: name); \
	lua_setfield(l, -2, "label"); \
	lua_rawseti(l, -2, idx++);
#include "KeyBindings.inc.h"

	return 1;
}

static int set_key_binding(lua_State *l, const char *config_id, KeyBindings::KeyAction *action) {
	const char *binding_config_1 = lua_tostring(l, 2);
	const char *binding_config_2 = lua_tostring(l, 3);
	KeyBindings::KeyBinding kb1, kb2;
	if (binding_config_1) {
		if (!KeyBindings::KeyBinding::FromString(binding_config_1, kb1))
			return luaL_error(l, "invalid first key binding given to Engine.SetKeyBinding");
	} else
		kb1.Clear();
	if (binding_config_2) {
		if (!KeyBindings::KeyBinding::FromString(binding_config_2, kb2))
			return luaL_error(l, "invalid second key binding given to Engine.SetKeyBinding");
	} else
		kb2.Clear();
	action->binding1 = kb1;
	action->binding2 = kb2;
	Pi::config->SetString(config_id, action->ToString());
	Pi::config->Save();
	return 0;
}

static int set_axis_binding(lua_State *l, const char *config_id, KeyBindings::AxisBinding *binding) {
	const char *binding_config = lua_tostring(l, 2);
	KeyBindings::AxisBinding ab;
	if (binding_config) {
		if (!KeyBindings::AxisBinding::FromString(binding_config, ab))
			return luaL_error(l, "invalid axis binding given to Engine.SetKeyBinding");
	} else
		ab.Clear();
	*binding = ab;
	Pi::config->SetString(config_id, ab.ToString());
	Pi::config->Save();
	return 0;
}

static int l_engine_set_key_binding(lua_State *l)
{
	const char *binding_id = luaL_checkstring(l, 1);

#define KEY_BINDING(action, config_id, label, def1, def2) \
	if (strcmp(binding_id, config_id) == 0) { return set_key_binding(l, config_id, &KeyBindings :: action); }
#define AXIS_BINDING(action, config_id, label, default_axis) \
	if (strcmp(binding_id, config_id) == 0) { return set_axis_binding(l, config_id, &KeyBindings :: action); }

#include "KeyBindings.inc.h"

	return luaL_error(l, "Invalid binding ID given to Engine.SetKeyBinding");
}

static int l_engine_get_mouse_y_inverted(lua_State *l)
{
	lua_pushboolean(l, Pi::config->Int("InvertMouseY") != 0);
	return 1;
}

static int l_engine_set_mouse_y_inverted(lua_State *l)
{
	if (lua_isnone(l, 1))
		return luaL_error(l, "SetMouseYInverted takes one boolean argument");
	const bool inverted = lua_toboolean(l, 1);
	Pi::config->SetInt("InvertMouseY", (inverted ? 1 : 0));
	Pi::config->Save();
	Pi::SetMouseYInvert(inverted);
	return 0;
}

/*
 * Method: ShipSpaceToScreenSpace
 *
 * Convert a Vector from ship space to screen space
 *
 * > screen_space = Engine.ShipSpaceToScreenSpace(ship_space)
 *
 * Parameters:
 *
 *   ship_space - a Vector in ship space
 *
 * Availability:
 *
 *   2017-04
 *
 * Status:
 *
 *   stable
 */

static int l_engine_ship_space_to_screen_space(lua_State *l)
{
	vector3d pos = LuaPull<vector3d>(l, 1);
	vector3d cam = Pi::game->GetWorldView()->ShipSpaceToScreenSpace(pos);
	LuaPush(l, cam);
	return 1;
}

/*
 * Method: CameraSpaceToScreenSpace
 *
 * Convert a Vector from camera space to screen space
 *
 * > screen_space = Engine.CameraSpaceToScreenSpace(camera_space)
 *
 * Parameters:
 *
 *   camera_space - a Vector in camera space
 *
 * Availability:
 *
 *   2017-04
 *
 * Status:
 *
 *   stable
 */

static int l_engine_camera_space_to_screen_space(lua_State *l)
{
	vector3d pos = LuaPull<vector3d>(l, 1);
	vector3d cam = Pi::game->GetWorldView()->CameraSpaceToScreenSpace(pos);
	LuaPush(l, cam);
	return 1;
}

/*
 * Method: WorldSpaceToScreenSpace
 *
 * Convert a Vector from world space to screen space
 *
 * > screen_space = Engine.WorldSpaceToScreenSpace(world_space)
 *
 * Parameters:
 *
 *   world_space - a Vector in world space
 *
 * Availability:
 *
 *   2017-04
 *
 * Status:
 *
 *   stable
 */

static int l_engine_world_space_to_screen_space(lua_State *l)
{
	vector3d pos = LuaPull<vector3d>(l, 1);

	std::tuple<bool, vector3d, vector3d> res = lua_world_space_to_screen_space(pos); // defined in LuaPiGui.cpp
	
	LuaPush<bool>(l, std::get<0>(res));
	LuaPush<vector3d>(l, std::get<1>(res));
	LuaPush<vector3d>(l, std::get<2>(res));
	return 3;
}

static int l_engine_world_space_to_ship_space(lua_State *l)
{
	vector3d vec = LuaPull<vector3d>(l, 1);
	auto res = vec * Pi::game->GetPlayer()->GetOrient();
	
	LuaPush<vector3d>(l, res);
	return 1;
}

static int l_engine_get_confirm_quit(lua_State *l)
{
	lua_pushboolean(l, Pi::config->Int("ConfirmQuit") != 0);
	return 1;
}

static int l_engine_set_confirm_quit(lua_State *l)
{
	if (lua_isnone(l, 1))
		return luaL_error(l, "ConfirmQuit takes one boolean argument");
	const bool confirm = lua_toboolean(l, 1);
	Pi::config->SetInt("ConfirmQuit", (confirm ? 1 : 0));
	Pi::config->Save();
	return 0;
}

static int l_engine_get_joystick_enabled(lua_State *l)
{
	lua_pushboolean(l, Pi::config->Int("EnableJoystick") != 0);
	return 1;
}

static int l_engine_set_joystick_enabled(lua_State *l)
{
	if (lua_isnone(l, 1))
		return luaL_error(l, "SetJoystickEnabled takes one boolean argument");
	const bool enabled = lua_toboolean(l, 1);
	Pi::config->SetInt("EnableJoystick", (enabled ? 1 : 0));
	Pi::config->Save();
	Pi::SetJoystickEnabled(enabled);
	return 0;
}

static int l_engine_get_model(lua_State *l)
{
	const std::string name(luaL_checkstring(l, 1));
	SceneGraph::Model *model = Pi::FindModel(name);
	LuaObject<SceneGraph::Model>::PushToLua(model);
	return 1;
}

static int l_get_can_browse_user_folders(lua_State *l)
{
	lua_pushboolean(l, OS::SupportsFolderBrowser());
	return 1;
}

static int l_browse_user_folders(lua_State *l)
{
	OS::OpenUserFolderBrowser();
	return 0;
}

void LuaEngine::Register()
{
	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);

	static const luaL_Reg l_methods[] = {
		{ "Quit", l_engine_quit },

		{ "GetVideoModeList", l_engine_get_video_mode_list },
		{ "GetVideoResolution", l_engine_get_video_resolution },
		{ "SetVideoResolution", l_engine_set_video_resolution },
		{ "GetFullscreen", l_engine_get_fullscreen },
		{ "SetFullscreen", l_engine_set_fullscreen },
		{ "GetVSyncEnabled", l_engine_get_vsync_enabled },
		{ "SetVSyncEnabled", l_engine_set_vsync_enabled },
		{ "GetTextureCompressionEnabled", l_engine_get_texture_compression_enabled },
		{ "SetTextureCompressionEnabled", l_engine_set_texture_compression_enabled },
		{ "GetMultisampling", l_engine_get_multisampling },
		{ "SetMultisampling", l_engine_set_multisampling },

		{ "GetGpuJobsEnabled", l_engine_get_gpu_jobs_enabled },
		{ "SetGpuJobsEnabled", l_engine_set_gpu_jobs_enabled },

		{ "GetPlanetDetailLevel", l_engine_get_planet_detail_level },
		{ "SetPlanetDetailLevel", l_engine_set_planet_detail_level },
		{ "GetCityDetailLevel", l_engine_get_city_detail_level },
		{ "SetCityDetailLevel", l_engine_set_city_detail_level },
		{ "GetFractalDetailLevel", l_engine_get_fractal_detail_level },
		{ "SetFractalDetailLevel", l_engine_set_fractal_detail_level },
		{ "GetPlanetFractalColourEnabled", l_engine_get_planet_fractal_colour_enabled },
		{ "SetPlanetFractalColourEnabled", l_engine_set_planet_fractal_colour_enabled },

		{ "GetDisplayNavTunnels", l_engine_get_display_nav_tunnels },
		{ "SetDisplayNavTunnels", l_engine_set_display_nav_tunnels },

		{ "GetDisplaySpeedLines", l_engine_get_display_speed_lines },
		{ "SetDisplaySpeedLines", l_engine_set_display_speed_lines },

		{ "GetCockpitEnabled", l_engine_get_cockpit_enabled },
		{ "SetCockpitEnabled", l_engine_set_cockpit_enabled },

		{ "GetAnisoFiltering", l_engine_get_aniso_enabled },
		{ "SetAnisoFiltering", l_engine_set_aniso_enabled },

		{ "GetAutosaveEnabled", l_engine_get_autosave_enabled },
		{ "SetAutosaveEnabled", l_engine_set_autosave_enabled },

		{ "GetDisplayHudTrails", l_engine_get_display_hud_trails },
		{ "SetDisplayHudTrails", l_engine_set_display_hud_trails },

		{ "GetConfirmQuit", l_engine_get_confirm_quit },
		{ "SetConfirmQuit", l_engine_set_confirm_quit },

		{ "SetAmountStars", l_engine_set_amount_stars },
		{ "GetAmountStars", l_engine_get_amount_stars },

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

		{ "GetKeyBindings", l_engine_get_key_bindings },
		{ "SetKeyBinding", l_engine_set_key_binding },
		{ "GetMouseYInverted", l_engine_get_mouse_y_inverted },
		{ "SetMouseYInverted", l_engine_set_mouse_y_inverted },
		{ "GetJoystickEnabled", l_engine_get_joystick_enabled },
		{ "SetJoystickEnabled", l_engine_set_joystick_enabled },

		{ "CanBrowseUserFolder", l_get_can_browse_user_folders },
		{ "OpenBrowseUserFolder", l_browse_user_folders },

		{ "GetModel", l_engine_get_model },

		{ "ShipSpaceToScreenSpace",   l_engine_ship_space_to_screen_space },
		{ "CameraSpaceToScreenSpace", l_engine_camera_space_to_screen_space },
		{ "WorldSpaceToScreenSpace",     l_engine_world_space_to_screen_space },
		{ "WorldSpaceToShipSpace",     l_engine_world_space_to_ship_space },
		{ 0, 0 }
	};

	static const luaL_Reg l_attrs[] = {
		{ "rand",    l_engine_attr_rand    },
		{ "ticks",   l_engine_attr_ticks   },
		{ "ui",      l_engine_attr_ui      },
		{ "pigui",   l_engine_attr_pigui   },
		{ "version", l_engine_attr_version },
		{ 0, 0 }
	};

	lua_getfield(l, LUA_REGISTRYINDEX, "CoreImports");
	LuaObjectBase::CreateObject(l_methods, l_attrs, 0);
	lua_setfield(l, -2, "Engine");
	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);
}
