// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaTimer.h"
#include "LuaUtils.h"
#include "Game.h"
#include "Pi.h"

void LuaTimer::RemoveAll()
{
    lua_State *l = Lua::manager->GetLuaState();

    lua_pushnil(l);
    lua_setfield(l, LUA_REGISTRYINDEX, "PiTimerCallbacks");
}

void LuaTimer::Tick()
{
	assert(Pi::game);
	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);

	lua_getfield(l, LUA_REGISTRYINDEX, "PiTimerCallbacks");
	if (lua_isnil(l, -1)) {
		lua_pop(l, 1);
		LUA_DEBUG_END(l, 0);
		return;
	}
	assert(lua_istable(l, -1));

	double now = Pi::game->GetTime();

	lua_pushnil(l);
	while (lua_next(l, -2)) {
		assert(lua_istable(l, -1));

		lua_getfield(l, -1, "at");
		double at = lua_tonumber(l, -1);
		lua_pop(l, 1);

		if (at <= now) {
			lua_getfield(l, -1, "callback");
			pi_lua_protected_call(l, 0, 1);
			bool cancel = lua_toboolean(l, -1);
			lua_pop(l, 1);

			lua_getfield(l, -1, "every");
			if (lua_isnil(l, -1) || cancel) {
				lua_pop(l, 1);

				lua_pushvalue(l, -2);
				lua_pushnil(l);
				lua_settable(l, -5);
			}
			else {
				double every = lua_tonumber(l, -1);
				lua_pop(l, 1);

				pi_lua_settable(l, "at", Pi::game->GetTime() + every);
			}
		}

		lua_pop(l, 1);
	}
	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);
}

/*
 * Class: Timer
 *
 * A class to invoke functions at specific times.
 *
 * The <Timer> class provides a facility whereby scripts can request that a
 * function be called at a given time, or regularly.
 *
 * Pioneer provides a single <Timer> object to the Lua environment. It resides
 * in the global namespace and is simply called Timer.
 *
 * The <Timer> is bound to the game clock, not the OS (real time) clock. The
 * game clock is subject to time acceleration. As such, timer triggers will
 * not necessarily occur at the exact time you request but can arrive seconds,
 * minutes or even hours after the requested time (game time).
 *
 * Because timer functions are called outside of the normal event model, it is
 * possible that game objects no longer exist. Consider this example:
 *
 * > local enemy = Space.SpawnShipNear("eagle_lrf", Game.player, 20, 20)
 * > Comms.ImportantMessage(enemy:GetLabel(), "You have 20 seconds to surrender or you will be destroyed.")
 * > Timer:CallAt(Game.time+20, function ()
 * >     Comms.ImportantMessage(enemy:GetLabel(), "You were warned. Prepare to die!")
 * >     enemy:Kill(Game.player)
 * > end)
 *
 * This works exactly as you'd expect: 20 seconds after the threat message is
 * sent, the enemy comes to life and attacks the player. If however the player
 * chooses to avoid the battle by hyperspacing away, the enemy ship is
 * destroyed by the game engine. In that case, the "enemy" object held by the
 * script is a shell, and any attempt to use it will be greeted by a Lua error.
 *
 * To protect against this, you should call <Object.exists> to confirm that the
 * underlying object exists before trying to use it.
 */

static void _finish_timer_create(lua_State *l)
{
	lua_pushstring(l, "callback");
	lua_pushvalue(l, 3);
	lua_settable(l, -3);

	lua_getfield(l, LUA_REGISTRYINDEX, "PiTimerCallbacks");
	if (lua_isnil(l, -1)) {
		lua_pop(l, 1);
		lua_newtable(l);
		lua_pushvalue(l, -1);
		lua_setfield(l, LUA_REGISTRYINDEX, "PiTimerCallbacks");
	}

	lua_insert(l, -2);
	lua_pushinteger(l, lua_rawlen(l, -2) + 1);
	lua_insert(l, -2);
	lua_settable(l, -3);

	lua_pop(l, 1);
}

/*
 * Method: CallAt
 *
 * Request that a function be called at a specific game time.
 *
 * > Timer:CallAt(time, function)
 *
 * Time acceleration may cause the function to be called long after the desired
 * time has passed.
 *
 * Parameters:
 *
 *   time - the absolute game time to call the function at. This will usually
 *          be created by adding some small amount to <Game.time>.
 *
 *   function - the function to call. Takes no parameters and returns nothing.
 *
 * Example:
 *
 * > Timer:CallAt(Game.time+30, function ()
 * >     Comms.Message("Special offer expired, sorry.")
 * > end)
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   stable
 */
static int l_timer_call_at(lua_State *l)
{
	if (!Pi::game) {
		luaL_error(l, "Game is not started");
		return 0;
	}

	double at = luaL_checknumber(l, 2);
	luaL_checktype(l, 3, LUA_TFUNCTION); // any type of function

	if (at <= Pi::game->GetTime())
		luaL_error(l, "Specified time is in the past");

	LUA_DEBUG_START(l);

	lua_newtable(l);
	pi_lua_settable(l, "at", at);

	_finish_timer_create(l);

	LUA_DEBUG_END(l, 0);

	return 0;
}

/*
 * Method: CallEvery
 *
 * Request that a function be called over at over at a regular interval.
 *
 * > Timer:CallEvery(interval, function)
 *
 * Since the <Timer> system is locked to the game time, time acceleration may
 * cause the function to be called more frequently than the corresponding
 * number of real-time seconds. Even under time acceleration, the function
 * will never called more than once per real-time second.
 *
 * If the called function returns a false value (as is the default for Lua
 * when no return value is specified), the timer will continue to be triggered
 * after each interval. To request that no further timer events be fired, the
 * function should explicitly return a true value.
 *
 * Parameters:
 *
 *   time - the interval between calls to the function, in seconds
 *
 *   function - the function to call. Returns false to continue receiving
 *              calls after the next interval, or true to cancel the timer.
 *
 * Example:
 *
 * > -- dump fuel every two seconds until none left
 * > Timer:CallEvery(2, function ()
 * >     local did_dump = Game.player:Jettison(Equip.Type.HYDROGEN)
 * >     return not did_dump
 * > end)
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   stable
 */
static int l_timer_call_every(lua_State *l)
{
	if (!Pi::game) {
		luaL_error(l, "Game is not started");
		return 0;
	}

	double every = luaL_checknumber(l, 2);
	luaL_checktype(l, 3, LUA_TFUNCTION); // any type of function

	if (every <= 0)
		luaL_error(l, "Specified interval must be greater than zero");

	LUA_DEBUG_START(l);

	lua_newtable(l);
	pi_lua_settable(l, "every", every);
	pi_lua_settable(l, "at", Pi::game->GetTime() + every);

	_finish_timer_create(l);

	LUA_DEBUG_END(l, 0);

	return 0;
}

template <> const char *LuaObject<LuaTimer>::s_type = "Timer";

template <> void LuaObject<LuaTimer>::RegisterClass()
{
	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);

	static const luaL_Reg l_methods[] = {
		{ "CallAt",    l_timer_call_at    },
		{ "CallEvery", l_timer_call_every },
		{ 0, 0 }
	};

	lua_getfield(l, LUA_REGISTRYINDEX, "CoreImports");
	LuaObjectBase::CreateObject(l_methods, 0, 0);
	lua_setfield(l, -2, "Timer");
	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);
}
