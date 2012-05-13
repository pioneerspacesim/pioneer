#ifndef _LUASTARSYSTEM_H
#define _LUASTARSYSTEM_H

#include "LuaObject.h"
#include "galaxy/StarSystem.h"

// this is a specialisation for the starsystem acquirer. it modifies the
// refcount so that it doesn't get removed from the system cache while we're
// using it
template <>
class LuaAcquirer<StarSystem> {
public:
	virtual void OnAcquire(StarSystem *o) {
		o->IncRefCount();
	}
	virtual void OnRelease(StarSystem *o) {
		o->DecRefCount();
	}
};

typedef LuaObject<StarSystem> LuaStarSystem;

#endif
