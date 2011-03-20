#ifndef _LUAOBJECT_H
#define _LUAOBJECT_H

#include <stdint.h>
#include <map>

#include "LuaManager.h"
#include "Object.h"

typedef uintptr_t lid;

struct lpair {
	Object *o;
	const char *type;
};

class LuaObject {
public:
	virtual const char *GetType() const = 0;

	static lpair Lookup(lid id);
	inline lid GetId() const { return m_id; }

	static void Deregister(lid id);

protected:
	LuaObject(Object *o, const char *type);
	virtual ~LuaObject() {}

	virtual void PushToLua(const char *type);

	lid m_id;

	static void CreateClass(const char *type, const luaL_reg methods[], const luaL_reg meta[]);

private:
	LuaObject(LuaObject &lo);

	void Register(Object *o, const char *type);
	void Deregister();
};

#endif
