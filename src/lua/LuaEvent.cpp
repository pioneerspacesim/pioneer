// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaEvent.h"
#include "LuaManager.h"
#include "LuaObject.h"
#include "LuaUtils.h"

#include "core/Log.h"

namespace LuaEvent {

	static LuaRef s_eventTable;

	static bool _get_method_onto_stack(lua_State *l, const char *method)
	{
		LUA_DEBUG_START(l);
		if (!s_eventTable.IsValid())
			return false;

		s_eventTable.PushCopyToStack();

		lua_getfield(l, -1, method);
		if (!lua_isfunction(l, -1)) {
			lua_pop(l, 2);
			LUA_DEBUG_END(l, 0);
			return false;
		}

		lua_replace(l, -2);
		LUA_DEBUG_END(l, 1);

		return true;
	}

	void Init()
	{
		lua_State *l = Lua::manager->GetLuaState();

		if (!pi_lua_import(l, "Event")) {
			Log::Error("Could not load lua Event queue implementation!");
			return;
		}

		s_eventTable = LuaRef(l, -1);

		lua_pop(l, 1);
	}

	void Uninit()
	{
		s_eventTable.Unref();
	}

	void Clear()
	{
		lua_State *l = Lua::manager->GetLuaState();

		LUA_DEBUG_START(l);
		if (!_get_method_onto_stack(l, "_Clear")) return;
		pi_lua_protected_call(l, 0, 0);
		LUA_DEBUG_END(l, 0);
	}

	void Emit()
	{
		lua_State *l = Lua::manager->GetLuaState();

		LUA_DEBUG_START(l);
		if (!_get_method_onto_stack(l, "_Emit")) return;
		pi_lua_protected_call(l, 0, 0);
		LUA_DEBUG_END(l, 0);
	}

	LuaRef &GetEventQueue()
	{
		return s_eventTable;
	}

} // namespace LuaEvent
