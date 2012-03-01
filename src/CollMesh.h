#ifndef _COLLMESH_H
#define _COLLMESH_H

#include "Aabb.h"

class CollMesh {
public:
	CollMesh() : m_radius(0.f) { }
	virtual ~CollMesh() { };
	virtual const Aabb &GetAabb() const { return m_aabb; }
	virtual float GetBoundingRadius() const { return m_radius; }
protected:
	Aabb m_aabb;
	float m_radius;
};

#endif
