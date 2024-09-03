// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaEngine.h"

#include "EnumStrings.h"
#include "FileSystem.h"
#include "FloatComparison.h"
#include "Game.h"
#include "GameConfig.h"
#include "Intro.h"
#include "Lang.h"
#include "LuaColor.h"
#include "LuaConstants.h"
#include "LuaObject.h"
#include "LuaPiGuiInternal.h"
#include "LuaUtils.h"
#include "LuaVector.h"
#include "LuaVector2.h"
#include "Pi.h"
#include "Player.h"
#include "Random.h"
#include "SDL_video.h"
#include "WorldView.h"
#include "buildopts.h"
#include "core/OS.h"
#include "graphics/Graphics.h"
#include "pigui/PiGui.h"
#include "scenegraph/Model.h"
#include "sound/Sound.h"
#include "sound/SoundMusic.h"
#include "utils.h"

#include <SDL_timer.h>
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
 * Attribute: time
 *
 * Number of real-time seconds since Pioneer was started. This should be used
 * for debugging or UI purposes only (eg animations), and should never be used
 * in game logic of any kind.
 *
 * Availability:
 *
 *   July 2022
 *
 * Status:
 *
 *   stable
 */
static int l_engine_attr_time(lua_State *l)
{
	lua_pushnumber(l, Pi::GetApp()->GetTime());
	return 1;
}

/*
 * Attribute: frameTime
 *
 * Length of the last frame in seconds. This should be used for debugging or UI
 * purposes only (e.g. animations) and should never be used in game logic of
 * any kind.
 *
 * Availability:
 *
 *   July 2022
 *
 * Status:
 *
 *   stable
 */
