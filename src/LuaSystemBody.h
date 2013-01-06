// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LUASYSTEMBODY_H
#define _LUASYSTEMBODY_H

#include "LuaObject.h"
#include "galaxy/StarSystem.h"

// this specialisation clears out the copied SystemBody parent and children list,
// which right now points to other SystemBodys in a StarSystem somewhere, and will
// be deleted when the system is deleted. without this we crash when this
// object is collected/destroyed.
template <>
class LuaAcquirer< LuaUncopyable<SystemBody> > {
public:
	virtual void OnAcquire(LuaUncopyable<SystemBody> *o) {
		o->parent = 0;
		o->children.clear();
	}
	virtual void OnRelease(LuaUncopyable<SystemBody> *o) { }
};

class SystemBody;
typedef LuaObjectUncopyable<SystemBody,LuaUncopyable<SystemBody> > LuaSystemBody;

#endif
