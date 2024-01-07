// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LUAEVENT_H
#define _LUAEVENT_H

#include "Lua.h"
#include "LuaObject.h"
#include "LuaPushPull.h"
#include "LuaTable.h"

namespace LuaEvent {

	void Init();
	void Uninit();

	void Clear();
	void Emit();

	LuaRef &GetEventQueue();

	// Push an event to the specified event queue, passed as a LuaRef &
	template <typename... TArgs>
	inline void Queue(const LuaRef &queue, std::string_view event, TArgs... args)
	{
		ScopedTable ev(queue.GetLua());
		ev.Set("name", event);
		ev.PushMultiple(args...);

		ScopedTable(queue).PushBack(ev);
	}

	// Push an event to the specified event queue, passed as a LuaRef &
	inline void Queue(const LuaRef &queue, std::string_view event)
	{
		ScopedTable ev(queue.GetLua());
		ev.Set("name", event);

		ScopedTable(queue).PushBack(ev);
	}

	// Push an event to the specified event queue, passed as a LuaRef &
	template <typename... TArgs>
	inline void Queue(std::string_view event, TArgs... args)
	{
		Queue(GetEventQueue(), event, args...);
	}

	// Push an event to the specified event queue, passed as a LuaRef &
	inline void Queue(std::string_view event)
	{
		Queue(GetEventQueue(), event);
	}

} // namespace LuaEvent

#endif
