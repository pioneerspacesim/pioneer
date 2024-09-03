// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _COLLMESH_H
#define _COLLMESH_H

#include "Aabb.h"
#include "RefCounted.h"

#include <vector>

class GeomTree;

namespace Serializer {
	class Writer;
	class Reader;
} // namespace Serializer

//This simply stores the collision GeomTrees
//and AABB.
class CollMesh : public RefCounted {
public:
	CollMesh() :
		m_geomTree(0),
		m_totalTris(0)
	{}
	virtual ~CollMesh();

	inline Aabb &GetAabb() { return m_aabb; }

	inline double GetRadius() const { return m_aabb.GetRadius(); }
	inline void SetRadius(double v)
	{
		//0 radius = trouble
		m_aabb.radius = std::max(v, 0.1);
	}

	const std::vector<vector3f> &GetGeomTreeVertices() const;
	const Uint32 *GetGeomTreeIndices() const;
	const unsigned int *GetGeomTreeTriFlags() const;
	unsigned int GetGeomTreeNumTris() const;

	inline GeomTree *GetGeomTree() const { return m_geomTree; }

	inline void SetGeomTree(GeomTree *t)
	{
		assert(t);
		m_geomTree = t;
	}

	inline const std::vector<GeomTree *> &GetDynGeomTrees() const { return m_dynGeomTrees; }
	inline void AddDynGeomTree(GeomTree *t)
	{
		assert(t);
		m_dynGeomTrees.push_back(t);
	}

	//for statistics
	inline unsigned int GetNumTriangles() const { return m_totalTris; }
	inline void SetNumTriangles(unsigned int i) { m_totalTris = i; }

	void Save(Serializer::Writer &wr) const;
	void Load(Serializer::Reader &rd);

protected:
	Aabb m_aabb;
	GeomTree *m_geomTree;
	std::vector<GeomTree *> m_dynGeomTrees;
	unsigned int m_totalTris;
};

#endif
