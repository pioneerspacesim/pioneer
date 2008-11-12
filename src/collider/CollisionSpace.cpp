#include "CollisionSpace.h"
#include "Geom.h"
#include "GeomTree.h"

CollisionSpace::CollisionSpace()
{
	sphere.radius = 0;
}

void CollisionSpace::AddGeom(Geom *geom)
{
	m_geoms.push_back(geom);
	printf("%d geoms in space\n", m_geoms.size());
}

void CollisionSpace::RemoveGeom(Geom *geom)
{
	m_geoms.remove(geom);
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
						isect->dist = i1;
						isect->triIdx = 0;
					}
				}
			}
		}
	}
}

void CollisionSpace::TraceRay(const vector3d &start, const vector3d &dir, isect_t *isect)
{
	for (std::list<Geom*>::iterator i = m_geoms.begin(); i != m_geoms.end(); ++i) {
		if ((*i)->IsEnabled()) {
			const matrix4x4d &invTrans = (*i)->GetInvTransform();
			vector3d ms = invTrans * start;
			vector3d md = invTrans.ApplyRotationOnly(dir);
			vector3f modelStart = vector3f(ms.x, ms.y, ms.z);
			vector3f modelDir = vector3f(md.x, md.y, md.z);
			(*i)->GetGeomTree()->TraceRay(modelStart, modelDir, isect);
		}
	}
	CollideRaySphere(start, dir, isect);
}

void CollisionSpace::TraceRay(const vector3d &start, const vector3d &dir, float len, CollisionContact *c, Geom *ignore)
{
	double dist = len;
	for (std::list<Geom*>::iterator i = m_geoms.begin(); i != m_geoms.end(); ++i) {
		if ((*i) == ignore) continue;
		if ((*i)->IsEnabled()) {
			const matrix4x4d &invTrans = (*i)->GetInvTransform();
			vector3d ms = invTrans * start;
			vector3d md = invTrans.ApplyRotationOnly(dir);
			vector3f modelStart = vector3f(ms.x, ms.y, ms.z);
			vector3f modelDir = vector3f(md.x, md.y, md.z);
	
			isect_t isect;
			isect.dist = dist;
			isect.triIdx = -1;
			(*i)->GetGeomTree()->TraceRay(modelStart, modelDir, &isect);
			if (isect.triIdx != -1) {
				c->pos = start + dir*isect.dist;
				c->normal = vector3d(0.0);
				c->depth = len - isect.dist;
				c->triIdx = isect.triIdx;
				c->userData1 = (*i)->GetUserData();
				c->userData2 = 0;
				c->geomFlag = (*i)->GetGeomTree()->GetTriFlag(isect.triIdx);
				dist = isect.dist;
			}
		}
	}
	{
		isect_t isect;
		isect.dist = dist;
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
	Aabb bigAabb;
	bigAabb = a->GetGeomTree()->GetMaxAabb();
	bigAabb.min += pos;
	bigAabb.max += pos;


	/* first test the fucker against the planet sphere thing */
	if (sphere.radius != 0) {
		a->CollideSphere(sphere);
	}

	for (std::list<Geom*>::iterator i = m_geoms.begin(); i != m_geoms.end(); ++i) {
		if ((*i) != a) {
			Aabb bigAabb2;
			bigAabb2 = (*i)->GetGeomTree()->GetMaxAabb();
			vector3d pos2 = (*i)->GetPosition();
			bigAabb2.min += pos2;
			bigAabb2.max += pos2;
			if (bigAabb.Intersects(bigAabb2)) {
				a->Collide(*i);
				if (!(*i)->HasMoved()) (*i)->Collide(a);
			}
		}
	}
}

void CollisionSpace::Collide(void (*callback)(CollisionContact*))
{
	for (std::list<Geom*>::iterator i = m_geoms.begin(); i != m_geoms.end(); ++i) {
		(*i)->contact = CollisionContact();
		if ((*i)->HasMoved()) {
			CollideGeoms(*i);
			if ((*i)->contact.triIdx != -1)
				(*callback)(&(*i)->contact);
		}
	}
}
