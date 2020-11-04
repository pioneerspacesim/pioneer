// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _OBJECT_H
#define _OBJECT_H

struct Object {
	// only creating enum strings for types that are exposed to Lua
	enum Type { // <enum scope='Object' name=PhysicsObjectType public>
		BODY,
		MODELBODY,
		DYNAMICBODY, // <enum skip>
		SHIP,
		PLAYER,
		SPACESTATION,
		TERRAINBODY, // <enum skip>
		PLANET,
		STAR,
		CARGOBODY,
		PROJECTILE, // <enum skip>
		MISSILE,
		HYPERSPACECLOUD // <enum skip>
	};
};

#define OBJDEF(__thisClass, __parentClass, __TYPE)                           \
	virtual Object::Type GetType() const override { return Object::__TYPE; } \
	virtual bool IsType(Object::Type c) const override                       \
	{                                                                        \
		if (__thisClass::GetType() == (c))                                   \
			return true;                                                     \
		else                                                                 \
			return __parentClass::IsType(c);                                 \
	}

#endif /* _OBJECT_H */
