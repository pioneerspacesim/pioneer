// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "CollisionSpace.h"

#include "BVHTree.h"
#include "CollisionContact.h"
#include "Geom.h"
#include "GeomTree.h"
#include "profiler/Profiler.h"

#include <algorithm>

int CollisionSpace::s_nextHandle = 1;

CollisionSpace::CollisionSpace() :
	m_staticObjectTree(new SingleBVHTree()),
	m_dynamicObjectTree(new SingleBVHTree()),
	m_enabledStaticGeoms(0),
	m_enabledDynGeoms(0),
	m_needStaticGeomRebuild(true),
	m_duringCollision(false)
{
	sphere.radius = 0;
}

CollisionSpace::~CollisionSpace()
{
}

void CollisionSpace::AddGeom(Geom *geom)
{
	PROFILE_SCOPED()
	assert(!m_duringCollision);

	m_geoms.push_back(geom);
}

void CollisionSpace::RemoveGeom(Geom *geom)
{
	PROFILE_SCOPED()
	assert(!m_duringCollision);

	auto iter = std::find(m_geoms.begin(), m_geoms.end(), geom);
	if (iter == m_geoms.end())
		return;

	if (m_geoms.size() > 1)
		std::swap(*iter, m_geoms.back());
	m_geoms.pop_back();
}

void CollisionSpace::AddStaticGeom(Geom *geom)
{
	PROFILE_SCOPED()
	assert(!m_duringCollision);

	m_staticGeoms.push_back(geom);
	m_needStaticGeomRebuild = true;
}

void CollisionSpace::RemoveStaticGeom(Geom *geom)
{
	PROFILE_SCOPED()
	assert(!m_duringCollision);

	m_needStaticGeomRebuild = true;

	auto iter = std::find(m_staticGeoms.begin(), m_staticGeoms.end(), geom);
	if (iter == m_staticGeoms.end())
		return;

	if (m_staticGeoms.size() > 1)
		std::swap(*iter, m_staticGeoms.back());
	m_staticGeoms.pop_back();
}

void CollisionSpace::CollideRaySphere(const vector3d &start, const vector3d &dir, isect_t *isect)
{
	PROFILE_SCOPED()
	if (sphere.radius > 0.0) {
		/* Collide with lovely sphere! */
		const vector3d v = start - sphere.pos;
		const double b = -v.Dot(dir);
		double det = (b * b) - v.LengthSqr() + (sphere.radius * sphere.radius);
		if (det > 0) {
			det = sqrt(det);
			const double i1 = b - det;
			const double i2 = b + det;
			if (i2 > 0) {
				/*if (i1 < 0) {
					if (i2 < *dist) {
						*dist = i2;
						//retval = INPRIM;
						retval = true;
					}
				}*/
				if (i1 > 0) {
					if (i1 < isect->dist) {
						isect->dist = float(i1);
						isect->triIdx = 0;
					}
				}
			}
		}
	}
}

void CollisionSpace::TraceRay(const vector3d &start, const vector3d &dir, double len, CollisionContact *c, const Geom *ignore /*= nullptr*/)
{
	PROFILE_SCOPED()
	vector3d invDir(1.0 / dir.x, 1.0 / dir.y, 1.0 / dir.z);
	c->distance = len;

	std::vector<uint32_t> isect_result;
	isect_result.reserve(8);

	if (m_enabledStaticGeoms > 0) {
		m_staticObjectTree->TraceRay(start, invDir, len, isect_result);

		for (uint32_t &idx : isect_result) {
			Geom *g = m_staticGeoms[idx];
			TraceRayGeom(g, start, dir, len, c);
		}

		isect_result.clear();
	}

	if (m_enabledDynGeoms > 0) {
		m_dynamicObjectTree->TraceRay(start, invDir, len, isect_result);

		for (uint32_t &idx : isect_result) {
			Geom *g = m_geoms[idx];

			if (g != ignore)
				TraceRayGeom(g, start, dir, len, c);
		}
	}

	{
		isect_t isect;
		isect.dist = float(c->distance);
		isect.triIdx = -1;
		CollideRaySphere(start, dir, &isect);
		if (isect.triIdx != -1) {
			c->pos = start + dir * double(isect.dist);
			c->normal = vector3d(0.0);
			c->depth = len - isect.dist;
			c->triIdx = -1;
			c->userData1 = sphere.userData;
			c->userData2 = 0;
			c->geomFlag = 0;
			c->distance = isect.dist;
		}
	}
}

void CollisionSpace::TraceRayGeom(Geom *g, const vector3d &start, const vector3d &dir, double len, CollisionContact *c)
{
	PROFILE_SCOPED()

	const matrix4x4d &invTrans = g->GetInvTransform();
	vector3d ms = invTrans * start;
	vector3d md = invTrans.ApplyRotationOnly(dir);
	vector3f modelStart = vector3f(ms);
	vector3f modelDir = vector3f(md);

	isect_t isect;
	isect.dist = float(c->distance);
	isect.triIdx = -1;
	g->GetGeomTree()->TraceRay(modelStart, modelDir, &isect);
	if (isect.triIdx != -1) {
		c->pos = start + dir * double(isect.dist);

		vector3f n = g->GetGeomTree()->GetTriNormal(isect.triIdx);
		c->normal = vector3d(n.x, n.y, n.z);
		c->normal = g->GetTransform().ApplyRotationOnly(c->normal);

		c->depth = len - isect.dist;
		c->triIdx = isect.triIdx;
		c->userData1 = g->GetUserData();
		c->userData2 = 0;
		c->geomFlag = g->GetGeomTree()->GetTriFlag(isect.triIdx);
		c->distance = isect.dist;
	}
}

