#ifndef _LUAOBJECT_H
#define _LUAOBJECT_H

#include <stdint.h>
#include <map>

#include "LuaManager.h"
#include "DeleteEmitter.h"

typedef uintptr_t lid;

class LuaObject {
public:
	LuaObject() : m_id(-1), m_object(NULL), m_type(NULL) {}
	LuaObject(const LuaObject &lo) : m_id(lo.m_id), m_object(lo.m_object), m_type(lo.m_type), m_deleteConnection(lo.m_deleteConnection) {}
	virtual ~LuaObject() {}

	static LuaObject Lookup(lid id);

	inline lid GetId() const { return m_id; }
	inline DeleteEmitter *GetObject() const { return m_object; }
	inline const char *GetType() const { return m_type; }

	virtual void PushToLua() const;

protected:
	LuaObject(DeleteEmitter *o, const char *type) : m_object(o), m_type(type) { Register(); }

	static DeleteEmitter *PullFromLua(lua_State *l, const char *want_type);

	static void CreateClass(const char *type, const luaL_reg methods[], const luaL_reg meta[]);

private:
	void Register();
	void Deregister();

	static int GC(lua_State *l);

	lid            m_id;
	DeleteEmitter *m_object;
	const char    *m_type;

	sigc::connection m_deleteConnection;
};

template <typename T>
class LuaSubObject : public LuaObject {
public:
	LuaSubObject(T *o) : LuaObject(o, s_type) {}

	static inline void RegisterClass() {
		CreateClass(s_type, s_methods, s_meta);
	};

	static inline T *PullFromLua(lua_State *l) {
		return dynamic_cast<T *>(LuaObject::PullFromLua(l, s_type));
	}

private:
	static const char *s_type;
	static const luaL_reg s_methods[];
	static const luaL_reg s_meta[];
};

class Ship;
typedef LuaSubObject<Ship> LuaShip;

class SpaceStation;
typedef LuaSubObject<SpaceStation> LuaSpaceStation;

class StarSystem;
typedef LuaSubObject<StarSystem> LuaStarSystem;

#endif
