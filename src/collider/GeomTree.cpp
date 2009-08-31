
#include <float.h>
#include <stdio.h>
#include <assert.h>
#include <set>
#ifdef _WIN32
#include <malloc.h>
#else
#include <alloca.h>
#endif
#include "../Aabb.h"
#include "GeomTree.h"
#include "BVHTree.h"

#define MIN(a,b) ((a)<(b) ? (a) : (b))
#define MAX(a,b) ((a)>(b) ? (a) : (b))

int GeomTree::stats_rayTriIntersections;


GeomTree::~GeomTree()
{
	delete [] m_edges;
	delete m_triTree;
	delete m_edgeTree;
}

#include <SDL.h>

GeomTree::GeomTree(int numVerts, int numTris, float *vertices, int *indices, int *triflags): m_numVertices(numVerts)
{
	m_vertices = vertices;
	m_indices = indices;
	m_triFlags = triflags;
	m_aabb.min = vector3d(FLT_MAX,FLT_MAX,FLT_MAX);
	m_aabb.max = vector3d(-FLT_MAX,-FLT_MAX,-FLT_MAX);

	// activeTris = tris we are still trying to put into leaves
	std::vector<int> activeTris;
	/* So, we ignore tris with flag >= 0x8000 */
	for (int i=0; i<numTris; i++) {
		if (triflags[i] >= 0x8000) continue;
		activeTris.push_back(i*3);
	}
	
	std::set< std::pair<int,int> > edges;
#define ADD_EDGE(_i1,_i2) \
	if ((_i1) < (_i2)) edges.insert(std::pair<int,int>(_i1,_i2)); \
	else if ((_i1) > (_i2)) edges.insert(std::pair<int,int>(_i2,_i1));

	// eliminate duplicate vertices
	for (int i=0; i<numVerts; i++) {
		vector3d v = vector3d(&m_vertices[3*i]);
		for (int j=i+1; j<numVerts; j++) {
			vector3d v2 = vector3d(&m_vertices[3*j]);
			if (v2 == v) {
				for (int k=0; k<numTris*3; k++) {
					if ((indices[k] == j) && (triflags[k/3] < 0x8000)) indices[k] = i;
				}
			}
		}
	}

	/* Get radius, m_aabb, and merge duplicate edges */
	m_radius = 0;
	for (int i=0; i<numTris; i++) {
		if (m_triFlags[i] < 0x8000) {
			int vi1 = 3*m_indices[3*i];
			int vi2 = 3*m_indices[3*i+1];
			int vi3 = 3*m_indices[3*i+2];

			ADD_EDGE(vi1, vi2);
			ADD_EDGE(vi1, vi3);
			ADD_EDGE(vi2, vi3);

			vector3d v[3];
			v[0] = vector3d(&m_vertices[vi1]);
			v[1] = vector3d(&m_vertices[vi2]);
			v[2] = vector3d(&m_vertices[vi3]);
			m_aabb.Update(v[0]);
			m_aabb.Update(v[1]);
			m_aabb.Update(v[2]);
			for (int j=0; j<3; j++) {
				double rad = v[j].x*v[j].x + v[j].y*v[j].y + v[j].z*v[j].z;
				if (rad>m_radius) m_radius = rad;
			}
		}
	}
	m_radius = sqrt(m_radius);
	
	Aabb aabb = m_aabb;
	
	Aabb *aabbs = new Aabb[activeTris.size()];
	for (int i=0; i<activeTris.size(); i++) {
		vector3d v1 = vector3d(&m_vertices[3*m_indices[activeTris[i]]]);
		vector3d v2 = vector3d(&m_vertices[3*m_indices[activeTris[i]+1]]);
		vector3d v3 = vector3d(&m_vertices[3*m_indices[activeTris[i]+2]]);
		aabbs[i].min = aabbs[i].max = v1;
		aabbs[i].Update(v2);
		aabbs[i].Update(v3);
	}
	
	int t = SDL_GetTicks();
	m_triTree = new BVHTree(activeTris.size(), &activeTris[0], aabbs);
	delete [] aabbs;
	printf("Tri tree of %d tris build in %dms\n", activeTris.size(), SDL_GetTicks() - t);
	
	m_numEdges = edges.size();
	m_edges = new Edge[m_numEdges];
	// to build Edge bvh tree with.
	aabbs = new Aabb[m_numEdges];
	int *edgeIdxs = new int[m_numEdges];

	int pos = 0;
	for (std::set< std::pair<int,int> >::iterator i = edges.begin();
			i != edges.end(); ++i, pos++) {
		// precalc some jizz
		vector3d v1 = vector3d(&m_vertices[(*i).first]);
		vector3d v2 = vector3d(&m_vertices[(*i).second]);
		vector3d dir = (v2-v1);
		double len = dir.Length();
		dir *= 1.0/len;

		m_edges[pos].v1i = (*i).first;
		m_edges[pos].v2i = (*i).second;
		m_edges[pos].len = (float)len;
		m_edges[pos].dir = vector3f((float)dir.x, (float)dir.y, (float)dir.z);
		
		edgeIdxs[pos] = pos;
		aabbs[pos].min = aabbs[pos].max = v1;
		aabbs[pos].Update(v2);
	}
	t = SDL_GetTicks();
	m_edgeTree = new BVHTree(m_numEdges, edgeIdxs, aabbs);
	printf("Edge tree of %d edges build in %dms\n", m_numEdges, SDL_GetTicks() - t);
}

