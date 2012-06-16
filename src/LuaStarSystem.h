#ifndef _LUASTARSYSTEM_H
#define _LUASTARSYSTEM_H

#include "LuaObject.h"
#include "galaxy/StarSystem.h"

template <> class LuaAcquirer<StarSystem> : public LuaAcquirerRefCounted {};
typedef LuaObject<StarSystem> LuaStarSystem;

#endif
