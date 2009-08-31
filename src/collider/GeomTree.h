#ifndef _GEOMTREE_H
#define _GEOMTREE_H

#include <vector>
#include "../Aabb.h"

struct tri_t;

struct isect_t {
	// triIdx = -1 if no intersection
	int triIdx;
	float dist;
};

class BVHTree;

class GeomTree {
public:
	GeomTree(int numVerts, int numTris, float *vertices, int *indices, int *triflags);
	~GeomTree();
	const Aabb &GetAabb() const { return m_aabb; }
	// dir should be unit length,
	// isect.dist should be ray length
	// isect.triIdx should be -1 unless repeat calls with same isect_t
	void TraceRay(const vector3f &start, const vector3f &dir, isect_t *isect) const;
	void TraceCoherentRays(int numRays, const vector3f &a_origin, const vector3f *a_dirs, isect_t *isects) const;
	vector3f GetTriNormal(int triIdx) const;
	int GetTriFlag(int triIdx) const { return m_triFlags[triIdx]; }
	double GetRadius() const { return m_radius; }
	struct Edge {
		int v1i, v2i;
		float len;
		vector3f dir;
	};
	const Edge *GetEdges() const { return m_edges; }
	int GetNumEdges() const { return m_numEdges; }
	
	const int m_numVertices;
	const float *m_vertices;
	static int stats_rayTriIntersections;

private:
	void RayTriIntersect(int numRays, const vector3f &origin, const vector3f *dirs, int triIdx, isect_t *isects) const;

	double m_radius;
	Aabb m_aabb;
	
	int m_numEdges;
	Edge *m_edges;

	const int *m_indices;
	const int *m_triFlags;

	BVHTree *m_triTree;
	BVHTree *m_edgeTree;
};

#endif /* _GEOMTREE_H */
