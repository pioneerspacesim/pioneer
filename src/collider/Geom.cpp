#include <float.h>
#include "Geom.h"
#include "GeomTree.h"
#include "collider.h"

Geom::Geom(GeomTree *geomtree)
{
	m_geomtree = geomtree;
	m_orient[0] = matrix4x4d::Identity();
	m_orient[1] = matrix4x4d::Identity();
	m_invOrient = matrix4x4d::Identity();
	m_orientIdx = 0;
	m_active = true;
	m_moved = false;
	m_data = 0;
}

matrix4x4d Geom::GetRotation() const
{
	matrix4x4d m = GetTransform();
	m[12] = 0; m[13] = 0; m[14] = 0;
	return m;
}

void Geom::MoveTo(const matrix4x4d &m)
{
	m_orientIdx = !m_orientIdx;
	m_orient[m_orientIdx] = m;
	m_invOrient = m.InverseOf();
	m_moved = true;
}

void Geom::MoveTo(const matrix4x4d &m, const vector3d pos)
{
	m_orientIdx = !m_orientIdx;
	m_orient[m_orientIdx] = m;
	m_orient[m_orientIdx][12] = pos.x;
	m_orient[m_orientIdx][13] = pos.y;
	m_orient[m_orientIdx][14] = pos.z;
	m_invOrient = m_orient[m_orientIdx].InverseOf();
}

vector3d Geom::GetPosition() const
{
	return vector3d(m_orient[m_orientIdx][12],
		m_orient[m_orientIdx][13],
		m_orient[m_orientIdx][14]);
}

void Geom::CollideSphere(Sphere &sphere)
{
	/* if the geom is actually within the sphere, create a contact so
	 * that we can't fall into spheres forever and ever */
	vector3d v = GetPosition() - sphere.pos;
	const double len = v.Length();
	if (len < sphere.radius) {
		contact.pos = GetPosition();
		contact.normal = (1.0/len)*v;
		contact.depth = sphere.radius - len;
		contact.triIdx = 0;
		contact.userData1 = this->m_data;
		contact.userData2 = sphere.userData;
		contact.geomFlag = 0;
		return;
	}
	for (int i=0; i<m_geomtree->m_numVertices; i++) {
		vector3d vtx(&m_geomtree->m_vertices[3*i]);
		vector3d from = m_orient[!m_orientIdx] * vtx;
		vector3d to = m_orient[m_orientIdx] * vtx;
		vector3d dir = to - from;
		const double len = dir.Length();
		dir *= 1.0f/len;

		/* Collide with lovely sphere! */
		const vector3d v = from - sphere.pos;
		const double b = -vector3d::Dot (v, dir);
		double det = (b * b) - vector3d::Dot (v, v) + (sphere.radius*sphere.radius);
		if (det > 0) {
			det = sqrt(det);
			const double i1 = b - det;
			const double i2 = b + det;
			if (i2 > 0) {
				if (i1 > 0) {
					if (i1 < len) {
						contact.pos = from + dir*i1;
						contact.normal = v.Normalized();
						contact.depth = len - i1;
						contact.triIdx = 0;
						contact.userData1 = this->m_data;
						contact.userData2 = sphere.userData;
						contact.geomFlag = 0;
					}
				}
			}
		}
	}
}

void Geom::Collide(Geom *b)
{
	m_moved = false;
	for (int i=0; i<m_geomtree->m_numVertices; i++) {
		vector3d v(&m_geomtree->m_vertices[3*i]);
		vector3d from = m_orient[!m_orientIdx] * v;
		vector3d to = m_orient[m_orientIdx] * v;
		from = b->m_invOrient * from;
		to = b->m_invOrient * to;
		vector3d dir = to - from;
		const double len = dir.Length();
		dir *= 1.0f/len;

		vector3f _from(from.x, from.y, from.z);
		vector3f _dir(dir.x, dir.y, dir.z);

		isect_t isect;
		isect.dist = len;
		isect.triIdx = -1;
		b->m_geomtree->TraceRay(_from, _dir, &isect);
		if (isect.triIdx != -1) {
			const double depth = len - isect.dist;
			if (depth > contact.depth) {
				// in world coords
				contact.pos = b->GetTransform() * (from + dir*isect.dist);
				vector3f n = b->m_geomtree->GetTriNormal(isect.triIdx);
				contact.normal = vector3d(n.x, n.y, n.z);
				contact.normal = b->GetTransform().ApplyRotationOnly(contact.normal);
			
				contact.depth = len - isect.dist;
				contact.triIdx = isect.triIdx;
				contact.userData1 = m_data;
				contact.userData2 = b->m_data;
				contact.geomFlag = b->m_geomtree->GetTriFlag(isect.triIdx);
			}
		}
	}
}
