#include "libs.h"
#include "LuaEventQueue.h"
#include "LuaManager.h"
#include "LuaObject.h"
#include "LuaUtils.h"
#include "Pi.h"
#include "LuaConsole.h"
#include "StringF.h"

static void _get_method_onto_stack(lua_State *l, const char *queue, const char *method) {
	LUA_DEBUG_START(l);

	int top = lua_gettop(l);

	lua_rawgeti(l, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
	lua_pushstring(l, "EventQueue");
	lua_rawget(l, -2);
	assert(lua_istable(l, -1));

	lua_pushstring(l, queue);
	lua_rawget(l, -2);
	assert(lua_istable(l, -1));

	lua_pushstring(l, method);
	lua_rawget(l, -2);
	assert(lua_isfunction(l, -1));

	lua_insert(l, top+1);
	lua_settop(l, top+1);

	LUA_DEBUG_END(l, 1);
}

void LuaEventQueueBase::ClearEvents()
{
	lua_State *l = Pi::luaManager->GetLuaState();

	LUA_DEBUG_START(l);
	_get_method_onto_stack(l, m_name, "ClearEvents");
	pi_lua_protected_call(l, 0, 0);
	LUA_DEBUG_END(l, 0);
}

void LuaEventQueueBase::Emit()
{
	lua_State *l = Pi::luaManager->GetLuaState();

	LUA_DEBUG_START(l);
	_get_method_onto_stack(l, m_name, "Emit");
	pi_lua_protected_call(l, 0, 0);
	LUA_DEBUG_END(l, 0);
}

void LuaEventQueueBase::QueueSingleEvent(const LuaEventBase &eb)
{
	lua_State *l = Pi::luaManager->GetLuaState();

	LUA_DEBUG_START(l);
	_get_method_onto_stack(l, m_name, "Queue");

	int top = lua_gettop(l);
	PrepareLuaStack(l, eb);
	pi_lua_protected_call(l, lua_gettop(l) - top, 0);

	LUA_DEBUG_END(l, 0);
}

void LuaEventQueueBase::EmitSingleEvent(const LuaEventBase &eb)
{
	lua_State *l = Pi::luaManager->GetLuaState();

	LUA_DEBUG_START(l);
	_get_method_onto_stack(l, m_name, "Signal");

	int top = lua_gettop(l);
	PrepareLuaStack(l, eb);
	pi_lua_protected_call(l, lua_gettop(l) - top, 0);

	LUA_DEBUG_END(l, 0);
}
