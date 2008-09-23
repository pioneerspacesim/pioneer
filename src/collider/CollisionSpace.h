#ifndef _COLLISION_SPACE
#define _COLLISION_SPACE

#include <list>
#include "../vector3.h"

class Geom;
struct isect_t;

class CollisionSpace {
public:
	CollisionSpace();
	void AddGeom(Geom*);
	void RemoveGeom(Geom*);
	void TraceRay(const vector3d &start, const vector3d &dir, isect_t *isect);
private:
	std::list<Geom*> m_geoms;
	
};

#endif /* _COLLISION_SPACE */
