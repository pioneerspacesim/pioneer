#ifndef _LUASBODY_H
#define _LUASBODY_H

#include "LuaObject.h"
#include "StarSystem.h"

// this specialisation clears out the copied SBody children list, which right
// now points to other SBodys in a StarSystem somewhere, and will be deleted
// when the system is deleted. without this we crash when this object is
// collected/destroyed.
template <>
class LuaAcquirer< LuaUncopyable<SBody> > {
public:
	virtual void Acquire(LuaUncopyable<SBody> *o) {
		o->children.clear();
	}
	virtual void Release(LuaUncopyable<SBody> *o) { }
};

class SBody;
typedef LuaObjectUncopyable<SBody,LuaUncopyable<SBody> > LuaSBody;

#endif
