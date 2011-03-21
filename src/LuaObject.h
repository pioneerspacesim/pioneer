#ifndef _LUAOBJECT_H
#define _LUAOBJECT_H

#include <stdint.h>
#include <map>

#include "LuaManager.h"
#include "Object.h"

typedef uintptr_t lid;

class LuaObject {
public:
	LuaObject() : m_id(-1), m_object(NULL), m_type(NULL) {}
	LuaObject(const LuaObject &lo) : m_id(lo.m_id), m_object(lo.m_object), m_type(lo.m_type) {}
	virtual ~LuaObject() {}

	static LuaObject Lookup(lid id);

	inline lid GetId() const { return m_id; }
	inline Object *GetObject() const { return m_object; }
	inline const char *GetType() const { return m_type; }

	virtual void PushToLua() const;
	static Object *PullFromLua(lua_State *l, const char *want_type);

protected:
	LuaObject(Object *o, const char *type) : m_object(o), m_type(type) { Register(); }

	static void CreateClass(const char *type, const luaL_reg methods[], const luaL_reg meta[]);

private:
	void Register();
	void Deregister();

	static int GC(lua_State *l);

	lid         m_id;
	Object     *m_object;
	const char *m_type;
};

#endif
