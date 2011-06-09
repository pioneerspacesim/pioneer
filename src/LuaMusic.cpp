#include "LuaMusic.h"
#include "LuaObject.h"
#include "LuaUtils.h"

//events: onSongFinished

//should be attr?
static int l_get_volume(lua_State *l)
{
	lua_pushnumber(l, 1.0);
	return 1;
}

static int l_set_volume(lua_State *l)
{
	return 0;
}

//get the currently (or last active) song name w/path
//should be attr?
static int l_get_song(lua_State *l)
{
	lua_pushstring(l, "lol.ogg");
	return 1;
}

//plays a song n times (-1 = forever)
static int l_play(lua_State *l)
{
	return 0;
}

//immediately stops the current song
static int l_stop(lua_State *l)
{
	return 0;
}

//fades a song in during n ms, + repeat like l_play
static int l_fade_in(lua_State *l)
{
	return 0;
}

//fades the song out in n ms and then stops
static int l_fade_out(lua_State *l)
{
	return 0;
}

void LuaMusic::Register()
{
	lua_State *l = Pi::luaManager.GetLuaState();

	LUA_DEBUG_START();

	static const luaL_reg methods[]= {
		{ "GetVolume", l_get_volume },
		{ "SetVolume", l_set_volume },
		{ "SongName", l_get_song },
		{ "Play", l_play },
		{ "Stop", l_stop},
		{ "FadeIn", l_fade_in },
		{ "FadeOut", l_fade_out },
		{0, 0}
	};

	luaL_register(l, "Music", methods);
	lua_pop(1, l);

	LUA_DEBUG_END(l, 0);
}
