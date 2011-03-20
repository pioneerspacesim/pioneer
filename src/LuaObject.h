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
