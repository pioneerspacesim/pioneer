// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _BVHTREE_H
#define _BVHTREE_H

#include "../Aabb.h"
#include "../vector3.h"
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
	size_t GetNumNodes() const { return m_nodeAllocPos; }
	double CalculateSAH() const;

private:
	void BuildNode(BVHNode *node,
		const objPtr_t *objPtrs,
		const Aabb *objAabbs,
		std::vector<objPtr_t> &activeObjIdxs);
	void MakeLeaf(BVHNode *node, const objPtr_t *objPtrs, std::vector<objPtr_t> &objs);
	BVHNode *AllocNode();
	BVHNode *m_root;
	objPtr_t *m_objPtrAlloc;
	size_t m_objPtrAllocPos;
	size_t m_objPtrAllocMax;

	BVHNode *m_bvhNodes;
	size_t m_nodeAllocPos;
	size_t m_nodeAllocMax;
};

/*
 * Single-node binary-tree Bounding Volume Hierarchy tree.
 *
 * Uses a top-down construction heuristic to produce a "full" binary tree (i.e.
 * each node either is a leaf or is guaranteed to have two child nodes.)
 */
class SingleBVHTree {
public:
	struct Node {
		AABBd aabb;
		uint32_t kids[2];
		uint32_t leafIndex;
	};

	SingleBVHTree();

	void Clear();

	// Build the BVH structure from the given list of AABBs
	// Individual nodes will have leaf indices into this array
	void Build(const AABBd &bounds, AABBd *objAabbs, uint32_t numObjs);

	// Return a pointer to the node at the given index
	const Node *GetNode(uint32_t index) const { return m_nodes.data() + index; }

	// Compute a list of { objId, leafIndex } intersections and add it to the passed array
	void ComputeOverlap(uint32_t objId, const AABBd &objAabb, std::vector<std::pair<uint32_t, uint32_t>> &out_isect) const;
	// Trace a ray through this AABB and add the list of intersected leaves to the passed array
	void TraceRay(const vector3d &start, const vector3d &inv_dir, double len, std::vector<uint32_t> &out_isect) const;

	size_t GetNumNodes() const { return m_nodes.size(); }
	uint32_t GetHeight() const { return m_treeHeight; }
	double CalculateSAH() const;

private:
	struct SortKey {
		vector3f center;
		uint32_t index;
	};

	void BuildNode(Node *node, SortKey *keys, uint32_t numKeys, AABBd *objAabbs, uint32_t height);
	uint32_t Partition(SortKey *keys, uint32_t numKeys, const AABBd &aabb, AABBd *objAabbs);

	std::vector<Node> m_nodes;
	uint32_t m_treeHeight;
	vector3d m_boundsCenter;
	double m_inv_scale_factor;
};

#endif /* _BVHTREE_H */
