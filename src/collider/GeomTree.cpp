// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "GeomTree.h"

#include "BVHTree.h"
#include "Weld.h"
#include "scenegraph/Serializer.h"
#include "../utils.h"

#include <array>
#include <map>

#pragma GCC optimize("O3")

GeomTree::~GeomTree()
{
}

GeomTree::GeomTree(const int numVerts, const int numTris, const std::vector<vector3f> &vertices, const std::vector<Uint32> &indices, const std::vector<Uint32> &triFlags) :
	m_numVertices(numVerts),
	m_numTris(numTris),
	m_vertices(vertices),
	m_indices(indices),
	m_triFlags(triFlags)
{
	PROFILE_SCOPED()

	assert(static_cast<int>(vertices.size()) == m_numVertices);

	m_aabb.min = vector3d(FLT_MAX, FLT_MAX, FLT_MAX);
	m_aabb.max = vector3d(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	// activeTris = tris we are still trying to put into leaves
	std::vector<int> activeTris;
	activeTris.reserve(numTris);
	for (int i = 0; i < numTris; i++) {
		activeTris.push_back(i * 3);
	}

	typedef std::map<std::pair<int, int>, int> EdgeType;
	EdgeType edges;
#define ADD_EDGE(_i1, _i2, _triflag)                     \
	if ((_i1) < (_i2))                                   \
		edges[std::pair<int, int>(_i1, _i2)] = _triflag; \
	else if ((_i1) > (_i2))                              \
		edges[std::pair<int, int>(_i2, _i1)] = _triflag;

	// eliminate duplicate vertices
	{
		std::vector<Uint32> xrefs;
		nv::Weld<vector3f> weld;
		weld(m_vertices, xrefs);
		m_numVertices = m_vertices.size();

		//Output("---   %d vertices welded\n", count - newCount);

		// Remap faces.
		const Uint32 faceCount = numTris;
		for (Uint32 f = 0; f < faceCount; f++) {
			const Uint32 idx = (f * 3);
			m_indices[idx + 0] = xrefs.at(m_indices[idx + 0]);
			m_indices[idx + 1] = xrefs.at(m_indices[idx + 1]);
			m_indices[idx + 2] = xrefs.at(m_indices[idx + 2]);
		}
	}

	// Get radius, m_aabb, and merge duplicate edges
	m_radius = 0;
	for (int i = 0; i < numTris; i++) {
		const Uint32 triflag = m_triFlags[i];
		const int vi1 = m_indices[3 * i + 0];
		const int vi2 = m_indices[3 * i + 1];
		const int vi3 = m_indices[3 * i + 2];

		ADD_EDGE(vi1, vi2, triflag);
		ADD_EDGE(vi1, vi3, triflag);
		ADD_EDGE(vi2, vi3, triflag);

		vector3d v[3];
		v[0] = vector3d(m_vertices[vi1]);
		v[1] = vector3d(m_vertices[vi2]);
		v[2] = vector3d(m_vertices[vi3]);
		m_aabb.Update(v[0]);
		m_aabb.Update(v[1]);
		m_aabb.Update(v[2]);
		for (int j = 0; j < 3; j++) {
			const double rad = v[j].x * v[j].x + v[j].y * v[j].y + v[j].z * v[j].z;
			if (rad > m_radius) m_radius = rad;
		}
	}
	m_radius = sqrt(m_radius);

	m_numEdges = edges.size();
	m_edges.resize(m_numEdges);
	m_edgeAABBs.reset(new AABBd[m_numEdges]);

	int pos = 0;
	typedef EdgeType::iterator MapPairIter;
	for (MapPairIter i = edges.begin(), iEnd = edges.end(); i != iEnd; ++i, pos++) {
		// precalc some jizz
		const std::pair<int, int> &vtx = (*i).first;
		const int triflag = (*i).second;
		const vector3f &v1 = m_vertices[vtx.first];
		const vector3f &v2 = m_vertices[vtx.second];
		vector3f dir = (v2 - v1);
		const float len = dir.Length();
		dir *= 1.0f / len;

		m_edges[pos].v1i = vtx.first;
		m_edges[pos].v2i = vtx.second;
		m_edges[pos].triFlag = triflag;
		m_edges[pos].len = len;
		m_edges[pos].dir = dir;

		// Build list of AABBs for edge BVH tree
		AABBd edgeAabb = AABBd::Invalid();
		edgeAabb.Update(vector3d(v1));
		edgeAabb.Update(vector3d(v2));
		m_edgeAABBs[pos] = edgeAabb;
	}

	// Compute the bounds for edge/triangle BVH tree
	AABBd bounds = AABBd { m_aabb.min, m_aabb.max};

	{
		PROFILE_SCOPED_DESC("GeomTree::CreateEdgeTree");
		m_edgeTree.reset(new BinnedAreaBVHTree());
		m_edgeTree->Build(bounds, m_edgeAABBs.get(), m_numEdges);
	}

	{
		PROFILE_SCOPED_DESC("GeomTree::CreateTriTree");

		m_triAABBs.reset(new AABBd[m_numTris]);
		for (size_t i = 0; i < m_numTris; i++) {
			const vector3d v0 = vector3d(m_vertices[m_indices[i * 3 + 0]]);
			const vector3d v1 = vector3d(m_vertices[m_indices[i * 3 + 1]]);
			const vector3d v2 = vector3d(m_vertices[m_indices[i * 3 + 2]]);

			AABBd aabb = { v0, v0 };
			aabb.Update(v1);
			aabb.Update(v2);
			m_triAABBs.get()[i] = aabb;
		}

		m_triTree.reset(new BinnedAreaBVHTree());
		m_triTree->Build(bounds, m_triAABBs.get(), m_numTris);
	}
}

GeomTree::GeomTree(Serializer::Reader &rd)
{
	PROFILE_SCOPED()
	m_numVertices = rd.Int32();
	m_numEdges = rd.Int32();
	m_numTris = rd.Int32();
	m_radius = rd.Double();

	m_aabb.max = rd.Vector3d();
	m_aabb.min = rd.Vector3d();
	m_aabb.radius = rd.Double();

	AABBd bounds = AABBd { m_aabb.min, m_aabb.max};

	{
		PROFILE_SCOPED_DESC("GeomTree::CreateEdgeTree");

		Aabb _oldAabb;
		const Uint32 numAabbs = rd.Int32();
		m_edgeAABBs.reset(new AABBd[numAabbs]);

		for (Uint32 iAabb = 0; iAabb < numAabbs; ++iAabb) {
			rd >> _oldAabb;
			m_edgeAABBs.get()[iAabb] = AABBd { _oldAabb.min, _oldAabb.max };
		}

		m_edgeTree.reset(new BinnedAreaBVHTree());
		m_edgeTree->Build(bounds, m_edgeAABBs.get(), m_numEdges);
	}

	{
		PROFILE_SCOPED_DESC("GeomTree::LoadEdges")
		m_edges.resize(m_numEdges);
		for (Sint32 iEdge = 0; iEdge < m_numEdges; ++iEdge) {
			auto &ed = m_edges[iEdge];
			rd >> ed.v1i >> ed.v2i >> ed.len >> ed.dir >> ed.triFlag;
		}
	}

	m_vertices.resize(m_numVertices);
	for (Sint32 iVert = 0; iVert < m_numVertices; ++iVert) {
		m_vertices[iVert] = rd.Vector3f();
	}

	const int numIndicies(m_numTris * 3);
	m_indices.resize(numIndicies);
	for (Sint32 iIndi = 0; iIndi < numIndicies; ++iIndi) {
		m_indices[iIndi] = rd.Int32();
	}

	m_triFlags.resize(m_numTris);
	for (Sint32 iTri = 0; iTri < m_numTris; ++iTri) {
		m_triFlags[iTri] = rd.Int32();
	}

	{
		PROFILE_SCOPED_DESC("GeomTree::CreateTriTree");

		// TODO: triangle AABBs should be written to the SGM file similarly to edge AABBs
		m_triAABBs.reset(new AABBd[m_numTris]);
		for (size_t i = 0; i < m_numTris; i++) {
			const vector3d v0 = vector3d(m_vertices[m_indices[i * 3 + 0]]);
			const vector3d v1 = vector3d(m_vertices[m_indices[i * 3 + 1]]);
			const vector3d v2 = vector3d(m_vertices[m_indices[i * 3 + 2]]);

			AABBd aabb = { v0, v0 };
			aabb.Update(v1);
			aabb.Update(v2);
			m_triAABBs.get()[i] = aabb;
		}

		m_triTree.reset(new BinnedAreaBVHTree());
		m_triTree->Build(bounds, m_triAABBs.get(), m_numTris);
	}
}

void GeomTree::TraceRay(const vector3f &start, const vector3f &dir, isect_t *isect) const
{
	std::vector<uint32_t> tri_isect;
	// Division by zero is intended and produces correct results in this case
	m_triTree->TraceRay(vector3d(start), 1.0 / vector3d(dir), FLT_MAX, tri_isect);

	for (uint32_t triIdx : tri_isect) {
		RayTriIntersect(1, start, &dir, triIdx * 3, isect);
	}
}

void GeomTree::RayTriIntersect(int numRays, const vector3f &origin, const vector3f *dirs, int triIdx, isect_t *isects) const
{
	// PROFILE_SCOPED()
	const vector3f a(m_vertices[m_indices[triIdx + 0]]);
	const vector3f b(m_vertices[m_indices[triIdx + 1]]);
	const vector3f c(m_vertices[m_indices[triIdx + 2]]);

	const vector3f n = (c - a).Cross(b - a);
	const float nominator = n.Dot(a - origin);

	const vector3f v0_cross((c - origin).Cross(b - origin));
	const vector3f v1_cross((b - origin).Cross(a - origin));
	const vector3f v2_cross((a - origin).Cross(c - origin));

	for (int i = 0; i < numRays; i++) {
		const float v0d = v0_cross.Dot(dirs[i]);
		const float v1d = v1_cross.Dot(dirs[i]);
		const float v2d = v2_cross.Dot(dirs[i]);

		if (((v0d > 0) && (v1d > 0) && (v2d > 0)) ||
			((v0d < 0) && (v1d < 0) && (v2d < 0))) {
			const float dist = nominator / dirs[i].Dot(n);
			if ((dist > 0) && (dist < isects[i].dist)) {
				isects[i].dist = dist;
				isects[i].triIdx = triIdx / 3;
			}
		}
	}
}

vector3f GeomTree::GetTriNormal(int triIdx) const
{
	PROFILE_SCOPED()
	const vector3f a(m_vertices[m_indices[3 * triIdx + 0]]);
	const vector3f b(m_vertices[m_indices[3 * triIdx + 1]]);
	const vector3f c(m_vertices[m_indices[3 * triIdx + 2]]);

	return (b - a).Cross(c - a).Normalized();
}

void GeomTree::Save(Serializer::Writer &wr) const
{
	PROFILE_SCOPED()
	wr.Int32(m_numVertices);
	wr.Int32(m_numEdges);
	wr.Int32(m_numTris);
	wr.Double(m_radius);

	wr.Vector3d(m_aabb.max);
	wr.Vector3d(m_aabb.min);
	wr.Double(m_aabb.radius);

	// TODO: the entire edge and triangle BVH should be written to disk and
	// loaded in future SGM versions rather than being re-computed on each load

	wr.Int32(m_numEdges);
	for (Sint32 iAabb = 0; iAabb < m_numEdges; ++iAabb) {
		AABBd &aabb = m_edgeAABBs.get()[iAabb];
		// Write back an old-style min-max-radius Aabb for compatibility with old SGM versions
		wr << aabb.min << aabb.max << double(0.0);
	}

	for (Sint32 iEdge = 0; iEdge < m_numEdges; ++iEdge) {
		auto &ed = m_edges[iEdge];
		wr << ed.v1i << ed.v2i << ed.len << ed.dir << ed.triFlag;
	}

	for (Sint32 iVert = 0; iVert < m_numVertices; ++iVert) {
		wr.Vector3f(m_vertices[iVert]);
	}

	for (Sint32 iIndi = 0; iIndi < (m_numTris * 3); ++iIndi) {
		wr.Int32(m_indices[iIndi]);
	}

	for (Sint32 iTri = 0; iTri < m_numTris; ++iTri) {
		wr.Int32(m_triFlags[iTri]);
	}
}