void CollisionSpace::CollideGeom(Geom *a, Geom *b, void (*callback)(CollisionContact *))
{
	PROFILE_SCOPED()

	if (!a->IsEnabled() || !b->IsEnabled())
		return;

	if (a->GetGroup() && a->GetGroup() == b->GetGroup())
		return;

	vector3d pos1 = a->GetPosition();
	vector3d pos2 = b->GetPosition();
	double r1 = a->GetGeomTree()->GetRadius();
	double r2 = b->GetGeomTree()->GetRadius();

	if ((pos1 - pos2).Length() <= (r1 + r2)) {
		a->Collide(b, callback);
	}
}

// ===================================================================

void CollisionSpace::RebuildObjectTrees()
{
	PROFILE_SCOPED()

	if (m_needStaticGeomRebuild) {
		std::vector<AABBd> staticAabbs;

		m_enabledStaticGeoms = SortEnabledGeoms(m_staticGeoms);
		RebuildBVHTree(m_staticObjectTree.get(), m_enabledStaticGeoms, m_staticGeoms, staticAabbs);
		m_needStaticGeomRebuild = false;
	}

	// NOTE: we store AABBs in m_geomAabbs for fast O(1) lookup during Collide()
	// This doubles the memory cost but allows SingleBVHTree to store leaf nodes
	// in a cache-friendly order.
	m_enabledDynGeoms = SortEnabledGeoms(m_geoms);
	RebuildBVHTree(m_dynamicObjectTree.get(), m_enabledDynGeoms, m_geoms, m_geomAabbs);
}

uint32_t CollisionSpace::SortEnabledGeoms(std::vector<Geom *> &geoms)
{
	PROFILE_SCOPED()

	if (geoms.empty())
		return 0;

	// Simple O(n) sort algorithm
	// Sort geoms according to enabled state (group all enabled geoms at start of array)
	uint32_t startIdx = 0;
	uint32_t endIdx = geoms.size();

	while (startIdx < endIdx) {
		if (geoms[startIdx]->IsEnabled()) {
			startIdx++;
		} else {
			endIdx--;
			std::swap(geoms[startIdx], geoms[endIdx]);
		}
	}

	return startIdx;
}

void CollisionSpace::RebuildBVHTree(SingleBVHTree *tree, uint32_t numGeoms, const std::vector<Geom *> &geoms, std::vector<AABBd> &aabbs)
{
	PROFILE_SCOPED()
	assert(numGeoms < INT32_MAX);

	if (numGeoms == 0) {
		tree->Clear();
		return;
	}

	aabbs.resize(0);
	aabbs.reserve(numGeoms);

	AABBd bounds = AABBd();

	for (size_t idx = 0; idx < numGeoms; idx++) {
		const Geom *geom = geoms[idx];

		// treat geoms as spheres and create a worst-case AABB around them
		vector3d pos = geom->GetPosition();
		double radius = geom->GetGeomTree()->GetRadius();

		AABBd aabb{ { pos - radius }, { pos + radius } };

		aabbs.push_back(aabb);
		bounds.Update(aabb);
	}

	tree->Build(bounds, aabbs.data(), aabbs.size());
}

void CollisionSpace::Collide(void (*callback)(CollisionContact *))
{
	PROFILE_SCOPED()
	m_duringCollision = true;

	RebuildObjectTrees();

	std::vector<Intersection> static_isect;
	std::vector<Intersection> dyn_isect;

	static_isect.reserve(32);
	dyn_isect.reserve(32);

	// Testing against an empty tree (one "invalid" node) is still slower than
	// a static branch (remains uniformly predictable across the entire loop)
	bool hasStatic = m_enabledStaticGeoms > 0;

	for (uint32_t idx = 0; idx < m_enabledDynGeoms; idx++) {
		Geom *g = m_geoms[idx];
		if (!g->IsEnabled())
			continue;

		const AABBd &aabb = m_geomAabbs[idx];

		if (hasStatic)
			m_staticObjectTree->ComputeOverlap(idx, aabb, static_isect);
		m_dynamicObjectTree->ComputeOverlap(idx, aabb, dyn_isect);
	}

	// No mailbox test needed for colliding a dynamic geom against static geoms
	for (const Intersection &isect : static_isect) {
		CollideGeom(m_geoms[isect.first], m_staticGeoms[isect.second], callback);
	}

	// Simple mailbox test to ensure every valid collision is only processed once
	for (const Intersection &isect : dyn_isect) {
		if (isect.first >= isect.second)
			continue;

		CollideGeom(m_geoms[isect.first], m_geoms[isect.second], callback);
	}

	CollidePlanet(callback);

	m_duringCollision = false;
}

void CollisionSpace::CollidePlanet(void (*callback)(CollisionContact *))
{
	PROFILE_SCOPED()

	// If this collision space exists for a planet, collide all dynamic geoms
	// against the planet
	if (sphere.radius > 0.0) {
		for (uint32_t idx = 0; idx < m_geoms.size(); idx++) {
			Geom *g = m_geoms[idx];
			if (!g->IsEnabled())
				continue;

			g->CollideSphere(sphere, callback);
		}
	}
}
