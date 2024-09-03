// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaTimer.h"
#include "Game.h"
#include "Lua.h"
#include "LuaObject.h"
#include "LuaUtils.h"
#include "Pi.h"
#include "profiler/Profiler.h"

#include <algorithm>

LuaTimer::LuaTimer()
{
	m_called.reserve(8);
}

LuaTimer::~LuaTimer()
{
}

void LuaTimer::Insert(double at, int callbackId, bool repeats)
{
	CallInfo info { at, callbackId, repeats };

	// Keep the queue sorted when inserting new elements.
	// This is O(log N) + O(N/2) worst-case
	auto iter = std::lower_bound(m_timeouts.cbegin(), m_timeouts.cend(), info, [](const CallInfo &timeout, const CallInfo &val) {
		return timeout.at < val.at;
	});

	m_timeouts.insert(iter, std::move(info));
}

void LuaTimer::RemoveAll()
{
	lua_State *l = Lua::manager->GetLuaState();

	lua_pushnil(l);
	lua_setfield(l, LUA_REGISTRYINDEX, "PiTimerCallbacks");
	m_timeouts.clear();
}

void LuaTimer::Tick()
{
	PROFILE_SCOPED()

	assert(Pi::game);
	lua_State *l = Lua::manager->GetLuaState();

	double now = Pi::game->GetTime();

	// Move called timeouts out of the list into our scratch buffer
	while (!m_timeouts.empty() && m_timeouts.front().at <= now) {
		m_called.push_back(m_timeouts.front());
		m_timeouts.pop_front();
	}

	if (m_called.empty())
		return;

	LUA_DEBUG_START(l);

	luaL_getsubtable(l, LUA_REGISTRYINDEX, "PiTimerCallbacks");
	int callbackRegistry = lua_gettop(l);

	// Call each callback for the timeout and re-queue if appropriate
	for (CallInfo &call : m_called) {
		lua_rawgeti(l, callbackRegistry, call.callbackId);
		lua_getfield(l, -1, "callback");

		pi_lua_protected_call(l, 0, 1);
		bool cancel = lua_toboolean(l, -1);
		lua_pop(l, 1);

		if (cancel || !call.repeats) {
			// Cleanup and remove the callback info
			lua_pop(l, 1);
			luaL_unref(l, callbackRegistry, call.callbackId);
			continue;
		}

		lua_getfield(l, -1, "every");
		double every = lua_tonumber(l, -1);
		// will take into account that we could skip the appointed time,
		// but will maintain the original "alignment"
		double next = now + every - fmod(now - call.at, every);
		lua_pop(l, 1);

		lua_pushnumber(l, next); // [ cb, next ]
		lua_setfield(l, -2, "at");

		lua_pop(l, 1); // remove callback info from the stack

		Insert(next, call.callbackId, true);
	}

	// Clear the scratch buffer
	m_called.clear();
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

static int _finish_timer_create(lua_State *l, int callbackIdx)
{
	// Expects the callback data table available at the top of the stack
	lua_pushstring(l, "callback");
	lua_pushvalue(l, callbackIdx);
	lua_settable(l, -3);

	luaL_getsubtable(l, LUA_REGISTRYINDEX, "PiTimerCallbacks");
	lua_insert(l, callbackIdx);
	int ref = luaL_ref(l, callbackIdx);

	lua_pop(l, 1);
	return ref;
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

	int ref = _finish_timer_create(l, 3);
	Pi::luaTimer->Insert(at, ref, false);

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
 * >     local did_dump = Game.player:Jettison(Equipment.cargo.hydrogen)
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

	double at = Pi::game->GetTime() + every;

	lua_newtable(l);
	pi_lua_settable(l, "every", every);
	pi_lua_settable(l, "at", at);

	int ref = _finish_timer_create(l, 3);
	Pi::luaTimer->Insert(at, ref, true);

	LUA_DEBUG_END(l, 0);

	return 0;
}

template <>
const char *LuaObject<LuaTimer>::s_type = "Timer";

template <>
void LuaObject<LuaTimer>::RegisterClass()
{
	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);

	static const luaL_Reg l_methods[] = {
		{ "CallAt", l_timer_call_at },
		{ "CallEvery", l_timer_call_every },
		{ 0, 0 }
	};

	lua_getfield(l, LUA_REGISTRYINDEX, "CoreImports");
	LuaObjectBase::CreateObject(l_methods, 0, 0);
	lua_setfield(l, -2, "Timer");
	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);
}
