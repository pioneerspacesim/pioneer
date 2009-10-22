#ifndef _OBJECT_H
#define _OBJECT_H

class Object {
	public:
	enum Type { OBJECT, BODY, MODELBODY, DYNAMICBODY, SHIP, PLAYER, SPACESTATION, PLANET, STAR, CARGOBODY, CITYONPLANET, PROJECTILE };
	virtual Type GetType() { return OBJECT; }
	virtual bool IsType(Type c) { return GetType() == c; }
};
#define OBJDEF(__thisClass,__parentClass,__TYPE) \
	virtual Object::Type GetType() { return Object::__TYPE; } \
	virtual bool IsType(Type c) { \
	if (__thisClass::GetType() == (c)) return true; \
	else return __parentClass::IsType(c); }
#endif /* _OBJECT_H */
