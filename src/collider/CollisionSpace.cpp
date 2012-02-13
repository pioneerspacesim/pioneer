#include "CollisionSpace.h"
#include "Geom.h"
#include "GeomTree.h"
#include "../libs.h"

/* volnode!!!!!!!!!!! */
struct BvhNode {
	Aabb aabb;
	
	/* if geomStart == 0 then not leaf,
	 * kids[] valid */
	int numGeoms;
	Geom **geomStart;
	
	BvhNode *kids[2];

	BvhNode() {
		kids[0] = 0;
		geomStart = 0;
	}

	bool CollideRay(const vector3d &start, const vector3d &invDir, isect_t *isect)
	{
		double
                l1      = (aabb.min.x - start.x) * invDir.x,
                l2      = (aabb.max.x - start.x) * invDir.x,
                lmin    = std::min(l1,l2),
                lmax    = std::max(l1,l2);

		l1      = (aabb.min.y - start.y) * invDir.y;
		l2      = (aabb.max.y - start.y) * invDir.y;
		lmin    = std::max(std::min(l1,l2), lmin);
		lmax    = std::min(std::max(l1,l2), lmax);

		l1      = (aabb.min.z - start.z) * invDir.z;
		l2      = (aabb.max.z - start.z) * invDir.z;
		lmin    = std::max(std::min(l1,l2), lmin);
		lmax    = std::min(std::max(l1,l2), lmax);

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

	BvhNode *AllocNode() {
		assert(m_nodesAllocPos < m_nodesAllocMax);
		return &m_nodesAlloc[m_nodesAllocPos++];
	}

	BvhTree(const std::list<Geom*> &geoms);
	~BvhTree() {
		if (m_geoms) delete [] m_geoms;
		if (m_nodesAlloc) delete [] m_nodesAlloc;
	}
	void CollideGeom(Geom *, const Aabb &, int minMailboxValue, void (*callback)(CollisionContact*));

private:
	void BuildNode(BvhNode *node, const std::list<Geom*> &a_geoms, int &outGeomPos);
};

BvhTree::BvhTree(const std::list<Geom*> &geoms)
{
	m_geoms = 0;
	m_nodesAlloc = 0;
	int numGeoms = geoms.size();
	if (numGeoms == 0) {
		m_root = 0;
		return;
	}
	m_geoms = new Geom*[numGeoms];
	int geomPos = 0;
	m_nodesAllocPos = 0;
	m_nodesAllocMax = numGeoms*2;
	m_nodesAlloc = new BvhNode[m_nodesAllocMax];
	m_root = AllocNode();
	BuildNode(m_root, geoms, geomPos);
	assert(geomPos == numGeoms);
}

void BvhTree::CollideGeom(Geom *g, const Aabb &geomAabb, int minMailboxValue, void (*callback)(CollisionContact*))
{
	if (!m_root) return;

	// our big aabb
	vector3d pos = g->GetPosition();
	double radius = g->GetGeomTree()->GetRadius();

	int stackPos = -1;
	BvhNode *stack[16];
	BvhNode *node = m_root;

	for (;;) {
		if (geomAabb.Intersects(node->aabb)) {
			if (node->geomStart) {
				for (int i=0; i<node->numGeoms; i++) {
					Geom *g2 = node->geomStart[i];
					if (!g2->IsEnabled()) continue;
					if (g2->GetMailboxIndex() < minMailboxValue) continue;
					if (g2 == g) continue;
					double radius2 = g2->GetGeomTree()->GetRadius();
					vector3d pos2 = g2->GetPosition();
					if ((pos-pos2).Length() <= (radius + radius2)) {
						g->Collide(g2, callback);
					}
				}
			}
			else if (node->kids[0]) {
				stack[++stackPos] = node->kids[0];
				node = node->kids[1];
				continue;
			}
		}

		if (stackPos < 0) break;
		node = stack[stackPos--];
	}
}

void BvhTree::BuildNode(BvhNode *node, const std::list<Geom*> &a_geoms, int &outGeomPos)
{
	const int numGeoms = a_geoms.size();
	// make aabb from spheres
	// XXX suboptimal for static objects, as they have fixed rotation so
	// we can use a precise rotated aabb rather than worst case XXX
	Aabb aabb;
	aabb.min = vector3d(FLT_MAX, FLT_MAX, FLT_MAX);
	aabb.max = vector3d(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (std::list<Geom*>::const_iterator i = a_geoms.begin();
			i != a_geoms.end(); ++i) {
		vector3d p = (*i)->GetPosition();
		double rad = (*i)->GetGeomTree()->GetRadius();
		aabb.Update(p + vector3d(rad,rad,rad));
		aabb.Update(p - vector3d(rad,rad,rad));
	}

	// divide by longest axis
	int axis;
	const vector3d axislen = aabb.max - aabb.min;
	if ((axislen.x > axislen.y) && (axislen.x > axislen.z)) axis = 0;
	else if (axislen.y > axislen.z) axis = 1;
	else axis = 2;
	const double pivot = 0.5*(aabb.max[axis] + aabb.min[axis]);

	std::list<Geom*> side[2];

	for (std::list<Geom*>::const_iterator i = a_geoms.begin();
			i != a_geoms.end(); ++i) {
		if ((*i)->GetPosition()[axis] < pivot) {
			side[0].push_back(*i);
		} else {
			side[1].push_back(*i);
		}
	}

	node->numGeoms = numGeoms;
	node->aabb = aabb;

	// side 1 has all nodes. just make a fucking child 
	if ((side[0].size() == 0) || (side[1].size() == 0)) {
		node->geomStart = &m_geoms[outGeomPos];

		// copy geoms to the stinking flat array
		for (std::list<Geom*>::const_iterator i = a_geoms.begin();
				i != a_geoms.end(); ++i) {
			m_geoms[outGeomPos++] = *i;
		}
	} else {
		// recurse!
		node->geomStart = 0;
		node->kids[0] = AllocNode();
		node->kids[1] = AllocNode();

		BuildNode(node->kids[0], side[0], outGeomPos);
		BuildNode(node->kids[1], side[1], outGeomPos);
	}
}

///////////////////////////////////////////////////////////////////////

CollisionSpace::CollisionSpace()
{
	sphere.radius = 0;
	m_needStaticGeomRebuild = true;
	m_staticObjectTree = 0;
	m_dynamicObjectTree = 0;
}

CollisionSpace::~CollisionSpace()
{
	if (m_staticObjectTree) delete m_staticObjectTree;
	if (m_dynamicObjectTree) delete m_dynamicObjectTree;
}

void CollisionSpace::AddGeom(Geom *geom)
{
	m_geoms.push_back(geom);
}

void CollisionSpace::RemoveGeom(Geom *geom)
{
	m_geoms.remove(geom);
}

void CollisionSpace::AddStaticGeom(Geom *geom)
{
	m_staticGeoms.push_back(geom);
	m_needStaticGeomRebuild = true;
}

void CollisionSpace::RemoveStaticGeom(Geom *geom)
{
	m_staticGeoms.remove(geom);
	m_needStaticGeomRebuild = true;
}

void CollisionSpace::CollideRaySphere(const vector3d &start, const vector3d &dir, isect_t *isect)
{
	if (sphere.radius > 0.0) {
		/* Collide with lovely sphere! */
		const vector3d v = start - sphere.pos;
		const double b = -v.Dot(dir);
		double det = (b * b) - v.LengthSqr() + (sphere.radius*sphere.radius);
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

void CollisionSpace::TraceRay(const vector3d &start, const vector3d &dir, double len, CollisionContact *c, Geom *ignore)
{
	vector3d invDir(1.0/dir.x, 1.0/dir.y, 1.0/dir.z);
	c->dist = len;
	
	BvhNode *vn_stack[16];
	BvhNode *node = m_staticObjectTree->m_root;
	int stackPos = -1;

	for (;node;) {
		// do we hit it?
		{
			isect_t isect;
			isect.dist = float(c->dist);
			isect.triIdx = -1;
			if (!node->CollideRay(start, invDir, &isect)) goto pop_jizz;
		}

		if (node->geomStart) {
			// it is a leaf node
			// collide with all geoms
			for (int i=0; i<node->numGeoms; i++) {
				Geom *g = node->geomStart[i];

				const matrix4x4d &invTrans = g->GetInvTransform();
				vector3d ms = invTrans * start;
				vector3d md = invTrans.ApplyRotationOnly(dir);
				vector3f modelStart = vector3f(ms.x, ms.y, ms.z);
				vector3f modelDir = vector3f(md.x, md.y, md.z);
		
				isect_t isect;
				isect.dist = float(c->dist);
				isect.triIdx = -1;
				g->GetGeomTree()->TraceRay(modelStart, modelDir, &isect);
				if (isect.triIdx != -1) {
					c->pos = start + dir*double(isect.dist);
					
					vector3f n = g->GetGeomTree()->GetTriNormal(isect.triIdx);
					c->normal = vector3d(n.x, n.y, n.z);
					c->normal = g->GetTransform().ApplyRotationOnly(c->normal);
					
					c->depth = len - isect.dist;
					c->triIdx = isect.triIdx;
					c->userData1 = g->GetUserData();
					c->userData2 = 0;
					c->geomFlag = g->GetGeomTree()->GetTriFlag(isect.triIdx);
					c->dist = isect.dist;
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

	for (std::list<Geom*>::iterator i = m_geoms.begin(); i != m_geoms.end(); ++i) {
		if ((*i) == ignore) continue;
		if ((*i)->IsEnabled()) {
			const matrix4x4d &invTrans = (*i)->GetInvTransform();
			vector3d ms = invTrans * start;
			vector3d md = invTrans.ApplyRotationOnly(dir);
			vector3f modelStart = vector3f(ms.x, ms.y, ms.z);
			vector3f modelDir = vector3f(md.x, md.y, md.z);
	
			isect_t isect;
			isect.dist = float(c->dist);
			isect.triIdx = -1;
			(*i)->GetGeomTree()->TraceRay(modelStart, modelDir, &isect);
			if (isect.triIdx != -1) {
				c->pos = start + dir*double(isect.dist);
				
				vector3f n = (*i)->GetGeomTree()->GetTriNormal(isect.triIdx);
				c->normal = vector3d(n.x, n.y, n.z);
				c->normal = (*i)->GetTransform().ApplyRotationOnly(c->normal);
				
				c->depth = len - isect.dist;
				c->triIdx = isect.triIdx;
				c->userData1 = (*i)->GetUserData();
				c->userData2 = 0;
				c->geomFlag = (*i)->GetGeomTree()->GetTriFlag(isect.triIdx);
				c->dist = isect.dist;
			}
		}
	}
	{
		isect_t isect;
		isect.dist = float(c->dist);
		isect.triIdx = -1;
		CollideRaySphere(start, dir, &isect);
		if (isect.triIdx != -1) {
			c->pos = start + dir*double(isect.dist);
			c->normal = vector3d(0.0);
			c->depth = len - isect.dist;
			c->triIdx = -1;
			c->userData1 = sphere.userData;
			c->userData2 = 0;
			c->geomFlag = 0;
		}
	}
}

/*
 * Do not collide objects with mailbox value < minMailboxValue
 */
void CollisionSpace::CollideGeoms(Geom *a, int minMailboxValue, void (*callback)(CollisionContact*))
{
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

void CollisionSpace::RebuildObjectTrees()
{
	if (m_needStaticGeomRebuild) {
		if (m_staticObjectTree) delete m_staticObjectTree;
		m_staticObjectTree = new BvhTree(m_staticGeoms);
	}
	if (m_dynamicObjectTree) delete m_dynamicObjectTree;
	m_dynamicObjectTree = new BvhTree(m_geoms);

	m_needStaticGeomRebuild = false;
}

void CollisionSpace::Collide(void (*callback)(CollisionContact*))
{
	RebuildObjectTrees();
	
	int mailboxMin = 0;
	for (std::list<Geom*>::iterator i = m_geoms.begin(); i != m_geoms.end(); ++i) {
		(*i)->SetMailboxIndex(mailboxMin++);
	}

	/* This mailbox nonsense is so: after collision(a,b), we will not
	 * attempt collision(b,a) */
	mailboxMin = 1;
	for (std::list<Geom*>::iterator i = m_geoms.begin(); i != m_geoms.end(); ++i, mailboxMin++) {
		CollideGeoms(*i, mailboxMin, callback);
	}
}
