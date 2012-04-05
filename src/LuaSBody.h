#ifndef _LUASBODY_H
#define _LUASBODY_H

#include "LuaObject.h"
#include "galaxy/StarSystem.h"

// this specialisation clears out the copied SBody parent and children list,
// which right now points to other SBodys in a StarSystem somewhere, and will
// be deleted when the system is deleted. without this we crash when this
// object is collected/destroyed.
template <>
class LuaAcquirer< LuaUncopyable<SBody> > {
public:
	virtual void OnAcquire(LuaUncopyable<SBody> *o) {
		o->parent = 0;
		o->children.clear();
	}
	virtual void OnRelease(LuaUncopyable<SBody> *o) { }
};

class SBody;
typedef LuaObjectUncopyable<SBody,LuaUncopyable<SBody> > LuaSBody;

#endif
