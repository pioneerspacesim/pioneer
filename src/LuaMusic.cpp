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
 * Example:
 *
 * > Music.Play("songName")
 *
 * Parameters:
 *
 *   name - song file name, without paths or file extension
 *   repeat - true or false, default true
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
	const std::string song(luaL_checkstring(l, 1));
	bool repeat = true;
	if (lua_isboolean(l, 2))
		repeat = lua_toboolean(l, 2) != 0;
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
 * Example:
 *
 * > Music.FadeIn("songName", 0.5)
 *
 * Parameters:
 *
 *   name - song file name, without paths or file extension
 *   fade factor - 0.1 = slow fade, 1.0 = instant. The fade factor of our sound system does not represent any natural unit. Sorry.
 *   repeat - true or false, default true
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
	bool repeat = true;
	if (lua_isboolean(l, 3))
		repeat = lua_toboolean(l, 3) != 0;
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
static int l_fade_out(lua_State *l)
{
	const float fadedelta = luaL_checknumber(l, 1);
	Pi::GetMusicPlayer().FadeOut(fadedelta);
	return 0;
}

/*
 * Method: GetSongList
 *
 * Returns an table of the available song names and their paths.
 * The paths can be useful if you want to categorize the music.
 *
 * Example:
 *
 * > songs = Music.GetSongList()
 * > for key,value in pairs(songs) do
 * >     print(key, value)
 * > end
 * > -- prints:
 * > -- tingle    data/music/tingle.ogg
 * > -- knighty   data/music/knighty.ogg
 *
 * Availability:
 *
 *   alpha 12
 *
 * Status:
 *
 *   experimental
 */
static int l_get_song_list(lua_State *l)
{
	using std::vector;
	using std::string;
	using std::pair;
	const vector<pair<string, string> > vec = Pi::GetMusicPlayer().GetSongList();
	lua_newtable(l);
	int idx = 1;
	for (vector<pair<string, string> >::const_iterator it = vec.begin(); it != vec.end(); ++it) {
		lua_pushstring(l, it->first.c_str());
		lua_pushstring(l, it->second.c_str());
		lua_settable(l, -3);
		++idx;
	}
	return 1;
}

void LuaMusic::Register()
{
	lua_State *l = Pi::luaManager.GetLuaState();

	LUA_DEBUG_START(l);

	static const luaL_reg methods[]= {
		{ "GetSongName", l_get_song },
		{ "GetSongList", l_get_song_list },
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
