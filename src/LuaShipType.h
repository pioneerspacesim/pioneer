#ifndef _LUASHIPTYPE_H
#define _LUASHIPTYPE_H

#include "ShipType.h"

typedef LuaObjectUncopyable<ShipType,LuaUncopyable<ShipType> > LuaShipType;

#endif
