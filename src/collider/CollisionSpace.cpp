// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "CollisionSpace.h"

#include "BVHTree.h"
#include "CollisionContact.h"
#include "Geom.h"
#include "GeomTree.h"
#include "profiler/Profiler.h"

#include <algorithm>

/* volnode!!!!!!!!!!! */
struct BvhNode {
	Aabb aabb;

	/* if geomStart == 0 then not leaf,
	 * kids[] valid */
	int numGeoms;
	Geom **geomStart;

	BvhNode *kids[2];

	BvhNode()
	{
		kids[0] = 0;
		geomStart = 0;
	}

	bool IsLeaf() const { return geomStart != nullptr; }

	bool CollideRay(const vector3d &start, const vector3d &invDir, isect_t *isect)
	{
		PROFILE_SCOPED()
		double
			l1 = (aabb.min.x - start.x) * invDir.x,
			l2 = (aabb.max.x - start.x) * invDir.x,
			lmin = std::min(l1, l2),
			lmax = std::max(l1, l2);

		l1 = (aabb.min.y - start.y) * invDir.y;
		l2 = (aabb.max.y - start.y) * invDir.y;
		lmin = std::max(std::min(l1, l2), lmin);
		lmax = std::min(std::max(l1, l2), lmax);

		l1 = (aabb.min.z - start.z) * invDir.z;
		l2 = (aabb.max.z - start.z) * invDir.z;
		lmin = std::max(std::min(l1, l2), lmin);
		lmax = std::min(std::max(l1, l2), lmax);

		return ((lmax >= 0.f) & (lmax >= lmin) & (lmin < isect->dist));
	}
};

/*
 * Tree of objects in collision space (one tree for static objects, one for
 * dynamic)
 */
class BvhTree {
public:
	Geom **m_geoms;
	BvhNode *m_root;
	BvhNode *m_nodesAlloc;
	int m_nodesAllocPos;
	int m_nodesAllocMax;
	// tree height at best log2(n), at worst n (n is the number of objects in the tree)
	// it seems that the height of that tree never exceeds 20
	// also TREE HEIGHT - THREE EIGHT, neat
	static constexpr int MAX_TREE_HEIGHT = 38;

	BvhNode *AllocNode()
	{
		assert(m_nodesAllocPos < m_nodesAllocMax);
		return &m_nodesAlloc[m_nodesAllocPos++];
	}

	// Avoid re-allocation of memory when m_geoms are less than before,
	void Refresh(const std::list<Geom *> &m_geoms);

	BvhTree(const std::list<Geom *> &geoms);
	~BvhTree()
	{
		FreeAll();
	}
	void CollideGeom(Geom *, const Aabb &, int minMailboxValue, void (*callback)(CollisionContact *));

private:
	void BuildNode(BvhNode *node, const std::list<Geom *> &a_geoms, int &outGeomPos, int maxHeight);

	void FreeAll()
	{
		if (m_geoms) delete[] m_geoms;
		m_geoms = nullptr;
		if (m_nodesAlloc) delete[] m_nodesAlloc;
		m_nodesAlloc = nullptr;
	}
};

BvhTree::BvhTree(const std::list<Geom *> &geoms)
{
	PROFILE_SCOPED()
	m_geoms = nullptr;
	m_nodesAlloc = nullptr;
	int numGeoms = geoms.size();
	if (numGeoms == 0) {
		m_root = nullptr;
		return;
	}
	m_geoms = new Geom *[numGeoms];
	int geomPos = 0;
	m_nodesAllocPos = 0;
	m_nodesAllocMax = numGeoms * 2;
	m_nodesAlloc = new BvhNode[m_nodesAllocMax];
	m_root = AllocNode();
	BuildNode(m_root, geoms, geomPos, MAX_TREE_HEIGHT);
	assert(geomPos == numGeoms);
}

void BvhTree::Refresh(const std::list<Geom *> &geoms)
{
	PROFILE_SCOPED()
	int numGeoms = geoms.size();
	if (numGeoms == 0) {
		m_root = nullptr;
		FreeAll();
		return;
	}
	int geomPos = 0;
	m_nodesAllocPos = 0;
	m_root = AllocNode();
	BuildNode(m_root, geoms, geomPos, MAX_TREE_HEIGHT);
	assert(geomPos == numGeoms);
}

