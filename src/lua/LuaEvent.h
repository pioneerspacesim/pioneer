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

	class LuaEvent {
	public:
		LuaEvent (const std::string_view event) :
			m_queue(GetEventQueue()), m_event(nullptr), m_args(nullptr)
		{
			m_event = new ScopedTable(m_queue.GetLua());
			m_event->Set("name", event);
		}

		LuaEvent (const LuaRef &queue, const std::string_view event) :
			m_queue(queue)
		{
			m_event = new ScopedTable(m_queue.GetLua());
			m_event->Set("name", event);
		}

		LuaEvent(const LuaEvent &) = delete;
		LuaEvent &operator=(const LuaEvent &) = delete;

		~LuaEvent()
		{
			delete m_args;
			delete m_event;
		}

		template <class Key, class Value>
		void addParameter(const Key &key, const Value &value)
		{
			if (!m_args) {
				m_args = new LuaTable(*m_event);
			}
			m_args->Set(key, value);
		}

		void enqueue()
		{
			if (!m_event) {
				return;
			}
			if (m_args) {
				m_event->PushBack(*m_args);
				delete m_args;
				m_args = nullptr;
			}
			ScopedTable(m_queue).PushBack(*m_event);
			delete m_event;
			m_event = nullptr;
		}

	private:
		const LuaRef &m_queue;
		ScopedTable *m_event;
		LuaTable *m_args;
	};
} // namespace LuaEvent

#endif
