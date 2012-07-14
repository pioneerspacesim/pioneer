#ifndef _COLLISION_SPACE
#define _COLLISION_SPACE

#include <list>
#include "../vector3.h"

class Geom;
struct isect_t;
struct CollisionContact;

struct Sphere {
	vector3d pos;
	double radius;
	void *userData;
};

class BvhTree;

/*
 * Collision spaces have a bunch of geoms and at most one sphere (for a planet).
 */
class CollisionSpace {
public:
	CollisionSpace();
	~CollisionSpace();
	void AddGeom(Geom*);
	void RemoveGeom(Geom*);
	void AddStaticGeom(Geom*);
	void RemoveStaticGeom(Geom*);
	void TraceRay(const vector3d &start, const vector3d &dir, double len, CollisionContact *c, Geom *ignore = 0);
	void Collide(void (*callback)(CollisionContact*));
	void SetSphere(const vector3d &pos, double radius, void *user_data) {
		sphere.pos = pos; sphere.radius = radius; sphere.userData = user_data;
	}
	void FlagRebuildObjectTrees() { m_needStaticGeomRebuild = true; }
	void RebuildObjectTrees();
private:
	void CollideGeoms(Geom *a, int minMailboxValue, void (*callback)(CollisionContact*));
	void CollideRaySphere(const vector3d &start, const vector3d &dir, isect_t *isect);
	std::list<Geom*> m_geoms;
	std::list<Geom*> m_staticGeoms;
	bool m_needStaticGeomRebuild;
	BvhTree *m_staticObjectTree;
	BvhTree *m_dynamicObjectTree;
	Sphere sphere;
};

#endif /* _COLLISION_SPACE */