void GeomTree::TraceRay(const vector3f &start, const vector3f &dir, isect_t *isect) const
{
	TraceCoherentRays(1, start, &dir, isect);
}

struct bvhstack {
	BVHNode *node;
	int activeRay;
};

/*
 * Bundle of rays with common origin
 */
inline void GeomTree::TraceCoherentRays(int numRays, const vector3f &a_origin, const vector3f *a_dirs, isect_t *isects) const
{
	BVHNode *currnode = m_triTree->GetRoot();
	bvhstack stack[32];
	int stackpos = -1;
	vector3f *invDirs = (vector3f*)alloca(sizeof(vector3f)*numRays);
	for (int i=0; i<numRays; i++) {
		invDirs[i] = vector3f(1.0f/a_dirs[i].x, 1.0f/a_dirs[i].y, 1.0f/a_dirs[i].z);
	}
	int activeRay = numRays - 1;

	for (;;) {
		while (!currnode->IsLeaf()) {
			do {
				if (currnode->SlabsRayAabbTest(a_origin, invDirs[activeRay], &isects[activeRay])) break;
			} while (activeRay-- > 0);
			if (activeRay < 0) goto pop_bstack;

			stackpos++;
			stack[stackpos].node = currnode->kids[1];
			stack[stackpos].activeRay = activeRay;
			currnode = currnode->kids[0];
		}
		// triangle intersection jizz
		for (int i=0; i<currnode->numTris; i++) {
			RayTriIntersect(activeRay+1, a_origin, a_dirs, currnode->triIndicesStart[i], isects);
		}
pop_bstack:
		if (stackpos < 0) break;
		currnode = stack[stackpos].node;
		activeRay = stack[stackpos].activeRay;
		stackpos--;
	}
}

void GeomTree::RayTriIntersect(int numRays, const vector3f &origin, const vector3f *dirs, int triIdx, isect_t *isects) const
{
	stats_rayTriIntersections++;
	const vector3f a(&m_vertices[3*m_indices[triIdx]]);
	const vector3f b(&m_vertices[3*m_indices[triIdx+1]]);
	const vector3f c(&m_vertices[3*m_indices[triIdx+2]]);

	vector3f v0_cross, v1_cross, v2_cross;
	const vector3f n = vector3f::Cross(c-a, b-a);
	const float nominator = vector3f::Dot(n, (a-origin));

	v0_cross = vector3f::Cross(c-origin, b-origin);
	v1_cross = vector3f::Cross(b-origin, a-origin);
	v2_cross = vector3f::Cross(a-origin, c-origin);

	for (int i=0; i<numRays; i++) {
		const float v0d = vector3f::Dot(v0_cross,dirs[i]);
		const float v1d = vector3f::Dot(v1_cross,dirs[i]);
		const float v2d = vector3f::Dot(v2_cross,dirs[i]);

		if (((v0d > 0) && (v1d > 0) && (v2d > 0)) ||
		    ((v0d < 0) && (v1d < 0) && (v2d < 0))) {
			const float dist = nominator / vector3f::Dot(dirs[i],n);
			if ((dist > 0) && (dist < isects[i].dist)) {
				isects[i].dist = dist;
				isects[i].triIdx = triIdx/3;
			}
		}
	}
}

vector3f GeomTree::GetTriNormal(int triIdx) const
{
	const vector3f a(&m_vertices[3*m_indices[3*triIdx]]);
	const vector3f b(&m_vertices[3*m_indices[3*triIdx+1]]);
	const vector3f c(&m_vertices[3*m_indices[3*triIdx+2]]);
	
	return vector3f::Cross(b-a, c-a).Normalized();
}
