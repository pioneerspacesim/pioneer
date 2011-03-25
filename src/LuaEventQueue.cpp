#include "LuaEventQueue.h"
#include "LuaManager.h"
#include "LuaObject.h"
#include "mylua.h" // XXX for mylua_panic

void LuaEventQueueBase::RegisterEventQueue()
{
	lua_State *l = LuaManager::Instance()->GetLuaState();

	// get the eventqueue table, or create it if it doesn't exist
	lua_getfield(l, LUA_GLOBALSINDEX, "EventQueue");
	if (lua_isnil(l, -1)) {
		lua_pop(l, 1);
		lua_newtable(l);
		lua_setfield(l, LUA_GLOBALSINDEX, "EventQueue");
		lua_getfield(l, LUA_GLOBALSINDEX, "EventQueue");
	}

	lua_pushstring(l, m_name);
	LuaSubObject<LuaEventQueueBase>::PushToLua(this);
	lua_rawset(l, -3);

	lua_pushstring(l, stringf(256, "_%s_callbacks", m_name).c_str());
	lua_newtable(l);
	lua_rawset(l, -3);
}

void LuaEventQueueBase::ClearEvents()
{
	while (m_events.size()) {
		LuaEventBase *e = m_events.front();
		m_events.pop_front();
		delete e;
	}
}

void LuaEventQueueBase::Emit()
{
	lua_State *l = LuaManager::Instance()->GetLuaState();

	lua_pushcfunction(l, mylua_panic);
	int panic_idx = lua_gettop(l);

	lua_getfield(l, LUA_GLOBALSINDEX, "EventQueue");
	lua_getfield(l, -1, stringf(256, "_%s_callbacks", m_name).c_str());

	while (m_events.size()) {
		LuaEventBase *e = m_events.front();
		m_events.pop_front();

		lua_pushnil(l);
		while (lua_next(l, -2) != 0) {
			int top = lua_gettop(l);
			PrepareLuaStack(l, e);
			lua_pcall(l, lua_gettop(l) - top, 0, panic_idx);
		}
		lua_pop(l, 1);

		delete e;
	}

	lua_pop(l, 3);
}

int LuaEventQueueBase::l_connect(lua_State *l)
{
	LuaEventQueueBase *q = LuaSubObject<LuaEventQueueBase>::PullFromLua();

	lua_getfield(l, LUA_GLOBALSINDEX, "EventQueue");
	lua_getfield(l, -1, stringf(256, "_%s_callbacks", q->m_name).c_str());

	lua_pushvalue(l, 1);
	lua_pushvalue(l, 1);
	lua_rawset(l, -3);

	return 0;
}

int LuaEventQueueBase::l_disconnect(lua_State *l)
{
	LuaEventQueueBase *q = LuaSubObject<LuaEventQueueBase>::PullFromLua();

	lua_getfield(l, LUA_GLOBALSINDEX, "EventQueue");
	lua_getfield(l, -1, stringf(256, "_%s_callbacks", q->m_name).c_str());

	lua_pushvalue(l, 1);
	lua_pushnil(l);
	lua_rawset(l, -3);

	return 0;
}

template <> const char *LuaSubObject<LuaEventQueueBase>::s_type = "EventQueue";
template <> const char *LuaSubObject<LuaEventQueueBase>::s_inherit = NULL;

template <> const luaL_reg LuaSubObject<LuaEventQueueBase>::s_methods[] = {
	{ "connect",    LuaEventQueueBase::l_connect    },
	{ "disconnect", LuaEventQueueBase::l_disconnect },
	{ 0, 0 }
};

template <> const luaL_reg LuaSubObject<LuaEventQueueBase>::s_meta[] = {
	{ 0, 0 }
};
