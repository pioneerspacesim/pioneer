// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _COLLISION_SPACE
#define _COLLISION_SPACE

#include "../vector3.h"
#include "../Aabb.h"

#include <list>
#include <vector>
#include <memory>

class Geom;
struct isect_t;
struct CollisionContact;

struct Sphere {
	vector3d pos;
	double radius;
	void *userData;
};

class BvhTree;
class SingleBVHTree;

/*
 * Collision spaces have a bunch of geoms and at most one sphere (for a planet).
 */
// FIXME: CollisionSpace is NOT MOVE/COPY CONSTRUCTIBLE once RebuildObjectTrees
// has been called. It will cause a double-free due to owning pointers being moved-by-copy.
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

	// Geoms with the same handle will not be collision tested against each other
	// should be used for geoms that are part of the same body
	// could also be used for autopiloted groups and LRCs near stations
	// zero means ungrouped. assumes that wraparound => no old crap left
	static int GetGroupHandle()
	{
		if (!s_nextHandle) s_nextHandle++;
		return s_nextHandle++;
	}

	double GetDynamicTreeSAH() const { return CalcSAH(m_dynamicObjectTree); }
	double GetDynamicTree2SAH() const  { return CalcSAH(m_dynamicObjectTree2.get()); }

	double GetStaticTreeSAH() const  { return CalcSAH(m_staticObjectTree); }
	double GetStaticTree2SAH() const  { return CalcSAH(m_staticObjectTree2.get()); }

private:
	using Intersection = std::pair<uint32_t, uint32_t>;

	void CollideGeoms(Geom *a, int minMailboxValue, void (*callback)(CollisionContact *));
	void CollideRaySphere(const vector3d &start, const vector3d &dir, isect_t *isect);
	std::list<Geom *> m_geoms;
	std::list<Geom *> m_staticGeoms;
	bool m_needStaticGeomRebuild;
	BvhTree *m_staticObjectTree;
	BvhTree *m_dynamicObjectTree;
	Sphere sphere;

	double CalcSAH(BvhTree *tree) const;
	double CalcSAH(SingleBVHTree *tree) const;

	uint32_t SortEnabledGeoms(std::vector<Geom *> &geoms);
	void RebuildBVHTree(SingleBVHTree *tree, uint32_t numEnabled, const std::vector<Geom *> &geoms, std::vector<AABBd> &aabbs);

	void CollideGeom(Geom *a, Geom *b, void (*callback)(CollisionContact *));
	void CollidePlanet(void (*callback)(CollisionContact *));

	std::unique_ptr<SingleBVHTree> m_staticObjectTree2;
	std::unique_ptr<SingleBVHTree> m_dynamicObjectTree2;

	uint32_t m_enabledStaticGeoms;
	uint32_t m_enabledDynGeoms;
	std::vector<Geom *> m_staticGeomVec;
	std::vector<Geom *> m_geomVec;

	std::vector<AABBd> m_geomAabbs;
	std::vector<AABBd> m_staticGeomAabbs;

	uint32_t m_oldGeomsNumber;
	bool m_duringCollision;

	static int s_nextHandle;
};

#endif /* _COLLISION_SPACE */
