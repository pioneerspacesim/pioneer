// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaEngine.h"

#include "EnumStrings.h"
#include "FileSystem.h"
#include "FloatComparison.h"
#include "Game.h"
#include "GameConfig.h"
#include "Intro.h"
#include "KeyBindings.h"
#include "Lang.h"
#include "LuaColor.h"
#include "LuaConstants.h"
#include "LuaObject.h"
#include "LuaPiGui.h"
#include "LuaUtils.h"
#include "LuaVector.h"
#include "LuaVector2.h"
#include "Pi.h"
#include "Player.h"
#include "Random.h"
#include "SectorView.h"
#include "WorldView.h"
#include "buildopts.h"
#include "core/OS.h"
#include "graphics/Graphics.h"
#include "pigui/PiGui.h"
#include "scenegraph/Model.h"
#include "sound/Sound.h"
#include "sound/SoundMusic.h"
#include "ui/Context.h"
#include "utils.h"
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
	LuaObject<PiGui::Instance>::PushToLua(Pi::pigui);
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
	Pi::RequestQuit();
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

		lua_rawseti(l, -2, i + 1);
	}

	LUA_DEBUG_END(l, 1);
	return 1;
}

/*
* Method: GetMaximumAASamples
*
* Get the maximum number of samples the current OpenGL context supports
*
* > Engine.GetMaximumAASamples()
*
* Availability:
*
*   2017-12
*
* Status:
*
*   stable
*/

