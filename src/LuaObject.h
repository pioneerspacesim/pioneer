#ifndef _LUAOBJECT_H
#define _LUAOBJECT_H

#include <stdint.h>
#include <map>

#include "LuaManager.h"
#include "Object.h"

typedef uintptr_t lid;

class LuaObject {
public:
	static Object *Lookup(lid id);
	inline lid GetId() const { return m_id; }

	virtual void Push(lua_State *l) = 0;

protected:
	LuaObject(Object *o);
	virtual ~LuaObject();

	lid m_id;

	static void RegisterClass() {};

private:
	LuaObject(LuaObject &lo);

	void Register(Object *o);
	void Deregister();
};

#endif
