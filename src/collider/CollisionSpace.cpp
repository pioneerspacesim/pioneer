#include "CollisionSpace.h"
#include "Geom.h"
#include "GeomTree.h"

CollisionSpace::CollisionSpace()
{

}

void CollisionSpace::AddGeom(Geom *geom)
{
	m_geoms.push_back(geom);
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

