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

/*
 * Class: EventQueue
 *
 * A class to manage a queue of events.
 *
 * The majority of the work done by a Pioneer Lua module is in response to
 * events. The typical structure of a module will be to define a number of
 * event handler functions and then register them with the appropriate event
 * queues.
 *
 * When particular events occur within the game, such as a ship docking or the
 * player coming under attack, the appropriate <EventQueue> object is
 * triggered. This causes all functions registered with the event queue to be
 * called, in no particular order.
 *
 * An event usually has one or more parameters attached to it that describe
 * the details of the event. For example, the <onShipDocked> event has two
 * parameters, the <Ship> that docked, and the <SpaceStation> it docked with.
 *
 * The <EventQueue> objects are available under the global EventQueue
 * namespace.
 *
 * Event: onGameStart
 *
 * Event: onGameEnd
 *
 * Event: onEnterSystem
 *
 * Event: onLeaveSystem
 *
 * Event: onShipDestroyed
 *
 * Event: onShipHit
 *
 * Event: onShipCollided
 *
 * Event: onShipDocked
 *
 * Event: onShipUndocked
 *
 * Event: onJettison
 *
 * Event: onCreateBB
 *
 * Event: onUpdateBB
 */

/*
 * Method: Connect
 *
 * Connect a function to an event queue. When the queue emits an event, the
 * function will be called.
 *
 * > onEvent:Connect(function)
 *
 * Parameters:
 *
 *   function - function to call when the queue emits an event. The function
 *              will recieve a copy of the arguments attached to the event
 *
 *
 * Example:
 *
 * > EventQueue.onEnterSystem:Connect(function (ship)
 * >     print("welcome to "..Game.system:GetName()..", "..ship:GetLabel())
 * > end)
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  stable
 */
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

/*
 * Method: Disconnect
 *
 * Disconnects a function from an event queue. The function will no long
 * receive events emitted by the queue.
 *
 * If the function is not connected to the queue this method does nothihg.
 *
 * > onEvent:Disconnect(function)
 *
 * Parameters:
 *
 *   function - a function that was previously connected to this queue with
 *              <Connect>
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  stable
 */
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

	LuaObjectBase::CreateClass(s_type, NULL, l_methods, NULL);
}
