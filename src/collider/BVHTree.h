// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _BVHTREE_H
#define _BVHTREE_H

#include "../Aabb.h"
#include "../utils.h"
#include "../vector3.h"
#include <assert.h>
#include <vector>

struct BVHNode {
	Aabb aabb;

	/* if triIndicesStart == 0 then not leaf,
	 * kids[] valid */
	int numTris;
	int *triIndicesStart;

	BVHNode *kids[2];

	BVHNode() :
		numTris(0),
		triIndicesStart(nullptr)
	{
		kids[0] = nullptr;
		kids[1] = nullptr;
	}
	bool IsLeaf() const
	{
		return triIndicesStart != nullptr;
	}
};

class BVHTree {
public:
	typedef int objPtr_t;
	BVHTree(const int numObjs, const objPtr_t *objPtrs, const Aabb *objAabbs);
	~BVHTree()
	{
		delete[] m_objPtrAlloc;
		delete[] m_bvhNodes;
	}
	BVHNode *GetRoot() { return m_root; }

private:
	void BuildNode(BVHNode *node,
		const objPtr_t *objPtrs,
		const Aabb *objAabbs,
		std::vector<objPtr_t> &activeObjIdxs);
	void MakeLeaf(BVHNode *node, const objPtr_t *objPtrs, std::vector<objPtr_t> &objs);
	BVHNode *AllocNode()
	{
		if (m_nodeAllocPos >= m_nodeAllocMax) Error("Out of space in m_bvhNodes.");
		return &m_bvhNodes[m_nodeAllocPos++];
	}
	BVHNode *m_root;
	objPtr_t *m_objPtrAlloc;
	size_t m_objPtrAllocPos;
	size_t m_objPtrAllocMax;

	BVHNode *m_bvhNodes;
	size_t m_nodeAllocPos;
	size_t m_nodeAllocMax;
};

#endif /* _BVHTREE_H */
