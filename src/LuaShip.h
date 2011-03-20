#ifndef _LUASHIP_H
#define _LUASHIP_H

#include "LuaObject.h"
#include "Ship.h"

class LuaShip : public LuaObject {
public:
	LuaShip(Ship *s) : LuaObject(s) {}

	virtual void Push(lua_State *l);

	static void RegisterClass(lua_State *l);
};


#endif
