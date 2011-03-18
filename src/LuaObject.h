#ifndef _LUAOBJECT_H
#define _LUAOBJECT_H

#include <stdint.h>
#include <map>

#include "LuaManager.h"
#include "Object.h"

class LuaObject {
	virtual void RegisterClass() {};

protected:
	LuaObject(Object *o);
	virtual ~LuaObject();

	uint32_t m_id;

private:
	LuaObject(LuaObject &lo);

	void Register(Object *o);
	void Deregister();

	static Object *Lookup(uint32_t id);
};

#endif
