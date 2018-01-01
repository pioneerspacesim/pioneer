// Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "CollMesh.h"

//This simply stores the collision GeomTrees
//and AABB.

void CollMesh::Save(Serializer::Writer &wr) const
{
	PROFILE_SCOPED()
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

void CollMesh::Load(Serializer::Reader &rd)
{
	PROFILE_SCOPED()
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
