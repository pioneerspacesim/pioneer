#ifndef _COLLISION_SPACE
#define _COLLISION_SPACE

#include <list>
#include "../vector3.h"

class Geom;
struct isect_t;
class CollisionContact;

struct Sphere {
	vector3d pos;
	double radius;
	void *userData;
};
/*
 * Collision spaces have a bunch of geoms and at most one sphere (for a planet).
 */
class CollisionSpace {
public:
	CollisionSpace();
	void AddGeom(Geom*);
	void RemoveGeom(Geom*);
	void TraceRay(const vector3d &start, const vector3d &dir, isect_t *isect);
	void Collide(void (*callback)(CollisionContact*));
	void SetSphere(const vector3d &pos, double radius, void *user_data) {
		sphere.pos = pos; sphere.radius = radius; sphere.userData = user_data;
	}
private:
	void CollideGeoms(Geom *a);
	void CollideRaySphere(const vector3d &start, const vector3d &dir, isect_t *isect);
	std::list<Geom*> m_geoms;
	Sphere sphere;
};

#endif /* _COLLISION_SPACE */
