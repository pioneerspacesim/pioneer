#include "libs.h"
#include "LuaEventQueue.h"
#include "LuaManager.h"
#include "LuaObject.h"
#include "LuaUtils.h"
#include "Pi.h"

void LuaEventQueueBase::RegisterEventQueue()
{
	lua_State *l = Pi::luaManager.GetLuaState();

	LUA_DEBUG_START(l);

	// get the eventqueue table, or create it if it doesn't exist
	lua_getfield(l, LUA_GLOBALSINDEX, "EventQueue");
	if (lua_isnil(l, -1)) {
		lua_pop(l, 1);
		lua_newtable(l);
		lua_pushvalue(l, -1);
		lua_setfield(l, LUA_GLOBALSINDEX, "EventQueue");
	}

	lua_pushstring(l, m_name);
	LuaObject<LuaEventQueueBase>::PushToLua(this);
	lua_rawset(l, -3);

	lua_pop(l, 1);

	lua_getfield(l, LUA_REGISTRYINDEX, "PiEventQueue");
	if (lua_isnil(l, -1)) {
		lua_pop(l, 1);
		lua_newtable(l);
		lua_setfield(l, LUA_REGISTRYINDEX, "PiEventQueue");
		lua_getfield(l, LUA_REGISTRYINDEX, "PiEventQueue");
	}

	lua_pushstring(l, m_name);
	lua_newtable(l);
	lua_rawset(l, -3);

	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);
}

void LuaEventQueueBase::ClearEvents()
{
	while (m_events.size()) {
		LuaEventBase *e = m_events.front();
		m_events.pop_front();
		delete e;
	}
}

void LuaEventQueueBase::EmitSingleEvent(LuaEventBase *e)
{
	lua_State *l = Pi::luaManager.GetLuaState();

	LUA_DEBUG_START(l);

	lua_getfield(l, LUA_REGISTRYINDEX, "PiEventQueue");
	assert(lua_istable(l, -1));
	lua_getfield(l, -1, m_name);
	assert(lua_istable(l, -1));

	lua_pushnil(l);
	while (lua_next(l, -2) != 0) {
		int top = lua_gettop(l);
		PrepareLuaStack(l, e);
		lua_call(l, lua_gettop(l) - top, 0);
	}

	lua_pop(l, 2);

	LUA_DEBUG_END(l, 0);

	delete e;
}

void LuaEventQueueBase::Emit()
{
	if (!m_events.size()) return;

	lua_State *l = Pi::luaManager.GetLuaState();

	LUA_DEBUG_START(l);

	lua_getfield(l, LUA_REGISTRYINDEX, "PiEventQueue");
	assert(lua_istable(l, -1));
	lua_getfield(l, -1, m_name);
	assert(lua_istable(l, -1));

	while (m_events.size()) {
		LuaEventBase *e = m_events.front();
		m_events.pop_front();

		lua_pushnil(l);
		while (lua_next(l, -2) != 0) {
			int top = lua_gettop(l);
			PrepareLuaStack(l, e);
			lua_call(l, lua_gettop(l) - top, 0);
		}

		delete e;
	}

	lua_pop(l, 2);

	LUA_DEBUG_END(l, 0);
}

int LuaEventQueueBase::l_connect(lua_State *l)
{
	LUA_DEBUG_START(l);

	LuaEventQueueBase *q = LuaObject<LuaEventQueueBase>::GetFromLua(1);

	if (!lua_isfunction(l, 2))
		luaL_typerror(l, 2, lua_typename(l, LUA_TFUNCTION));

	lua_getfield(l, LUA_REGISTRYINDEX, "PiEventQueue");
	lua_getfield(l, -1, q->m_name);

	lua_pushvalue(l, 2);
	lua_pushvalue(l, 2);
	lua_rawset(l, -3);

	lua_pop(l, 2);

	LUA_DEBUG_END(l, 0);

	return 0;
}

int LuaEventQueueBase::l_disconnect(lua_State *l)
{
	LUA_DEBUG_START(l);

	LuaEventQueueBase *q = LuaObject<LuaEventQueueBase>::GetFromLua(1);

	if (!lua_isfunction(l, 2))
		luaL_typerror(l, 2, lua_typename(l, LUA_TFUNCTION));

	lua_getfield(l, LUA_REGISTRYINDEX, "PiEventQueue");
	lua_getfield(l, -1, q->m_name);

	lua_pushvalue(l, 2);
	lua_pushnil(l);
	lua_rawset(l, -3);

	lua_pop(l, 2);

	LUA_DEBUG_END(l, 0);

	return 0;
}

template <> const char *LuaObject<LuaEventQueueBase>::s_type = "EventQueue";

template <> void LuaObject<LuaEventQueueBase>::RegisterClass()
{
	static const luaL_reg l_methods[] = {
		{ "Connect",    LuaEventQueueBase::l_connect    },
		{ "Disconnect", LuaEventQueueBase::l_disconnect },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, NULL, l_methods, NULL, NULL);
}