static int l_engine_get_maximum_aa_samples(lua_State *l)
{
	LUA_DEBUG_START(l);

	if (Pi::renderer != nullptr) {
		int maxSamples = Pi::renderer->GetMaximumNumberAASamples();
		lua_pushinteger(l, maxSamples);
	} else {
		lua_pushinteger(l, 0);
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
	lua_pushinteger(l, Pi::config->Int("ScrWidth"));
	lua_pushinteger(l, Pi::config->Int("ScrHeight"));
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

/*
 * Method: SetShowDebugInfo
 *
 * Show, hide, or toggle the debug information window
 *
 * > Engine.SetShowDebugInfo(true)
 * > // toggle
 * > Engine.SetShowDebugInfo()
 *
 * Parameters:
 *
 *   enabled - true to show, false to hide. If not present, toggles the state instead
 *
 * Availability: 2020-05
 *
 * Status: experimental
 */
static int l_engine_set_show_debug_info(lua_State *l)
{
	if (lua_gettop(l) < 1) {
		Pi::ToggleShowDebugInfo();
	} else {
		const bool enabled = lua_toboolean(l, 1);
		Pi::SetShowDebugInfo(enabled);
	}
	return 0;
}

static int l_engine_get_enum_value(lua_State *l)
{
	auto enum_name = LuaPull<const char *>(l, 1);
	auto enum_tag = LuaPull<const char *>(l, 2);
	LuaPush<int>(l, EnumStrings::GetValue(enum_name, enum_tag));
	return 1;
}

static int l_engine_get_disable_screenshot_info(lua_State *l)
{
	LuaPush<bool>(l, Pi::config->Int("DisableScreenshotInfo") != 0);
	return 1;
}

static int l_engine_set_disable_screenshot_info(lua_State *l)
{
	if (lua_isnone(l, 1))
		return luaL_error(l, "SetDisableScreenshotInfo takes one boolean argument");
	const bool disable = LuaPull<bool>(l, 1);
	Pi::config->SetInt("DisableScreenshotInfo", (disable ? 1 : 0));
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
		return luaL_error(l, "SetAutosaveEnabled takes one boolean argument");
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

static int l_engine_is_intro_zooming(lua_State *l)
{
	if (Pi::intro) {
		LuaPush(l, Pi::intro->isZooming());
		return 1;
	} else {
		LuaPush(l, false);
		return 1;
	}
}

static int l_engine_get_intro_current_model_name(lua_State *l)
{
	if (Pi::intro) {
		SceneGraph::Model *m = Pi::intro->getCurrentModel();
		if (m) {
			LuaPush(l, m->GetName());
			return 1;
		} else {
			lua_pushnil(l);
			return 1;
		}
	} else {
		lua_pushnil(l);
		return 1;
	}
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
	LuaPush<vector3d>(l, cam);
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

	PiGUI::TScreenSpace res = PiGUI::lua_world_space_to_screen_space(pos); // defined in LuaPiGui.cpp

	LuaPush<bool>(l, res._onScreen);
	LuaPush<vector2d>(l, res._screenPosition);
	LuaPush<vector3d>(l, res._direction);
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

static int l_engine_get_model(lua_State *l)
{
	const std::string name(luaL_checkstring(l, 1));
	SceneGraph::Model *model = Pi::FindModel(name);
	LuaObject<SceneGraph::Model>::PushToLua(model);
	return 1;
}

static int l_engine_sector_map_clear_route(lua_State *l)
{
	SectorView *sv = Pi::game->GetSectorView();
	sv->ClearRoute();
	return 0;
}

static int l_engine_sector_map_add_to_route(lua_State *l)
{
	SectorView *sv = Pi::game->GetSectorView();
	SystemPath *path = LuaObject<SystemPath>::CheckFromLua(1);
	sv->AddToRoute(path);
	return 0;
}

static int l_engine_get_sector_map_zoom_level(lua_State *l)
{
	SectorView *sv = Pi::game->GetSectorView();
	LuaPush(l, sv->GetZoomLevel());
	return 1;
}

static int l_engine_get_sector_map_center_distance(lua_State *l)
{
	SectorView *sv = Pi::game->GetSectorView();
	LuaPush(l, sv->GetCenterDistance());
	return 1;
}

static int l_engine_get_sector_map_center_sector(lua_State *l)
{
	SectorView *sv = Pi::game->GetSectorView();
	LuaPush<vector3d>(l, vector3d(sv->GetCenterSector()));
	return 1;
}

static int l_engine_get_sector_map_current_system_path(lua_State *l)
{
	SectorView *sv = Pi::game->GetSectorView();
	LuaObject<SystemPath>::PushToLua(sv->GetCurrent());
	return 1;
}

static int l_engine_get_sector_map_selected_system_path(lua_State *l)
{
	SectorView *sv = Pi::game->GetSectorView();
	LuaObject<SystemPath>::PushToLua(sv->GetSelected());
	return 1;
}

static int l_engine_get_sector_map_hyperspace_target_system_path(lua_State *l)
{
	SectorView *sv = Pi::game->GetSectorView();
	LuaObject<SystemPath>::PushToLua(sv->GetHyperspaceTarget());
	return 1;
}

static int l_engine_set_sector_map_draw_uninhabited_labels(lua_State *l)
{
	SectorView *sv = Pi::game->GetSectorView();
	bool value = LuaPull<bool>(l, 1);
	sv->SetDrawUninhabitedLabels(value);
	return 0;
}

static int l_engine_set_sector_map_draw_out_range_labels(lua_State *l)
{
	SectorView *sv = Pi::game->GetSectorView();
	bool value = LuaPull<bool>(l, 1);
	sv->SetDrawOutRangeLabels(value);
	return 0;
}

static int l_engine_set_sector_map_lock_hyperspace_target(lua_State *l)
{
	SectorView *sv = Pi::game->GetSectorView();
	bool value = LuaPull<bool>(l, 1);
	sv->LockHyperspaceTarget(value);
	return 0;
}

static int l_engine_set_sector_map_draw_vertical_lines(lua_State *l)
{
	SectorView *sv = Pi::game->GetSectorView();
	bool value = LuaPull<bool>(l, 1);
	sv->SetDrawVerticalLines(value);
	return 0;
}

static int l_engine_set_sector_map_automatic_system_selection(lua_State *l)
{
	SectorView *sv = Pi::game->GetSectorView();
	bool value = LuaPull<bool>(l, 1);
	sv->SetAutomaticSystemSelection(value);
	return 0;
}

static int l_engine_sector_map_get_route(lua_State *l)
{
	SectorView *sv = Pi::game->GetSectorView();
	std::vector<SystemPath> route = sv->GetRoute();

	lua_newtable(l);
	int i = 1;
	for (const SystemPath &j : route) {
		lua_pushnumber(l, i++);
		LuaObject<SystemPath>::PushToLua(j);
		lua_settable(l, -3);
	}
	return 1;
}

static int l_engine_sector_map_get_route_size(lua_State *l)
{
	SectorView *sv = Pi::game->GetSectorView();
	std::vector<SystemPath> route = sv->GetRoute();
	const int size = route.size();
	LuaPush(l, size);
	return 1;
}

static int l_engine_sector_map_auto_route(lua_State *l)
{
	SectorView *sv = Pi::game->GetSectorView();
	SystemPath current_path = sv->GetCurrent();
	SystemPath target_path = sv->GetSelected();

	std::vector<SystemPath> route;
	sv->AutoRoute(current_path, target_path, route);
	sv->ClearRoute();
	for (auto it = route.begin(); it != route.end(); it++) {
		sv->AddToRoute(*it);
	}

	return l_engine_sector_map_get_route(l);
}

static int l_engine_sector_map_move_route_item_up(lua_State *l)
{
	SectorView *sv = Pi::game->GetSectorView();
	int element = LuaPull<int>(l, 1);

	// lua indexes start at 1
	element -= 1;

	bool r = sv->MoveRouteItemUp(element);
	LuaPush<bool>(l, r);
	return 1;
}

static int l_engine_sector_map_move_route_item_down(lua_State *l)
{
	SectorView *sv = Pi::game->GetSectorView();
	int element = LuaPull<int>(l, 1);

	// lua indexes start at 1
	element -= 1;

	bool r = sv->MoveRouteItemDown(element);
	LuaPush<bool>(l, r);
	return 1;
}

static int l_engine_sector_map_remove_route_item(lua_State *l)
{
	SectorView *sv = Pi::game->GetSectorView();
	int element = LuaPull<int>(l, 1);

	// lua indexes start at 1
	element -= 1;

	bool r = sv->RemoveRouteItem(element);
	LuaPush<bool>(l, r);
	return 1;
}

static int l_engine_set_sector_map_selected(lua_State *l)
{
	SectorView *sv = Pi::game->GetSectorView();
	SystemPath *path = LuaObject<SystemPath>::CheckFromLua(1);
	sv->SetSelected(*path);
	return 0;
}

static int l_engine_sector_map_goto_sector_path(lua_State *l)
{
	SectorView *sv = Pi::game->GetSectorView();
	SystemPath *path = LuaObject<SystemPath>::CheckFromLua(1);
	sv->GotoSector(*path);
	return 0;
}

static int l_engine_sector_map_goto_system_path(lua_State *l)
{
	SectorView *sv = Pi::game->GetSectorView();
	SystemPath *path = LuaObject<SystemPath>::CheckFromLua(1);
	sv->GotoSystem(*path);
	return 0;
}

static int l_engine_search_nearby_star_systems_by_name(lua_State *l)
{
	SectorView *sv = Pi::game->GetSectorView();
	std::string pattern = LuaPull<std::string>(l, 1);

	std::vector<SystemPath> matches = sv->GetNearbyStarSystemsByName(pattern);
	int i = 1;
	lua_newtable(l);
	for (const SystemPath &path : matches) {
		lua_pushnumber(l, i++);
		LuaObject<SystemPath>::PushToLua(path);
		lua_settable(l, -3);
	}
	return 1;
}

static int l_engine_sector_map_zoom_in(lua_State *l)
{
	Pi::game->GetSectorView()->ZoomIn();
	return 0;
}

static int l_engine_sector_map_zoom_out(lua_State *l)
{
	Pi::game->GetSectorView()->ZoomOut();
	return 0;
}

static int l_engine_set_sector_map_faction_visible(lua_State *l)
{
	SectorView *sv = Pi::game->GetSectorView();
	Faction *faction = LuaObject<Faction>::CheckFromLua(1);
	bool visible = LuaPull<bool>(l, 2);
	sv->SetFactionVisible(faction, visible);
	return 0;
}

static int l_engine_get_sector_map_factions(lua_State *l)
{
	SectorView *sv = Pi::game->GetSectorView();
	const std::set<const Faction *> visible = sv->GetVisibleFactions();
	const std::set<const Faction *> hidden = sv->GetHiddenFactions();
	lua_newtable(l); // outer table
	int i = 1;
	for (const Faction *f : visible) {
		lua_pushnumber(l, i++);
		lua_newtable(l); // inner table
		LuaObject<Faction>::PushToLua(const_cast<Faction *>(f));
		lua_setfield(l, -2, "faction");
		lua_pushboolean(l, hidden.count(f) == 0);
		lua_setfield(l, -2, "visible"); // inner table
		lua_settable(l, -3);			// outer table
	}
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

		{ "SetShowDebugInfo", l_engine_set_show_debug_info },

		{ "GetVideoModeList", l_engine_get_video_mode_list },
		{ "GetMaximumAASamples", l_engine_get_maximum_aa_samples },
		{ "GetVideoResolution", l_engine_get_video_resolution },
		{ "SetVideoResolution", l_engine_set_video_resolution },
		{ "GetFullscreen", l_engine_get_fullscreen },
		{ "SetFullscreen", l_engine_set_fullscreen },
		{ "GetDisableScreenshotInfo", l_engine_get_disable_screenshot_info },
		{ "SetDisableScreenshotInfo", l_engine_set_disable_screenshot_info },
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

		{ "CanBrowseUserFolder", l_get_can_browse_user_folders },
		{ "OpenBrowseUserFolder", l_browse_user_folders },

		{ "GetModel", l_engine_get_model },

		{ "IsIntroZooming", l_engine_is_intro_zooming },
		{ "GetIntroCurrentModelName", l_engine_get_intro_current_model_name },

		{ "GetSectorMapZoomLevel", l_engine_get_sector_map_zoom_level },
		{ "SectorMapZoomIn", l_engine_sector_map_zoom_in },
		{ "SectorMapZoomOut", l_engine_sector_map_zoom_out },
		{ "GetSectorMapCenterSector", l_engine_get_sector_map_center_sector },
		{ "GetSectorMapCenterDistance", l_engine_get_sector_map_center_distance },
		{ "GetSectorMapCurrentSystemPath", l_engine_get_sector_map_current_system_path },
		{ "GetSectorMapSelectedSystemPath", l_engine_get_sector_map_selected_system_path },
		{ "GetSectorMapHyperspaceTargetSystemPath", l_engine_get_sector_map_hyperspace_target_system_path },
		{ "SetSectorMapDrawUninhabitedLabels", l_engine_set_sector_map_draw_uninhabited_labels },
		{ "SetSectorMapDrawVerticalLines", l_engine_set_sector_map_draw_vertical_lines },
		{ "SetSectorMapDrawOutRangeLabels", l_engine_set_sector_map_draw_out_range_labels },
		{ "SetSectorMapAutomaticSystemSelection", l_engine_set_sector_map_automatic_system_selection },
		{ "SetSectorMapLockHyperspaceTarget", l_engine_set_sector_map_lock_hyperspace_target },
		{ "SetSectorMapSelected", l_engine_set_sector_map_selected },
		{ "SectorMapGotoSectorPath", l_engine_sector_map_goto_sector_path },
		{ "SectorMapGotoSystemPath", l_engine_sector_map_goto_system_path },
		{ "GetSectorMapFactions", l_engine_get_sector_map_factions },
		{ "SetSectorMapFactionVisible", l_engine_set_sector_map_faction_visible },
		{ "SectorMapAutoRoute", l_engine_sector_map_auto_route },
		{ "SectorMapGetRoute", l_engine_sector_map_get_route },
		{ "SectorMapGetRouteSize", l_engine_sector_map_get_route_size },
		{ "SectorMapMoveRouteItemUp", l_engine_sector_map_move_route_item_up },
		{ "SectorMapMoveRouteItemDown", l_engine_sector_map_move_route_item_down },
		{ "SectorMapRemoveRouteItem", l_engine_sector_map_remove_route_item },
		{ "SectorMapClearRoute", l_engine_sector_map_clear_route },
		{ "SectorMapAddToRoute", l_engine_sector_map_add_to_route },
		{ "SearchNearbyStarSystemsByName", l_engine_search_nearby_star_systems_by_name },
		{ "ShipSpaceToScreenSpace", l_engine_ship_space_to_screen_space },
		{ "CameraSpaceToScreenSpace", l_engine_camera_space_to_screen_space },
		{ "WorldSpaceToScreenSpace", l_engine_world_space_to_screen_space },
		{ "WorldSpaceToShipSpace", l_engine_world_space_to_ship_space },
		{ "GetEnumValue", l_engine_get_enum_value },
		{ 0, 0 }
	};

	static const luaL_Reg l_attrs[] = {
		{ "rand", l_engine_attr_rand },
		{ "ticks", l_engine_attr_ticks },
		{ "ui", l_engine_attr_ui },
		{ "pigui", l_engine_attr_pigui },
		{ "version", l_engine_attr_version },
		{ 0, 0 }
	};

	lua_getfield(l, LUA_REGISTRYINDEX, "CoreImports");
	LuaObjectBase::CreateObject(l_methods, l_attrs, 0);
	lua_setfield(l, -2, "Engine");
	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);
}
