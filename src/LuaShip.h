#ifndef _LUASHIP_H
#define _LUASHIP_H

#include "LuaObject.h"
#include "Ship.h"

class LuaShip : public LuaObject {
public:
	LuaShip(Ship *s) : LuaObject(s, s_type) {}

	static inline void RegisterClass() {
		CreateClass(s_type, s_methods, s_meta);
	};

	static inline Ship *PullFromLua(lua_State *l) {
		return dynamic_cast<Ship*>(LuaObject::PullFromLua(l, s_type));
	}

private:
	static const char *s_type;
	static const luaL_reg s_methods[];
	static const luaL_reg s_meta[];
};


#endif
