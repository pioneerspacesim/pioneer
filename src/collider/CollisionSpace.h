// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _COLLISION_SPACE
#define _COLLISION_SPACE

#include "../Aabb.h"
#include "../vector3.h"

#include <memory>
#include <vector>

class Geom;
class SingleBVHTree;

struct isect_t;
struct CollisionContact;

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
	~CollisionSpace();
	void AddGeom(Geom *);
	void RemoveGeom(Geom *);
	void AddStaticGeom(Geom *);
	void RemoveStaticGeom(Geom *);
	void TraceRay(const vector3d &start, const vector3d &dir, double len, CollisionContact *c, const Geom *ignore = nullptr);
	void Collide(void (*callback)(CollisionContact *));
	void SetSphere(const vector3d &pos, double radius, void *user_data)
	{
		sphere.pos = pos;
		sphere.radius = radius;
		sphere.userData = user_data;
	}
	void FlagRebuildObjectTrees() { m_needStaticGeomRebuild = true; }
	void RebuildObjectTrees();

	const SingleBVHTree *GetDynamicTree() const { return m_dynamicObjectTree.get(); }
	const SingleBVHTree *GetStaticTree() const { return m_staticObjectTree.get(); }

	// Geoms with the same handle will not be collision tested against each other
	// should be used for geoms that are part of the same body
	// could also be used for autopiloted groups and LRCs near stations
	// zero means ungrouped. assumes that wraparound => no old crap left
	static int GetGroupHandle()
	{
		if (!s_nextHandle) s_nextHandle++;
		return s_nextHandle++;
	}

private:
	using Intersection = std::pair<uint32_t, uint32_t>;

	void CollideRaySphere(const vector3d &start, const vector3d &dir, isect_t *isect);
	uint32_t SortEnabledGeoms(std::vector<Geom *> &geoms);
	void RebuildBVHTree(SingleBVHTree *tree, uint32_t numEnabled, const std::vector<Geom *> &geoms, std::vector<AABBd> &aabbs);

	void CollideGeom(Geom *a, Geom *b, void (*callback)(CollisionContact *));
	void CollidePlanet(void (*callback)(CollisionContact *));
	void TraceRayGeom(Geom *g, const vector3d &start, const vector3d &dir, double len, CollisionContact *c);

	std::unique_ptr<SingleBVHTree> m_staticObjectTree;
	std::unique_ptr<SingleBVHTree> m_dynamicObjectTree;

	std::vector<Geom *> m_staticGeoms;
	std::vector<Geom *> m_geoms;

	uint32_t m_enabledStaticGeoms;
	uint32_t m_enabledDynGeoms;

	std::vector<AABBd> m_geomAabbs;
	Sphere sphere;

	bool m_needStaticGeomRebuild;
	bool m_duringCollision;

	static int s_nextHandle;
};

#endif /* _COLLISION_SPACE */
