// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _COLLMESH_H
#define _COLLMESH_H
#include "RefCounted.h"
#include "Aabb.h"
#include "collider/GeomTree.h"

class CollMesh : public RefCounted {
public:
	CollMesh() : m_geomTree(0) { }
	virtual ~CollMesh() {
		delete m_geomTree;
	}
	virtual Aabb &GetAabb() { return m_aabb; }
	virtual double GetRadius() const { return m_aabb.GetRadius(); }
	virtual void SetRadius(double v) { m_aabb.radius = std::max(v, 0.1); } //0 radius = trouble
	virtual GeomTree *GetGeomTree() const { return m_geomTree; }
	void SetGeomTree(GeomTree *t) { m_geomTree = t; }

	std::vector<vector3f> m_vertices;
	std::vector<int> m_indices;
	std::vector<unsigned int> m_flags; //1 per triangle

protected:
	Aabb m_aabb;
	GeomTree *m_geomTree;
};

#endif
