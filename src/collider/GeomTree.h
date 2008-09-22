#ifndef _GEOMTREE_H
#define _GEOMTREE_H

#include "../Aabb.h"

class BIHNode;
class tri_t;

struct isect_t {
	// triIdx = -1 if no intersection
	int triIdx;
	float dist;
};

class GeomTree {
public:
	GeomTree(int numTris, float *vertices, int *indices);
	~GeomTree();
	Aabb GetAabb() { return m_aabb; }
	void TraceRay(vector3f &start, vector3f &dir, isect_t *isect);
private:
	friend class BIHNode;
	void RayTriIntersect(vector3f &a_origin, vector3f &a_dir, int triIdx, isect_t *isect);
	void TraverseRay(vector3f &a_origin, vector3f &a_dir, isect_t *isect);
	BIHNode *AllocNode();
	void BihTreeGhBuild(BIHNode* a_node, Aabb &a_box, Aabb &a_splitBox, int a_depth, int a_prims);

	Aabb m_aabb;
	BIHNode *m_nodes;
	int m_nodesAllocPos;
	int m_nodesAllocSize;
	tri_t *m_triAlloc;
	int m_triAllocPos;
	int m_triAllocSize;

	const float *m_vertices;
	const int *m_indices;
};

#endif /* _GEOMTREE_H */