void BvhTree::CollideGeom(Geom *g, const Aabb &geomAabb, int minMailboxValue, void (*callback)(CollisionContact *))
{
	PROFILE_SCOPED()
	if (!m_root) return;

	// our big aabb
	vector3d pos = g->GetPosition();
	double radius = g->GetGeomTree()->GetRadius();

	int stackPos = -1;
	BvhNode *stack[MAX_TREE_HEIGHT];
	BvhNode *node = m_root;

	for (;;) {
		if (geomAabb.Intersects(node->aabb)) {
			if (node->geomStart) {
				for (int i = 0; i < node->numGeoms; i++) {
					Geom *g2 = node->geomStart[i];
					if (!g2->IsEnabled()) continue;
					if (g2->GetMailboxIndex() < minMailboxValue) continue;
					if (g2 == g) continue;
					if (g->GetGroup() && g2->GetGroup() == g->GetGroup()) continue;
					double radius2 = g2->GetGeomTree()->GetRadius();
					vector3d pos2 = g2->GetPosition();
					if ((pos - pos2).Length() <= (radius + radius2)) {
						g->Collide(g2, callback);
					}
				}
			} else if (node->kids[0]) {
				stack[++stackPos] = node->kids[0];
				node = node->kids[1];
				continue;
			}
		}

		if (stackPos < 0) break;
		node = stack[stackPos--];
	}
}

