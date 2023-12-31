// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaMusic.h"
#include "LuaEvent.h"
#include "LuaObject.h"
#include "LuaUtils.h"
#include "Pi.h"
#include "sound/SoundMusic.h"

/*
 * Class: Music
 *
 * A class to control music playback
 *
 * Event: onSongFinished
 *
 * Triggered when a non-repeating song has played all the way to the end.
 *
 * This event does not not trigger when a song finishes
 * prematurely (sound system stopped, another song starts playing).
 *
 * Availability:
 *
 *   alpha 12
 *
 * Status:
 *
 *   experimental
 */

/*
 * Function: GetSongName
 *
 * Get the currently playing, or last song's name.
 *
 * Example:
 *
 * > name = Music.GetSongName()
 *
 * The name does not include the data/music/ prefix or .ogg suffix.
 *
 * Availability:
 *
 *   alpha 12
 *
 * Status:
 *
 *   experimental
 */
static int l_music_get_song(lua_State *l)
{
	lua_pushstring(l, Pi::GetMusicPlayer().GetCurrentSongName().c_str());
	return 1;
}

/*
 * Method: Play
 *
 * Starts playing a song instantly.
 *
 * Example:
 *
 * > Music.Play("action/track01")
 *
 * Parameters:
 *
 *   name - song file name, without data/music/ or file extension
 *   repeat - true or false, default false
 *
 * Availability:
 *
 *   alpha 12
 *
 * Status:
 *
 *   experimental
 */
static int l_music_play(lua_State *l)
{
	const std::string song(luaL_checkstring(l, 1));
	bool repeat = LuaPull<bool>(l, 2, false);
	Pi::GetMusicPlayer().Play(song, repeat);
	return 0;
}

/*
 * Method: Stop
 *
 * Immediately stops the currently playing song.
 *
 * Example:
 *
 * > Music.Stop()
 *
 * Availability:
 *
 *   alpha 12
 *
 * Status:
 *
 *   experimental
 */
static int l_music_stop(lua_State *l)
{
	Pi::GetMusicPlayer().Stop();
	return 0;
}

/*
 * Method: FadeIn
 *
 * Fades in a song and fades out any currently playing song (crossfade).
 *
 * Example:
 *
 * > Music.FadeIn("songName", 0.5)
 *
 * Parameters:
 *
 *   name - song file name, without data/music/ or file extension
 *   fade factor - 0.1 = slow fade, 1.0 = instant. The fade factor of our sound system does not represent any natural unit. Sorry.
 *   repeat - true or false, default false
 *
 * Availability:
 *
 *   alpha 12
 *
 * Status:
 *
 *   experimental
 */
static int l_music_fade_in(lua_State *l)
{
	const std::string song(luaL_checkstring(l, 1));
	const float fadedelta = luaL_checknumber(l, 2);
	bool repeat = LuaPull<bool>(l, 3, false);
	Pi::GetMusicPlayer().Play(song, repeat, fadedelta);
	return 0;
}

/*
 * Method: FadeOut
 *
 * Fades the currently playing song to silence and then stops it.
 *
 * Example:
 *
 * > Music.FadeOut(0.8)
 *
 * Parameters:
 *
 *   fade factor - 0.1 = slow fade, 1.0 = instant.
 *
 * Availability:
 *
 *   alpha 12
 *
 * Status:
 *
 *   experimental
 */
static int l_music_fade_out(lua_State *l)
{
	const float fadedelta = luaL_checknumber(l, 1);
	Pi::GetMusicPlayer().FadeOut(fadedelta);
	return 0;
}

/*
 * Method: GetSongList
 *
 * Returns an table of the available song names.
 *
 * Example:
 *
 * > songs = Music.GetSongList()
 * > for key,value in pairs(songs) do
 * >     print(key, value)
 * > end
 * > -- prints:
 * > -- 1   tingle
 * > -- 2   action/knighty
 *
 * Availability:
 *
 *   alpha 12
 *
 * Status:
 *
 *   experimental
 */
static int l_music_get_song_list(lua_State *l)
{
	using std::string;
	using std::vector;
	const vector<string> vec = Pi::GetMusicPlayer().GetSongList();
	lua_newtable(l);
	int idx = 1;
	for (vector<string>::const_iterator it = vec.begin(); it != vec.end(); ++it) {
		lua_pushnumber(l, idx);
		lua_pushstring(l, it->c_str());
		lua_settable(l, -3);
		++idx;
	}
	return 1;
}

/*
 * Function: IsPlaying
 *
 * Check if music is currently playing.
 *
 * Returns:
 *
 *   true or false
 *
 * Availability:
 *
 *   alpha 12
 *
 * Status:
 *
 *   experimental
 */
static int l_music_is_playing(lua_State *l)
{
	const bool playing = Pi::GetMusicPlayer().IsPlaying();
	lua_pushboolean(l, playing);
	return 1;
}

void LuaMusic::Register()
{
	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);

	static const luaL_Reg l_methods[] = {
		{ "GetSongName", l_music_get_song },
		{ "GetSongList", l_music_get_song_list },
		{ "Play", l_music_play },
		{ "Stop", l_music_stop },
		{ "FadeIn", l_music_fade_in },
		{ "FadeOut", l_music_fade_out },
		{ "IsPlaying", l_music_is_playing },
		{ 0, 0 }
	};

	lua_getfield(l, LUA_REGISTRYINDEX, "CoreImports");
	LuaObjectBase::CreateObject(l_methods, 0, 0);
	lua_setfield(l, -2, "Music");
	lua_pop(l, 1);

	Pi::GetMusicPlayer().onSongFinished.connect([]() {
		LuaEvent::Queue("onSongFinished");
	});

	LUA_DEBUG_END(l, 0);
}
