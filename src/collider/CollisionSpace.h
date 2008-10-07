#ifndef _COLLISION_SPACE
#define _COLLISION_SPACE

#include <list>
#include "../vector3.h"

class Geom;
struct isect_t;
class CollisionContact;

class CollisionSpace {
public:
	CollisionSpace();
	void AddGeom(Geom*);
	void RemoveGeom(Geom*);
	void TraceRay(const vector3d &start, const vector3d &dir, isect_t *isect);
	void Collide(void (*callback)(CollisionContact*));
private:
	void CollideGeoms(Geom *a, Geom *b, void (*callback)(CollisionContact*));
	std::list<Geom*> m_geoms;
	
};

#endif /* _COLLISION_SPACE */
