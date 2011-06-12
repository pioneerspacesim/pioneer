#include "LuaMusic.h"
#include "LuaObject.h"
#include "LuaUtils.h"
#include "Pi.h"
#include "SoundMusic.h"

//events: onSongFinished

//get the currently (or last active) song name w/path
//should be attr?
static int l_get_song(lua_State *l)
{
	lua_pushstring(l, Pi::GetMusicPlayer().GetCurrentSongName().c_str());
	return 1;
}

//plays a song immediately, repeating
static int l_play(lua_State *l)
{
	std::string song(luaL_checkstring(l, 1));
	Pi::GetMusicPlayer().Play(song, true);
	return 0;
}

//immediately stops the current song
static int l_stop(lua_State *l)
{
	Pi::GetMusicPlayer().Stop();
	return 0;
}

//fades in a song using fade factor and fades out the currently playing song (aka crossfade)
static int l_fade_in(lua_State *l)
{
	const std::string song(luaL_checkstring(l, 1));
	const float fadedelta = luaL_checknumber(l, 2);
	Pi::GetMusicPlayer().Play(song, true, fadedelta);
	return 0;
}

//fades the current song out and then stops
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
