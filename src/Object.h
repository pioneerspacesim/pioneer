#ifndef _OBJECT_H
#define _OBJECT_H

class Object {
	public:
	enum Type { NONE, BODY, SHIP, SPACESTATION, LASER, GEOM, PLANET };
	virtual Type GetType() = 0;
};

#endif /* _OBJECT_H */
