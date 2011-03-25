#ifndef _LUAEVENTQUEUE_H
#define _LUAEVENTQUEUE_H

#include "LuaManager.h"
#include "LuaObject.h"
#include "DeleteEmitter.h"

#include <list>

class LuaEventBase { };

template <typename T0>
class LuaEvent : public LuaEventBase {
public:
	LuaEvent(T0 *o) : m_arg0(o) { }
	
	T0 *m_arg0;
};

class LuaEventQueueBase : public DeleteEmitter {
	friend class LuaSubObject<LuaEventQueueBase>;

public:
	void Emit();

protected:
	LuaEventQueueBase(const char *name);

	const char *m_name;

	std::list<LuaEventBase*> m_events;

private:
	static int l_connect(lua_State *l);
	static int l_disconnect(lua_State *l);

	virtual void InvokeLuaCallback(lua_State *l, const LuaEventBase *eb, int panic_idx) = 0;
};

template <typename T0>
class LuaEventQueue : public LuaEventQueueBase {
public:
	LuaEventQueue(const char *name) : LuaEventQueueBase(name) { }

	inline void Queue(T0 *o) {
		m_events.push_back(new LuaEvent<T0>(o));
	}

protected:
	inline void InvokeLuaCallback(lua_State *l, const LuaEventBase *eb, int panic_idx) {
		const LuaEvent<T0> *e = static_cast<const LuaEvent<T0>*>(eb);
		LuaSubObject<T0>::PushToLua(e->m_arg0);
		lua_pcall(l, 1, 0, panic_idx);
	}
};

#endif
