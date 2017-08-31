// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "../libs.h"
#include "GeomTree.h"
#include "BVHTree.h"
#include "Weld.h"

GeomTree::~GeomTree()
{
}

GeomTree::GeomTree(const int numVerts, const int numTris, const std::vector<vector3f> &vertices, const Uint32 *indices, const Uint32 *triflags)
: m_numVertices(numVerts)
, m_numTris(numTris)
, m_vertices(vertices)
{
	PROFILE_SCOPED()
	assert(static_cast<int>(vertices.size()) == m_numVertices);
	Profiler::Timer timer;
	timer.Start();

	const int numIndices = numTris * 3;
	m_indices.reserve(numIndices);
	for (int i = 0; i < numIndices; ++i) {
		m_indices.push_back(indices[i]);
	}

	m_triFlags.reserve(numTris);
	for (int i = 0; i < numTris; ++i) {
		m_triFlags.push_back(triflags[i]);
	}

	m_aabb.min = vector3d(FLT_MAX,FLT_MAX,FLT_MAX);
	m_aabb.max = vector3d(-FLT_MAX,-FLT_MAX,-FLT_MAX);

	// activeTris = tris we are still trying to put into leaves
	std::vector<int> activeTris;
	activeTris.reserve(numTris);
	for (int i=0; i<numTris; i++)
	{
		activeTris.push_back(i*3);
	}

	typedef std::map< std::pair<int,int>, int > EdgeType;
	EdgeType edges;
#define ADD_EDGE(_i1,_i2,_triflag) \
	if ((_i1) < (_i2)) edges[std::pair<int,int>(_i1,_i2)] = _triflag; \
	else if ((_i1) > (_i2)) edges[std::pair<int,int>(_i2,_i1)] = _triflag;

	// eliminate duplicate vertices
	{
		std::vector<Uint32> xrefs;
		nv::Weld<vector3f> weld;
		weld(m_vertices, xrefs);
		m_numVertices = m_vertices.size();

		//Output("---   %d vertices welded\n", count - newCount);

		// Remap faces.
		const Uint32 faceCount = numTris;
		for (Uint32 f = 0; f < faceCount; f++)
		{
			const Uint32 idx = (f * 3);
			m_indices[idx+0] = xrefs.at(m_indices[idx+0]);
			m_indices[idx+1] = xrefs.at(m_indices[idx+1]);
			m_indices[idx+2] = xrefs.at(m_indices[idx+2]);
		}
	}

	// Get radius, m_aabb, and merge duplicate edges
	m_radius = 0;
	for (int i=0; i<numTris; i++)
	{
		const Uint32 triflag = m_triFlags[i];
		const int vi1 = m_indices[3*i+0];
		const int vi2 = m_indices[3*i+1];
		const int vi3 = m_indices[3*i+2];

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
		for (int j=0; j<3; j++) {
			const double rad = v[j].x*v[j].x + v[j].y*v[j].y + v[j].z*v[j].z;
			if (rad>m_radius) m_radius = rad;
		}
	}
	m_radius = sqrt(m_radius);

	{
		Aabb *aabbs = new Aabb[activeTris.size()];
		for (Uint32 i = 0; i < activeTris.size(); i++)
		{
			const vector3d v1 = vector3d(m_vertices[m_indices[activeTris[i] + 0]]);
			const vector3d v2 = vector3d(m_vertices[m_indices[activeTris[i] + 1]]);
			const vector3d v3 = vector3d(m_vertices[m_indices[activeTris[i] + 2]]);
			aabbs[i].min = aabbs[i].max = v1;
			aabbs[i].Update(v2);
			aabbs[i].Update(v3);
		}

		//int t = SDL_GetTicks();
		m_triTree.reset(new BVHTree(activeTris.size(), &activeTris[0], aabbs));
		delete[] aabbs;
	}
	//Output("Tri tree of %d tris build in %dms\n", activeTris.size(), SDL_GetTicks() - t);

	m_numEdges = edges.size();
	m_edges.resize( m_numEdges );
	// to build Edge bvh tree with.
	m_aabbs.resize( m_numEdges );
	int *edgeIdxs = new int[m_numEdges];

	int pos = 0;
	typedef EdgeType::iterator MapPairIter;
	for (MapPairIter i = edges.begin(), iEnd = edges.end();	i != iEnd; ++i, pos++)
	{
		// precalc some jizz
		const std::pair<int, int> &vtx = (*i).first;
		const int triflag = (*i).second;
		const vector3f &v1 = m_vertices[vtx.first];
		const vector3f &v2 = m_vertices[vtx.second];
		vector3f dir = (v2-v1);
		const float len = dir.Length();
		dir *= 1.0f/len;

		m_edges[pos].v1i = vtx.first;
		m_edges[pos].v2i = vtx.second;
		m_edges[pos].triFlag = triflag;
		m_edges[pos].len = len;
		m_edges[pos].dir = dir;

		edgeIdxs[pos] = pos;
		m_aabbs[pos].min = m_aabbs[pos].max = vector3d(v1);
		m_aabbs[pos].Update(vector3d(v2));
	}

	//t = SDL_GetTicks();
	m_edgeTree.reset(new BVHTree(m_numEdges, edgeIdxs, &m_aabbs[0]));
	delete [] edgeIdxs;
	//Output("Edge tree of %d edges build in %dms\n", m_numEdges, SDL_GetTicks() - t);

	timer.Stop();
	//Output(" - - GeomTree::GeomTree took: %lf milliseconds\n", timer.millicycles());
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

	const Uint32 numAabbs = rd.Int32();
	m_aabbs.resize(numAabbs);
	for (Uint32 iAabb = 0; iAabb < numAabbs; ++iAabb) {
		m_aabbs[iAabb].max = rd.Vector3d();
		m_aabbs[iAabb].min = rd.Vector3d();
		m_aabbs[iAabb].radius = rd.Double();
	}

	m_edges.resize(m_numEdges);
	for (Sint32 iEdge = 0; iEdge < m_numEdges; ++iEdge) {
		m_edges[iEdge].Load(rd);
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

	// activeTris = tris we are still trying to put into leaves
	std::vector<int> activeTris;
	activeTris.reserve(m_numTris);
	for (int i = 0; i<m_numTris; i++)
	{
		activeTris.push_back(i * 3);
	}

	// regenerate the aabb data
	Aabb *aabbs = new Aabb[activeTris.size()];
	for (Uint32 i = 0; i<activeTris.size(); i++)
	{
		const vector3d v1 = vector3d(m_vertices[m_indices[activeTris[i] + 0]]);
		const vector3d v2 = vector3d(m_vertices[m_indices[activeTris[i] + 1]]);
		const vector3d v3 = vector3d(m_vertices[m_indices[activeTris[i] + 2]]);
		aabbs[i].min = aabbs[i].max = v1;
		aabbs[i].Update(v2);
		aabbs[i].Update(v3);
	}
	m_triTree.reset(new BVHTree(activeTris.size(), &activeTris[0], aabbs));
	delete[] aabbs;

	//
	int *edgeIdxs = new int[m_numEdges];
	memset(edgeIdxs, 0, sizeof(int)*m_numEdges);
	for (int i = 0; i<m_numEdges; i++) {
		edgeIdxs[i] = i;
	}
	m_edgeTree.reset(new BVHTree(m_numEdges, edgeIdxs, &m_aabbs[0]));
}

static bool SlabsRayAabbTest(const BVHNode *n, const vector3f &start, const vector3f &invDir, isect_t *isect)
{
	PROFILE_SCOPED()
	float
	l1      = (n->aabb.min.x - start.x) * invDir.x,
	l2      = (n->aabb.max.x - start.x) * invDir.x,
	lmin    = std::min(l1,l2),
	lmax    = std::max(l1,l2);

	l1      = (n->aabb.min.y - start.y) * invDir.y;
	l2      = (n->aabb.max.y - start.y) * invDir.y;
	lmin    = std::max(std::min(l1,l2), lmin);
	lmax    = std::min(std::max(l1,l2), lmax);

	l1      = (n->aabb.min.z - start.z) * invDir.z;
	l2      = (n->aabb.max.z - start.z) * invDir.z;
	lmin    = std::max(std::min(l1,l2), lmin);
	lmax    = std::min(std::max(l1,l2), lmax);

	return ((lmax >= 0.f) & (lmax >= lmin) & (lmin < isect->dist));
}

void GeomTree::TraceRay(const vector3f &start, const vector3f &dir, isect_t *isect) const
{
	PROFILE_SCOPED()
	TraceRay(m_triTree->GetRoot(), start, dir, isect);
}

void GeomTree::TraceRay(const BVHNode *currnode, const vector3f &a_origin, const vector3f &a_dir, isect_t *isect) const
{
	PROFILE_SCOPED()
	BVHNode *stack[32];
	int stackpos = -1;
	const vector3f invDir( // avoid division by zero please
		is_zero_exact(a_dir.x) ? 0.0f : (1.0f / a_dir.x),
		is_zero_exact(a_dir.y) ? 0.0f : (1.0f / a_dir.y),
		is_zero_exact(a_dir.z) ? 0.0f : (1.0f / a_dir.z));

	for (;;) {
		while (!currnode->IsLeaf()) {
			if (!SlabsRayAabbTest(currnode, a_origin, invDir, isect)) goto pop_bstack;

			stackpos++;
			stack[stackpos] = currnode->kids[1];
			currnode = currnode->kids[0];
		}
		// triangle intersection jizz
		for (int i=0; i<currnode->numTris; i++) {
			RayTriIntersect(1, a_origin, &a_dir, currnode->triIndicesStart[i], isect);
		}
pop_bstack:
		if (stackpos < 0) break;
		currnode = stack[stackpos];
		stackpos--;
	}
}

void GeomTree::RayTriIntersect(int numRays, const vector3f &origin, const vector3f *dirs, int triIdx, isect_t *isects) const
{
	PROFILE_SCOPED()
	const vector3f a(m_vertices[m_indices[triIdx+0]]);
	const vector3f b(m_vertices[m_indices[triIdx+1]]);
	const vector3f c(m_vertices[m_indices[triIdx+2]]);

	const vector3f n = (c-a).Cross(b-a);
	const float nominator = n.Dot(a-origin);

	const vector3f v0_cross((c-origin).Cross(b-origin));
	const vector3f v1_cross((b-origin).Cross(a-origin));
	const vector3f v2_cross((a-origin).Cross(c-origin));

	for (int i=0; i<numRays; i++) {
		const float v0d = v0_cross.Dot(dirs[i]);
		const float v1d = v1_cross.Dot(dirs[i]);
		const float v2d = v2_cross.Dot(dirs[i]);

		if ( ((v0d > 0) && (v1d > 0) && (v2d > 0)) ||
			 ((v0d < 0) && (v1d < 0) && (v2d < 0)) ) {
			const float dist = nominator / dirs[i].Dot(n);
			if ((dist > 0) && (dist < isects[i].dist)) {
				isects[i].dist = dist;
				isects[i].triIdx = triIdx/3;
			}
		}
	}
}

vector3f GeomTree::GetTriNormal(int triIdx) const
{
	PROFILE_SCOPED()
	const vector3f a(m_vertices[m_indices[3*triIdx+0]]);
	const vector3f b(m_vertices[m_indices[3*triIdx+1]]);
	const vector3f c(m_vertices[m_indices[3*triIdx+2]]);

	return (b-a).Cross(c-a).Normalized();
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

	wr.Int32(m_numEdges);
	for (Sint32 iAabb = 0; iAabb < m_numEdges; ++iAabb) {
		wr.Vector3d(m_aabbs[iAabb].max);
		wr.Vector3d(m_aabbs[iAabb].min);
		wr.Double(m_aabbs[iAabb].radius);
	}

	for (Sint32 iEdge = 0; iEdge < m_numEdges; ++iEdge) {
		m_edges[iEdge].Save(wr);
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
