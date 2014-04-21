// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _OBJECT_H
#define _OBJECT_H

#include "DeleteEmitter.h"
#include "libs.h"

class Object : public DeleteEmitter {
public:
	// only creating enum strings for types that are exposed to Lua
	enum Type { // <enum scope='Object' name=PhysicsObjectType public>
		OBJECT,         // <enum skip>
		BODY,
		MODELBODY,
		DYNAMICBODY,    // <enum skip>
		SHIP,
		PLAYER,
		SPACESTATION,
		TERRAINBODY,    // <enum skip>
		PLANET,
		STAR,
		CARGOBODY,
		CITYONPLANET,   // <enum skip>
		PROJECTILE,     // <enum skip>
		MISSILE,
		HYPERSPACECLOUD // <enum skip>
	};
	virtual Type GetType() const { return OBJECT; }
	virtual bool IsType(Type c) const { return GetType() == c; }
};
#define OBJDEF(__thisClass,__parentClass,__TYPE) \
	virtual Object::Type GetType() const { return Object::__TYPE; } \
	virtual bool IsType(Type c) const { \
	if (__thisClass::GetType() == (c)) return true; \
	else return __parentClass::IsType(c); }
#endif /* _OBJECT_H */
