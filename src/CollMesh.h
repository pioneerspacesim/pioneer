// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _COLLMESH_H
#define _COLLMESH_H
#include "RefCounted.h"
#include "Aabb.h"
#include "collider/GeomTree.h"

//This simply stores the collision GeomTrees
//and AABB.
class CollMesh : public RefCounted {
public:
	CollMesh()
	: m_geomTree(0)
	, m_totalTris(0)
	{ }
	virtual ~CollMesh() {
		for (auto it = m_dynGeomTrees.begin(); it != m_dynGeomTrees.end(); ++it)
			delete *it;
		delete m_geomTree;
	}
	virtual Aabb &GetAabb() { return m_aabb; }

	virtual double GetRadius() const { return m_aabb.GetRadius(); }
	virtual void SetRadius(double v) {
		//0 radius = trouble
		m_aabb.radius = std::max(v, 0.1);
	}

	virtual GeomTree *GetGeomTree() const { return m_geomTree; }
	void SetGeomTree(GeomTree *t) {
		assert(t);
		m_geomTree = t;
	}

	const std::vector<GeomTree*> &GetDynGeomTrees() const { return m_dynGeomTrees; }
	void AddDynGeomTree(GeomTree *t) {
		assert(t);
		m_dynGeomTrees.push_back(t);
	}

	//for statistics
	unsigned int GetNumTriangles() const { return m_totalTris; }
	void SetNumTriangles(unsigned int i) { m_totalTris = i; }

protected:
	Aabb m_aabb;
	GeomTree *m_geomTree;
	std::vector<GeomTree*> m_dynGeomTrees;
	unsigned int m_totalTris;
};

#endif
