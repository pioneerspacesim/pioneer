#ifndef _LUAOBJECT_H
#define _LUAOBJECT_H

#include <stdint.h>
#include <map>

#include "LuaManager.h"
#include "Object.h"

typedef uintptr_t lid;

class LuaObject {
protected:
	LuaObject(Object *o);
	virtual ~LuaObject();

	static void RegisterClass() {};

	lid m_id;

private:
	LuaObject(LuaObject &lo);

	void Register(Object *o);
	void Deregister();

	static Object *Lookup(lid id);
};

#endif
