#include "CollisionSpace.h"
#include "Geom.h"
#include "GeomTree.h"

CollisionSpace::CollisionSpace()
{

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
}

void CollisionSpace::CollideGeoms(Geom *a, void (*callback)(CollisionContact*))
{
	// our big aabb
	vector3d pos = a->GetPosition();
	Aabb bigAabb;
	bigAabb = a->GetGeomTree()->GetMaxAabb();
	bigAabb.min += pos;
	bigAabb.max += pos;

	for (std::list<Geom*>::iterator i = m_geoms.begin(); i != m_geoms.end(); ++i) {
		if ((*i) != a) {
			Aabb bigAabb2;
			bigAabb2 = (*i)->GetGeomTree()->GetMaxAabb();
			vector3d pos2 = (*i)->GetPosition();
			bigAabb2.min += pos2;
			bigAabb2.max += pos2;
			if (bigAabb.Intersects(bigAabb2)) {
				a->Collide(*i, callback);
				if (!(*i)->HasMoved()) (*i)->Collide(a, callback);
			}
		}
	}
}

void CollisionSpace::Collide(void (*callback)(CollisionContact*))
{
	// for the time being do a totally retarded intersection of every body on every other
	for (std::list<Geom*>::iterator i = m_geoms.begin(); i != m_geoms.end(); ++i) {
		if ((*i)->HasMoved()) CollideGeoms(*i, callback);
	}
}
