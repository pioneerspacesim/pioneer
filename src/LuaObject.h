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

protected:
	LuaObject(Object *o);
	virtual ~LuaObject();

	virtual void PushToLua(const char *name);

	lid m_id;

	static void CreateClass(const char *name, const luaL_reg methods[], const luaL_reg meta[]);

private:
	LuaObject(LuaObject &lo);

	void Register(Object *o);
	void Deregister();
};

#endif