static int l_engine_attr_frame_time(lua_State *l)
{
	lua_pushnumber(l, Pi::GetFrameTime());
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

	Pi::renderer->SetVSyncEnabled(vsync);
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
	const float amount = Clamp(luaL_checknumber(l, 1), 0.0, 1.0);
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

static int l_engine_set_star_field_star_size_factor(lua_State *l)
{
	const float amount = Clamp(luaL_checknumber(l, 1), 0.0, 1.0);
	Pi::config->SetFloat("StarFieldStarSizeFactor", amount);
	Pi::config->Save();
	Pi::SetStarFieldStarSizeFactor(amount);
	return 0;
}

static int l_engine_get_star_field_star_size_factor(lua_State *l)
{
	lua_pushnumber(l, Pi::config->Float("StarFieldStarSizeFactor"));
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

static int l_engine_get_realistic_scattering(lua_State *l)
{
	lua_pushinteger(l, Pi::config->Int("RealisticScattering"));
	return 1;
}

static int l_engine_set_realistic_scattering(lua_State *l)
{
	const int scattering = luaL_checkinteger(l, 1);
	if (scattering != Pi::config->Int("RealisticScattering")) {
		Pi::config->SetInt("RealisticScattering", scattering);
		Pi::config->Save();
		Pi::OnChangeDetailLevel();
	}
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
 * Method: WorldSpaceToShipSpace
 *
 * Convert a direction Vector from world space to screen space
 *
 * > screen_space = Engine.WorldSpaceToShipSpace(world_space)
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

static int l_engine_world_space_to_ship_space(lua_State *l)
{
	vector3d vec = LuaPull<vector3d>(l, 1);
	auto res = vec * Pi::game->GetPlayer()->GetOrient();

	LuaPush<vector3d>(l, res);
	return 1;
}

/*
 * Method: ShipSpaceToScreenSpace
 *
 * Convert a direction Vector from ship space to screen space
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
	vector3d cam = Pi::game->GetWorldView()->WorldDirToScreenSpace(Pi::player->GetInterpOrient() * pos);
	LuaPush<vector3d>(l, cam);
	return 1;
}

/*
 * Method: CameraSpaceToScreenSpace
 *
 * Convert a direction Vector from camera space to screen space
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
 * Method: ProjectRelPosition
 *
 * Project a player-relative position onto the screen
 *
 * > visible, position, direction = Engine.ProjectRelativePosition(rel_space)
 *
 * Parameters:
 *
 *   rel_space - a position Vector in player-relative space
 *               (e.g. `body:GetPositionRelTo(player)`)*
 * Availability:
 *
 *   2020-12
 *
 * Status:
 *
 *   stable
 */

static int l_engine_project_rel_position(lua_State *l)
{
	vector3d pos = LuaPull<vector3d>(l, 1);

	pos = Pi::game->GetWorldView()->WorldSpaceToScreenSpace(pos + Pi::player->GetInterpPosition());
	return PiGui::pushOnScreenPositionDirection(l, pos);
}

/*
 * Method: ProjectRelDirection
 *
 * Project a direction relative to the player ship onto the screen
 *
 * > visible, position, direction = Engine.ProjectRelativeDirection(rel_space)
 *
 * Parameters:
 *
 *   rel_space - a direction Vector in player-relative space
 *               (e.g. `body:GetVelocityRelTo(player)`)
 *
 * Availability:
 *
 *   2020-12
 *
 * Status:
 *
 *   stable
 */

static int l_engine_project_rel_direction(lua_State *l)
{
	vector3d pos = LuaPull<vector3d>(l, 1);

	pos = Pi::game->GetWorldView()->WorldDirToScreenSpace(pos.NormalizedSafe());
	return PiGui::pushOnScreenPositionDirection(l, pos);
}

/*
 * Function: GetBodyProjectedScreenPosition
 *
 * Get the body's position projected to screen space as a Vector
 *
 * > Engine.GetBodyProjectedScreenPosition(body)
 *
 * Parameters:
 *   body - a <Body> to project onto the screen.
 *
 * Returns:
 *   onscreen - a boolean indicating if the body's position is visible.
 *   position - the screen-space position of the body if onscreen.
 *   direction - the screen-space direction from the center of the screen
 *               to the body if offscreen.
 *
 * Availability:
 *
 *   2020-12
 *
 * Status:
 *
 *   experimental
 */

static int l_engine_get_projected_screen_position(lua_State *l)
{
	Body *b = LuaObject<Body>::CheckFromLua(1);
	vector3d p = Pi::game->GetWorldView()->WorldSpaceToScreenSpace(b);
	return PiGui::pushOnScreenPositionDirection(l, p);
}

/*
 * Function: GetTargetIndicatorScreenPosition
 *
 * Get a body's nav-target indicator override projected to screen space as a Vector
 *
 * > Engine.GetTargetIndicatorScreenPosition(body)
 *
 * Parameters:
 *   body - a <Body> to project onto the screen.
 *
 * Returns:
 *   onscreen - a boolean indicating if the body's position is visible.
 *   position - the screen-space position of the body if onscreen.
 *   direction - the screen-space direction from the center of the screen
 *               to the body if offscreen.
 *
 * Availability:
 *
 *   2020-12
 *
 * Status:
 *
 *   experimental
 */

static int l_engine_get_target_indicator_screen_position(lua_State *l)
{
	Body *b = LuaObject<Body>::CheckFromLua(1);
	vector3d p = Pi::game->GetWorldView()->GetTargetIndicatorScreenPosition(b);
	return PiGui::pushOnScreenPositionDirection(l, p);
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

static int l_engine_request_profile_frame(lua_State *l)
{
	if (lua_gettop(l) > 0) {
		Pi::GetApp()->RequestProfileFrame(luaL_checkstring(l, 1));
	} else {
		Pi::GetApp()->RequestProfileFrame();
	}

	return 0;
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

		{ "GetRealisticScattering", l_engine_get_realistic_scattering },
		{ "SetRealisticScattering", l_engine_set_realistic_scattering },

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
		{ "SetStarFieldStarSizeFactor", l_engine_set_star_field_star_size_factor },
		{ "GetStarFieldStarSizeFactor", l_engine_get_star_field_star_size_factor },

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

		{ "WorldSpaceToShipSpace", l_engine_world_space_to_ship_space },
		{ "ShipSpaceToScreenSpace", l_engine_ship_space_to_screen_space },
		{ "CameraSpaceToScreenSpace", l_engine_camera_space_to_screen_space },
		{ "ProjectRelPosition", l_engine_project_rel_position },
		{ "ProjectRelDirection", l_engine_project_rel_direction },
		{ "GetBodyProjectedScreenPosition", l_engine_get_projected_screen_position },
		{ "GetTargetIndicatorScreenPosition", l_engine_get_target_indicator_screen_position },
		{ "GetEnumValue", l_engine_get_enum_value },

		{ "RequestProfileFrame", l_engine_request_profile_frame },
		{ 0, 0 }
	};

	static const luaL_Reg l_attrs[] = {
		{ "rand", l_engine_attr_rand },
		{ "ticks", l_engine_attr_ticks },
		{ "time", l_engine_attr_time },
		{ "frameTime", l_engine_attr_frame_time },
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
