#include "LuaTimer.h"
#include "LuaUtils.h"
#include "Pi.h"

void LuaTimer::Tick()
{
	lua_State *l = Pi::luaManager.GetLuaState();

	LUA_DEBUG_START(l);

	lua_getfield(l, LUA_REGISTRYINDEX, "PiTimerCallbacks");
	if (lua_isnil(l, -1)) {
		lua_pop(l, 1);
		LUA_DEBUG_END(l, 0);
		return;
	}
	assert(lua_istable(l, -1));

	double now = Pi::GetGameTime();

	lua_pushnil(l);
	while (lua_next(l, -2)) {
		assert(lua_istable(l, -1));

		lua_getfield(l, -1, "at");
		double at = lua_tonumber(l, -1);
		lua_pop(l, 1);

		if (at <= now) {
			lua_getfield(l, -1, "callback");
			lua_call(l, 0, 1);
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

				pi_lua_settable(l, "at", Pi::GetGameTime() + every);
			}
		}

		lua_pop(l, 1);
	}
	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);
}

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
	lua_pushinteger(l, lua_objlen(l, -2) + 1);
	lua_insert(l, -2);
	lua_settable(l, -3);

	lua_pop(l, 1);
}

static int l_timer_call_at(lua_State *l)
{
	double at = luaL_checknumber(l, 2);
	if (!lua_isfunction(l, 3))
		luaL_typerror(l, 3, lua_typename(l, LUA_TFUNCTION));
	
	if (at <= Pi::GetGameTime())
		luaL_error(l, "Specified time is in the past");
	
	LUA_DEBUG_START(l);

	lua_newtable(l);
	pi_lua_settable(l, "at", at);

	_finish_timer_create(l);

	LUA_DEBUG_END(l, 0);

	return 0;
}

static int l_timer_call_every(lua_State *l)
{
	double every = luaL_checknumber(l, 2);
	if (!lua_isfunction(l, 3))
		luaL_typerror(l, 3, lua_typename(l, LUA_TFUNCTION));
	
	if (every <= 0)
		luaL_error(l, "Specified interval must be greater than zero");
	
	LUA_DEBUG_START(l);

	lua_newtable(l);
	pi_lua_settable(l, "every", every);
	pi_lua_settable(l, "at", Pi::GetGameTime() + every);

	_finish_timer_create(l);

	LUA_DEBUG_END(l, 0);

	return 0;
}

template <> const char *LuaObject<LuaTimer>::s_type = "Timer";

template <> void LuaObject<LuaTimer>::RegisterClass()
{
	static const luaL_reg l_methods[] = {
		{ "CallAt",    l_timer_call_at    },
		{ "CallEvery", l_timer_call_every },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, NULL, l_methods, NULL, NULL);
}
