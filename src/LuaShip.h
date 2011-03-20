#ifndef _LUASHIP_H
#define _LUASHIP_H

#include "LuaObject.h"
#include "Ship.h"

class LuaShip : public LuaObject {
public:
	LuaShip(Ship *s) : LuaObject(s, GetType()) {}

	virtual const char *GetType() const;
    virtual void PushToLua();

	static void RegisterClass();
};


#endif
