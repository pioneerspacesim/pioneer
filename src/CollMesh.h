// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _COLLMESH_H
#define _COLLMESH_H
#include "RefCounted.h"
#include "Aabb.h"
#include "collider/GeomTree.h"
#include "Serializer.h"

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

	void Save(Serializer::Writer &wr) const
	{
		wr.Vector3d(m_aabb.max);
		wr.Vector3d(m_aabb.min);
		wr.Double(m_aabb.radius);

		m_geomTree->Save(wr);

		wr.Int32(m_dynGeomTrees.size());
		for (auto it : m_dynGeomTrees) {
			it->Save(wr);
		}

		wr.Int32(m_totalTris);
	}

	void Load(Serializer::Reader &rd)
	{
		m_aabb.max = rd.Vector3d();
		m_aabb.min = rd.Vector3d();
		m_aabb.radius = rd.Double();

		m_geomTree = new GeomTree(rd);

		const Uint32 numDynGeomTrees = rd.Int32();
		m_dynGeomTrees.reserve(numDynGeomTrees);
		for (Uint32 it = 0; it < numDynGeomTrees; ++it) {
			m_dynGeomTrees.push_back(new GeomTree(rd));
		}

		m_totalTris = rd.Int32();
	}

protected:
	Aabb m_aabb;
	GeomTree *m_geomTree;
	std::vector<GeomTree*> m_dynGeomTrees;
	unsigned int m_totalTris;
};

#endif
