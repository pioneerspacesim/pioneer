// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "CollMesh.h"

#include "scenegraph/Serializer.h"
#include "collider/GeomTree.h"
#include "profiler/Profiler.h"

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

CollMesh::~CollMesh()
{
	for (auto it = m_dynGeomTrees.begin(); it != m_dynGeomTrees.end(); ++it)
		delete *it;
	delete m_geomTree;
}

const std::vector<vector3f> &CollMesh::GetGeomTreeVertices() const
{
	return m_geomTree->GetVertices();
}

const Uint32 *CollMesh::GetGeomTreeIndices() const
{
	return m_geomTree->GetIndices();
}

const unsigned int *CollMesh::GetGeomTreeTriFlags() const
{
	return m_geomTree->GetTriFlags();
}

unsigned int CollMesh::GetGeomTreeNumTris() const
{
	return m_geomTree->GetNumTris();
}
