#ifndef _LUAOBJECT_H
#define _LUAOBJECT_H

#include <stdint.h>

#include "LuaManager.h"
#include "DeleteEmitter.h"

typedef uintptr_t lid;

class LuaObject {
public:
	virtual ~LuaObject() {}

	static LuaObject *Lookup(lid id);

	inline lid GetId() const { return m_id; }
	inline DeleteEmitter *GetObject() const { return m_object; }
	inline const char *GetType() const { return m_type; }

protected:
	LuaObject(DeleteEmitter *o, const char *type, bool wantdelete) : m_object(o), m_type(type), m_wantDelete(wantdelete) { Register(this); }

	static DeleteEmitter *PullFromLua(const char *want_type);

	static void CreateClass(const char *type, const luaL_reg methods[], const luaL_reg meta[]);

	static void PushToLua(LuaObject *lo);

private:
	LuaObject(const LuaObject &) {}

	static void Register(LuaObject *lo);
	static void Deregister(LuaObject *lo);

	static int GC(lua_State *l);

	lid            m_id;
	DeleteEmitter *m_object;
	const char    *m_type;

	bool           m_wantDelete;

	sigc::connection m_deleteConnection;
};

template <typename T>
class LuaSubObject : public LuaObject {
public:
	static inline void RegisterClass() {
		CreateClass(s_type, s_methods, s_meta);
	};

	static inline LuaObject *PushToLua(T *o) {
		LuaSubObject *lo = new LuaSubObject(o, false);
		LuaObject::PushToLua(lo);
		return lo;
	}

	static inline LuaObject *PushToLuaGC(T *o) {
		LuaSubObject *lo = new LuaSubObject(o, true);
		LuaObject::PushToLua(lo);
		return lo;
	}

	static inline T *PullFromLua() {
		return dynamic_cast<T *>(LuaObject::PullFromLua(s_type));
	}

private:
	LuaSubObject(T *o, bool wantdelete) : LuaObject(o, s_type, wantdelete) {}

	static const char *s_type;
	static const luaL_reg s_methods[];
	static const luaL_reg s_meta[];
};

// fake push/pull stuff for primitive types, for consistency
namespace LuaString {
	inline void PushToLua(const char *s) {
		assert(s);
		lua_State *l = LuaManager::Instance()->GetLuaState();
		lua_pushstring(l, s);
	}
	inline const char *PullFromLua() {
		lua_State *l = LuaManager::Instance()->GetLuaState();
		const char *s = luaL_checkstring(l, 1);
		lua_remove(l, 1);
		return s;
	}
};

namespace LuaFloat {
	inline void PushToLua(double n) {
		lua_State *l = LuaManager::Instance()->GetLuaState();
		lua_pushnumber(l, n);
	}
	inline double PullFromLua() {
		lua_State *l = LuaManager::Instance()->GetLuaState();
		double n = luaL_checknumber(l, 1);
		lua_remove(l, 1);
		return n;
	}
};

namespace LuaInt {
	inline void PushToLua(int n) {
		lua_State *l = LuaManager::Instance()->GetLuaState();
		lua_pushinteger(l, n);
	}
	inline int PullFromLua() {
		lua_State *l = LuaManager::Instance()->GetLuaState();
		int n = luaL_checkinteger(l, 1);
		lua_remove(l, 1);
		return n;
	}
};

class Ship;
typedef LuaSubObject<Ship> LuaShip;

class SpaceStation;
typedef LuaSubObject<SpaceStation> LuaSpaceStation;

class Planet;
typedef LuaSubObject<Planet> LuaPlanet;

class Star;
typedef LuaSubObject<Star> LuaStar;

class StarSystem;
typedef LuaSubObject<StarSystem> LuaStarSystem;

class SBodyPath;
typedef LuaSubObject<SBodyPath> LuaSBodyPath;

#endif
