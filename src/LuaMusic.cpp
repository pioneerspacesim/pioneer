#include "LuaMusic.h"
#include "LuaObject.h"
#include "LuaUtils.h"
#include "Pi.h"
#include "SoundMusic.h"

/*
 * Class: Music
 *
 * A class to control music playback
 *
 * Event: onSongFinished
 *
 * Triggered when a non-repeating song has played all the way to the end.
 *
 * Besides songs on repeat, this event does not trigger when a song finishes
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
 * > name = Music.GetSongName()
 *
 * The name does not include any paths or the .ogg suffix.
 *
 * Availability:
 *
 *   alpha 12
 *
 * Status:
 *
 *   experimental
 */
static int l_get_song(lua_State *l)
{
	lua_pushstring(l, Pi::GetMusicPlayer().GetCurrentSongName().c_str());
	return 1;
}

/*
 * Method: Play
 *
 * Starts playing a song instantly, on repeat by default.
 *
 * > Music.Play("songName")
 *
 * Availability:
 *
 *   alpha 12
 *
 * Status:
 *
 *   experimental
 */
static int l_play(lua_State *l)
{
	std::string song(luaL_checkstring(l, 1));
	Pi::GetMusicPlayer().Play(song, true);
	return 0;
}

/*
 * Method: Stop
 *
 * Immediately stops the currently playing song.
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
static int l_stop(lua_State *l)
{
	Pi::GetMusicPlayer().Stop();
	return 0;
}

/*
 * Method: FadeIn
 *
 * Fades in a song and fades out any currently playing song (crossfade).
 *
 * > Music.FadeIn("songName", 0.5)
 *
 * The fade factor of our sound system does not represent any natural unit.
 * Sorry. 0.1 = slow fade, 1.0 = instant.
 *
 * Availability:
 *
 *   alpha 12
 *
 * Status:
 *
 *   experimental
 */
static int l_fade_in(lua_State *l)
{
	const std::string song(luaL_checkstring(l, 1));
	const float fadedelta = luaL_checknumber(l, 2);
	Pi::GetMusicPlayer().Play(song, true, fadedelta);
	return 0;
}

/*
 * Method: FadeOut
 *
 * Fades the currently playing song to silence and then stops it.
 *
 * > Music.FadeOut(0.8)
 *
 * Availability:
 *
 *   alpha 12
 *
 * Status:
 *
 *   experimental
 */
static int l_fade_out(lua_State *l)
{
	const float fadedelta = luaL_checknumber(l, 1);
	Pi::GetMusicPlayer().FadeOut(fadedelta);
	return 0;
}

void LuaMusic::Register()
{
	lua_State *l = Pi::luaManager.GetLuaState();

	LUA_DEBUG_START();

	static const luaL_reg methods[]= {
		{ "GetSongName", l_get_song },
		{ "Play", l_play },
		{ "Stop", l_stop},
		{ "FadeIn", l_fade_in },
		{ "FadeOut", l_fade_out },
		{0, 0}
	};

	luaL_register(l, "Music", methods);
	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);
}
