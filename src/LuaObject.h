#ifndef _LUAOBJECT_H
#define _LUAOBJECT_H

#include "Object.h"

#include <stdint.h>
#include <map>

class LuaObject {
protected:
	LuaObject(Object *o);
	~LuaObject();

	uint32_t m_id;

private:
	LuaObject(LuaObject &lo);

	void Register(Object *o);
	void Deregister();

	static Object *Lookup(uint32_t id);
};

#endif
