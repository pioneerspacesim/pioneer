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

	~BvhNode() {
		if (kids[0]) {
			delete kids[0];
			delete kids[1];
		}
	}
	bool CollideRay(const vector3d &start, const vector3d &invDir, isect_t *isect)
	{
		double
                l1      = (aabb.min.x - start.x) * invDir.x,
                l2      = (aabb.max.x - start.x) * invDir.x,
                lmin    = MIN(l1,l2),
                lmax    = MAX(l1,l2);

		l1      = (aabb.min.y - start.y) * invDir.y;
		l2      = (aabb.max.y - start.y) * invDir.y;
		lmin    = MAX(MIN(l1,l2), lmin);
		lmax    = MIN(MAX(l1,l2), lmax);

		l1      = (aabb.min.z - start.z) * invDir.z;
		l2      = (aabb.max.z - start.z) * invDir.z;
		lmin    = MAX(MIN(l1,l2), lmin);
		lmax    = MIN(MAX(l1,l2), lmax);

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

	BvhTree(const std::list<Geom*> &geoms);
	~BvhTree() {
		if (m_geoms) delete m_geoms;
		if (m_root) delete m_root;
	}

private:
	void BuildNode(BvhNode *node, const std::list<Geom*> &a_geoms, int &outGeomPos);
};

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
		node->kids[0] = new BvhNode();
		node->kids[1] = new BvhNode();

		BuildNode(node->kids[0], side[0], outGeomPos);
		BuildNode(node->kids[1], side[1], outGeomPos);
	}
}

BvhTree::BvhTree(const std::list<Geom*> &geoms)
{
	m_geoms = 0;
	int numGeoms = geoms.size();
	if (numGeoms == 0) {
		m_root = new BvhNode;
		memset(m_root, 0, sizeof(BvhNode));
	} else {
		m_geoms = new Geom*[numGeoms];
		m_root = new BvhNode;
		int geomPos = 0;
		BuildNode(m_root, geoms, geomPos);
		assert(geomPos == numGeoms);
	}
}

///////////////////////////////////////////////////////////////////////

CollisionSpace::CollisionSpace()
{
	sphere.radius = 0;
	m_needStaticGeomRebuild = true;
	m_staticObjectTree = 0;
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
	if (sphere.radius != 0) {
		/* Collide with lovely sphere! */
		const vector3d v = start - sphere.pos;
		const double b = -vector3d::Dot (v, dir);
		double det = (b * b) - vector3d::Dot (v, v) + (sphere.radius*sphere.radius);
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
						isect->dist = (float)i1;
						isect->triIdx = 0;
					}
				}
			}
		}
	}
}

void CollisionSpace::TraceRay(const vector3d &start, const vector3d &dir, float len, CollisionContact *c, Geom *ignore)
{
	vector3d invDir(1.0/dir.x, 1.0/dir.y, 1.0/dir.z);
	c->dist = len;
	
	BvhNode *vn_stack[16];
	BvhNode *node = m_staticObjectTree->m_root;
	int stackPos = -1;

	for (;;) {
		// do we hit it?
		isect_t isect;
		isect.dist = (float)c->dist;
		isect.triIdx = -1;
		if (!node->CollideRay(start, invDir, &isect)) goto pop_jizz;

		if (node->geomStart) {
			// it is a leaf node
			// collide with all geoms
			for (int i=0; i<node->numGeoms; i++) {
				Geom *g = node->geomStart[i];

				const matrix4x4d &invTrans = g->GetInvTransform();
				vector3d ms = invTrans * start;
				vector3d md = invTrans.ApplyRotationOnly(dir);
				vector3f modelStart = vector3f((float)ms.x, (float)ms.y, (float)ms.z);
				vector3f modelDir = vector3f((float)md.x, (float)md.y, (float)md.z);
		
				isect_t isect;
				isect.dist = (float)c->dist;
				isect.triIdx = -1;
				g->GetGeomTree()->TraceRay(modelStart, modelDir, &isect);
				if (isect.triIdx != -1) {
					c->pos = start + dir*isect.dist;
					
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
			vector3f modelStart = vector3f((float)ms.x, (float)ms.y, (float)ms.z);
			vector3f modelDir = vector3f((float)md.x, (float)md.y, (float)md.z);
	
			isect_t isect;
			isect.dist = (float)c->dist;
			isect.triIdx = -1;
			(*i)->GetGeomTree()->TraceRay(modelStart, modelDir, &isect);
			if (isect.triIdx != -1) {
				c->pos = start + dir*isect.dist;
				
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
		isect.dist = (float)c->dist;
		isect.triIdx = -1;
		CollideRaySphere(start, dir, &isect);
		if (isect.triIdx != -1) {
			c->pos = start + dir*isect.dist;
			c->normal = vector3d(0.0);
			c->depth = len - isect.dist;
			c->triIdx = -1;
			c->userData1 = sphere.userData;
			c->userData2 = 0;
			c->geomFlag = 0;
		}
	}
}

void CollisionSpace::CollideGeoms(Geom *a)
{
	// our big aabb
	vector3d pos = a->GetPosition();
	double radius = a->GetGeomTree()->GetRadius();
	Aabb ourAabb;
	ourAabb.min = pos - vector3d(radius, radius, radius);
	ourAabb.max = pos + vector3d(radius, radius, radius);

	int stackPos = -1;
	BvhNode *stack[16];
	BvhNode *node = m_staticObjectTree->m_root;

	for (;;) {
		if (ourAabb.Intersects(node->aabb)) {
			if (node->geomStart) {
				for (int i=0; i<node->numGeoms; i++) {
					Geom *g = node->geomStart[i];
					double radius2 = g->GetGeomTree()->GetRadius();
					vector3d pos2 = g->GetPosition();
					if ((pos-pos2).Length() <= (radius + radius2)) {
						a->Collide(node->geomStart[i]);
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

	for (std::list<Geom*>::iterator i = m_geoms.begin(); i != m_geoms.end(); ++i) {
		if ((*i) != a) {
			double radius2 = (*i)->GetGeomTree()->GetRadius();
			vector3d pos2 = (*i)->GetPosition();
			if ((pos-pos2).Length() <= (radius + radius2)) {
				a->Collide(*i);
				if (!(*i)->HasMoved()) (*i)->Collide(a);
			}
		}
	}
	/* test the fucker against the planet sphere thing
	 * (only if no geomFlag. they can be important like docking pads) */
	if ((!a->contact.geomFlag) && (sphere.radius != 0)) {
		a->CollideSphere(sphere);
	}

}

void CollisionSpace::RebuildObjectTrees()
{
	if (m_needStaticGeomRebuild) {
		if (m_staticObjectTree) delete m_staticObjectTree;
		m_staticObjectTree = new BvhTree(m_staticGeoms);
	}
	m_needStaticGeomRebuild = false;
}

void CollisionSpace::Collide(void (*callback)(CollisionContact*))
{
	RebuildObjectTrees();

	for (std::list<Geom*>::iterator i = m_geoms.begin(); i != m_geoms.end(); ++i) {
		(*i)->contact = CollisionContact();
		if ((*i)->HasMoved()) {
			CollideGeoms(*i);
			if ((*i)->contact.triIdx != -1)
				(*callback)(&(*i)->contact);
		}
	}
}