void BvhTree::BuildNode(BvhNode *node, const std::list<Geom *> &a_geoms, int &outGeomPos, int maxHeight)
{
	// PROFILE_SCOPED()
	const int numGeoms = a_geoms.size();
	// make aabb from spheres
	// XXX suboptimal for static objects, as they have fixed rotation so
	// we can use a precise rotated aabb rather than worst case XXX
	Aabb aabb;
	aabb.min = vector3d(FLT_MAX, FLT_MAX, FLT_MAX);
	aabb.max = vector3d(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (std::list<Geom *>::const_iterator i = a_geoms.begin();
		 i != a_geoms.end(); ++i) {
		vector3d p = (*i)->GetPosition();
		double rad = (*i)->GetGeomTree()->GetRadius();
		aabb.Update(p + vector3d(rad, rad, rad));
		aabb.Update(p - vector3d(rad, rad, rad));
	}

	// divide by longest axis
	int axis;
	const vector3d axislen = aabb.max - aabb.min;
	if ((axislen.x > axislen.y) && (axislen.x > axislen.z))
		axis = 0;
	else if (axislen.y > axislen.z)
		axis = 1;
	else
		axis = 2;
	const double pivot = 0.5 * (aabb.max[axis] + aabb.min[axis]);

	std::list<Geom *> side[2];

	for (std::list<Geom *>::const_iterator i = a_geoms.begin();
		 i != a_geoms.end(); ++i) {
		if ((*i)->GetPosition()[axis] < pivot) {
			side[0].push_back(*i);
		} else {
			side[1].push_back(*i);
		}
	}

	node->numGeoms = numGeoms;
	node->aabb = aabb;

	// one side has all nodes, or we have reached the maximum tree height - just make a fucking child
	if (side[0].size() == 0 || side[1].size() == 0 || maxHeight == 0) {
		node->geomStart = &m_geoms[outGeomPos];

		// copy geoms to the stinking flat array
		for (std::list<Geom *>::const_iterator i = a_geoms.begin();
			 i != a_geoms.end(); ++i) {
			m_geoms[outGeomPos++] = *i;
		}
	} else {
		// recurse!
		node->geomStart = 0;
		node->kids[0] = AllocNode();
		node->kids[1] = AllocNode();

		BuildNode(node->kids[0], side[0], outGeomPos, maxHeight - 1);
		BuildNode(node->kids[1], side[1], outGeomPos, maxHeight - 1);
	}
}

///////////////////////////////////////////////////////////////////////

int CollisionSpace::s_nextHandle = 1;

CollisionSpace::CollisionSpace() :
	m_staticObjectTree(nullptr),
	m_dynamicObjectTree(nullptr),
	m_duringCollision(false)
{
	PROFILE_SCOPED()
	sphere.radius = 0;
	m_needStaticGeomRebuild = true;
	m_oldGeomsNumber = 0;
}

CollisionSpace::~CollisionSpace()
{
	PROFILE_SCOPED()
	if (m_staticObjectTree) delete m_staticObjectTree;
	if (m_dynamicObjectTree) delete m_dynamicObjectTree;
}

void CollisionSpace::AddGeom(Geom *geom)
{
	PROFILE_SCOPED()
	assert(!m_duringCollision);

	m_geoms.push_back(geom);
	m_geomVec.push_back(geom);
}

void CollisionSpace::RemoveGeom(Geom *geom)
{
	PROFILE_SCOPED()
	assert(!m_duringCollision);
	m_geoms.remove(geom);

	auto iter = std::find(m_geomVec.begin(), m_geomVec.end(), geom);
	if (iter == m_geomVec.end())
		return;

	if (m_geomVec.size() > 1)
		std::swap(*iter, m_geomVec.back());
	m_geomVec.pop_back();
}

void CollisionSpace::AddStaticGeom(Geom *geom)
{
	PROFILE_SCOPED()
	assert(!m_duringCollision);

	m_staticGeoms.push_back(geom);
	m_staticGeomVec.push_back(geom);
	m_needStaticGeomRebuild = true;
}

void CollisionSpace::RemoveStaticGeom(Geom *geom)
{
	PROFILE_SCOPED()
	assert(!m_duringCollision);

	m_staticGeoms.remove(geom);
	m_needStaticGeomRebuild = true;

	auto iter = std::find(m_staticGeomVec.begin(), m_staticGeomVec.end(), geom);
	if (iter == m_staticGeomVec.end())
		return;

	if (m_staticGeomVec.size() > 1)
		std::swap(*iter, m_staticGeomVec.back());
	m_staticGeomVec.pop_back();
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

	BvhNode *vn_stack[BvhTree::MAX_TREE_HEIGHT];
	BvhNode *node = m_staticObjectTree->m_root;
	int stackPos = -1;

	for (; node;) {
		// do we hit it?
		{
			isect_t isect;
			isect.dist = float(c->distance);
			isect.triIdx = -1;
			if (!node->CollideRay(start, invDir, &isect)) goto pop_jizz;
		}

		if (node->geomStart) {
			// it is a leaf node
			// collide with all geoms
			for (int i = 0; i < node->numGeoms; i++) {
				Geom *g = node->geomStart[i];

				const matrix4x4d &invTrans = g->GetInvTransform();
				vector3d ms = invTrans * start;
				vector3d md = invTrans.ApplyRotationOnly(dir);
				vector3f modelStart = vector3f(ms.x, ms.y, ms.z);
				vector3f modelDir = vector3f(md.x, md.y, md.z);

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
		} else if (node->kids[0]) {
			vn_stack[++stackPos] = node->kids[0];
			node = node->kids[1];
			continue;
		}
	pop_jizz:
		if (stackPos < 0) break;
		node = vn_stack[stackPos--];
	}

	for (std::list<Geom *>::iterator i = m_geoms.begin(); i != m_geoms.end(); ++i) {
		if ((*i) == ignore) continue;
		if ((*i)->IsEnabled()) {
			const matrix4x4d &invTrans = (*i)->GetInvTransform();
			vector3d ms = invTrans * start;
			vector3d md = invTrans.ApplyRotationOnly(dir);
			vector3f modelStart = vector3f(ms.x, ms.y, ms.z);
			vector3f modelDir = vector3f(md.x, md.y, md.z);

			isect_t isect;
			isect.dist = float(c->distance);
			isect.triIdx = -1;
			(*i)->GetGeomTree()->TraceRay(modelStart, modelDir, &isect);
			if (isect.triIdx != -1) {
				c->pos = start + dir * double(isect.dist);

				vector3f n = (*i)->GetGeomTree()->GetTriNormal(isect.triIdx);
				c->normal = vector3d(n.x, n.y, n.z);
				c->normal = (*i)->GetTransform().ApplyRotationOnly(c->normal);

				c->depth = len - isect.dist;
				c->triIdx = isect.triIdx;
				c->userData1 = (*i)->GetUserData();
				c->userData2 = 0;
				c->geomFlag = (*i)->GetGeomTree()->GetTriFlag(isect.triIdx);
				c->distance = isect.dist;
			}
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

/*
 * Do not collide objects with mailbox value < minMailboxValue
 */
void CollisionSpace::CollideGeoms(Geom *a, int minMailboxValue, void (*callback)(CollisionContact *))
{
	PROFILE_SCOPED()
	if (!a->IsEnabled()) return;
	// our big aabb
	vector3d pos = a->GetPosition();
	double radius = a->GetGeomTree()->GetRadius();
	Aabb ourAabb;
	ourAabb.min = pos - vector3d(radius, radius, radius);
	ourAabb.max = pos + vector3d(radius, radius, radius);

	if (m_staticObjectTree) m_staticObjectTree->CollideGeom(a, ourAabb, 0, callback);
	if (m_dynamicObjectTree) m_dynamicObjectTree->CollideGeom(a, ourAabb, minMailboxValue, callback);

	/* test the fucker against the planet sphere thing */
	if (sphere.radius > 0.0) {
		a->CollideSphere(sphere, callback);
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

double CollisionSpace::CalcSAH(BvhTree *tree) const
{
	if (!tree || !tree->m_root)
		return 0.0;

	double outSAH = 0.0;
	auto *rootNode = tree->m_root;
	std::vector<BvhNode *> m_nodeStack = { rootNode };

	while (!m_nodeStack.empty()) {
		auto *node = m_nodeStack.back();
		m_nodeStack.pop_back();

		if (!node->IsLeaf() && node->kids[0])
			m_nodeStack.push_back(node->kids[0]);
		if (!node->IsLeaf() && node->kids[1])
			m_nodeStack.push_back(node->kids[1]);

		// surface area = 2 * width * depth * height
		vector3d size = node->aabb.max - node->aabb.min;
		double area = 2.f * size.x * size.y * size.z;

		// Cost function according to https://users.aalto.fi/~laines9/publications/aila2013hpg_paper.pdf Eq. 1
		if (node->IsLeaf())
			outSAH += node->numGeoms * area;
		else
			outSAH += 1.2 * area;
	}

	vector3d rootSize = rootNode->aabb.max - rootNode->aabb.min;
	float rootArea = 2.f * rootSize.x * rootSize.y * rootSize.z;

	// Perform 1 / Aroot * ( SAH sums )
	outSAH /= rootArea;

	// Remove the (normalized) SAH cost of the root node from the result
	// The SAH metric doesn't include the cost of the root node
	outSAH -= 1.f;

	return outSAH;
}

double CollisionSpace::CalcSAH(SingleBVHTree *tree) const
{
	return tree ? tree->CalculateSAH() : 0.0;
}

// ===================================================================

void CollisionSpace::RebuildObjectTrees()
{
	PROFILE_SCOPED()

	{
		PROFILE_SCOPED_DESC("Rebuild Old Trees")

		if (m_needStaticGeomRebuild) {
			if (m_staticObjectTree) delete m_staticObjectTree;
			m_staticObjectTree = new BvhTree(m_staticGeoms);
		}
		if (m_oldGeomsNumber < m_geoms.size()) {
			// Have more geoms: rebuild completely (ask more memory)
			if (m_dynamicObjectTree) delete m_dynamicObjectTree;
			m_dynamicObjectTree = new BvhTree(m_geoms);
		} else {
			// Same number or less (no needs for more memory)
			if (m_dynamicObjectTree)
				m_dynamicObjectTree->Refresh(m_geoms);
			else
				m_dynamicObjectTree = new BvhTree(m_geoms);
		}

		m_oldGeomsNumber = m_geoms.size();
	}
	{
		PROFILE_SCOPED_DESC("Rebuild New Trees")

		if (!m_staticObjectTree2) {
			m_staticObjectTree2.reset(new SingleBVHTree());
		}

		if (!m_dynamicObjectTree2) {
			m_dynamicObjectTree2.reset(new SingleBVHTree());
		}

		if (m_needStaticGeomRebuild) {
			std::vector<AABBd> staticAabbs;
			m_enabledStaticGeoms = SortEnabledGeoms(m_staticGeomVec);

			RebuildBVHTree(m_staticObjectTree2.get(), m_enabledStaticGeoms, m_staticGeomVec, staticAabbs);
		}

		m_enabledDynGeoms = SortEnabledGeoms(m_geomVec);
		RebuildBVHTree(m_dynamicObjectTree2.get(), m_enabledDynGeoms, m_geomVec, m_geomAabbs);
	}

	m_needStaticGeomRebuild = false;
}

uint32_t CollisionSpace::SortEnabledGeoms(std::vector<Geom *> &geoms)
{
	PROFILE_SCOPED()

	if (geoms.empty())
		return 0;

	// Simple O(n) sort algorithm
	// Sort geoms according to enabled state (group all enabled geoms at start of array)
	uint32_t startIdx = 0;
	uint32_t endIdx = geoms.size() - 1;

	while (startIdx <= endIdx && endIdx) {
		if (geoms[startIdx]->IsEnabled()) {
			startIdx++;
		} else {
			std::swap(geoms[startIdx], geoms[endIdx]);
			endIdx--;
		}
	}

	return startIdx;
}

void CollisionSpace::RebuildBVHTree(SingleBVHTree *tree, uint32_t numGeoms, const std::vector<Geom *> &geoms, std::vector<AABBd> &aabbs)
{
	PROFILE_SCOPED()
	assert(numGeoms < INT32_MAX);

	if (numGeoms == 0)
		return;

	aabbs.resize(0);
	aabbs.reserve(numGeoms);

	AABBd bounds = AABBd();

	for (size_t idx = 0; idx < numGeoms; idx++) {
		const Geom *geom = geoms[idx];

		// treat geoms as spheres and create a worst-case AABB around them
		vector3d pos = geom->GetPosition();
		double radius = geom->GetGeomTree()->GetRadius();

		AABBd aabb { {pos - radius }, { pos + radius } };

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

	{
		PROFILE_SCOPED_DESC("Mailbox Collision")

		int mailboxMin = 0;
		for (std::list<Geom *>::iterator i = m_geoms.begin(); i != m_geoms.end(); ++i) {
			(*i)->SetMailboxIndex(mailboxMin++);
		}

		/* This mailbox nonsense is so: after collision(a,b), we will not
		* attempt collision(b,a) */
		mailboxMin = 1;
		for (std::list<Geom *>::iterator i = m_geoms.begin(); i != m_geoms.end(); ++i, mailboxMin++) {
			CollideGeoms(*i, mailboxMin, callback);
		}
	}

	{
		PROFILE_SCOPED_DESC("BVHTree2 Collision")
		std::vector<Intersection> static_isect;
		std::vector<Intersection> dyn_isect;

		static_isect.reserve(32);
		dyn_isect.reserve(32);

		bool hasStatic = m_enabledStaticGeoms > 0;

		for (uint32_t idx = 0; idx < m_enabledDynGeoms; idx++) {
			Geom *g = m_geomVec[idx];
			if (!g->IsEnabled())
				continue;

			const AABBd &aabb = m_geomAabbs[idx];

			if (hasStatic)
				m_staticObjectTree2->ComputeOverlap(idx, aabb, static_isect);
			m_dynamicObjectTree2->ComputeOverlap(idx, aabb, dyn_isect);
		}

		// No mailbox test needed for colliding a dynamic geom against static geoms
		for (const Intersection &isect : static_isect) {
			CollideGeom(m_geomVec[isect.first], m_staticGeomVec[isect.second], callback);
		}

		// Simple mailbox test to ensure every valid collision is only processed once
		for (const Intersection &isect : dyn_isect) {
			if (isect.first >= isect.second)
				continue;

			CollideGeom(m_geomVec[isect.first], m_geomVec[isect.second], callback);
		}

		CollidePlanet(callback);
	}

	m_duringCollision = false;
}

void CollisionSpace::CollidePlanet(void (*callback)(CollisionContact *))
{
	PROFILE_SCOPED()

	// If this collision space exists for a planet, collide all dynamic geoms
	// against the planet
	if (sphere.radius > 0.0) {
		for (uint32_t idx = 0; idx < m_geomVec.size(); idx++) {
			Geom *g = m_geomVec[idx];
			if (!g->IsEnabled())
				continue;

			g->CollideSphere(sphere, callback);
		}
	}
}
