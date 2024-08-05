// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _BVHTREE_H
#define _BVHTREE_H

#include "../Aabb.h"
#include "../vector3.h"
#include <vector>

/*
 * Base class for BVH trees with a single leaf per node.
 */
class SingleBVHTreeBase {
public:
	struct Node {
		AABBd aabb;
		uint32_t kids[2];
		uint32_t leafIndex;
		uint32_t treeHeight;
	};

	SingleBVHTreeBase();
	virtual ~SingleBVHTreeBase();

	void Clear();

	// Build the BVH structure from the given list of AABBs
	// Individual nodes will have leaf indices into this array
	void Build(const AABBd &bounds, AABBd *objAabbs, uint32_t numObjs);

	// Return a pointer to the node at the given index
	inline const Node *GetNode(uint32_t index) const { return m_nodes.data() + index; }

	// Compute a list of { objId, leafIndex } intersections and add it to the passed array
	void ComputeOverlap(uint32_t objId, const AABBd &objAabb, std::vector<std::pair<uint32_t, uint32_t>> &out_isect, uint32_t startNode = 0) const;
	// Trace a ray through this AABB and add the list of intersected leaves to the passed array
	void TraceRay(const vector3d &start, const vector3d &inv_dir, double len, std::vector<uint32_t> &out_isect, uint32_t startNode = 0) const;

	size_t GetNumNodes() const { return m_nodes.size(); }
	uint32_t GetHeight() const { return m_treeHeight; }
	double CalculateSAH() const;

protected:
	struct SortKey {
		vector3f center;
		uint32_t index;
	};

	vector3f ToSortSpace(vector3d point) const { return vector3f((point - m_boundsCenter) * m_inv_scale_factor); }

	// Override in final for specific tree construction behavior without vfunction call overhead
	virtual void BuildNode(Node *node, SortKey *keys, uint32_t numKeys, const AABBd *objAabbs, uint32_t height) = 0;

	std::vector<Node> m_nodes;
	uint32_t m_treeHeight;
	vector3d m_boundsCenter;
	double m_inv_scale_factor;
};


/*
 * Single-node binary-tree Bounding Volume Hierarchy tree.
 *
 * Uses a top-down construction heuristic to produce a "full" binary tree (i.e.
 * each node either is a leaf or is guaranteed to have two child nodes.)
 *
 * This tree should be used when rebuild speed is more important than maximal
 * query performance of the resulting tree (though it is quite fast to query).
 */
class SingleBVHTree final : public SingleBVHTreeBase {
public:
	SingleBVHTree() {};
	~SingleBVHTree() {};

protected:
	virtual void BuildNode(Node *node, SortKey *keys, uint32_t numKeys, const AABBd *objAabbs, uint32_t height) override final;
	uint32_t Partition(SortKey *keys, uint32_t numKeys, const AABBd &aabb, const AABBd *objAabbs);
};

/*
 * Single-node binary Bounding Volume Hierarchy tree built using a binned
 * Surface Area Heuristic construction metric.
 *
 * This tree is intended to be used when BVH query performance is of absolute
 * importance and building can be amortized or done only once during the
 * program lifetime.
 *
 * Provides a SAH improvement of up to 2x over the SingleBVHTree implementation,
 * but at a significantly higher rebuild cost.
 */
class BinnedAreaBVHTree final : public SingleBVHTreeBase {
public:
	BinnedAreaBVHTree() {};

protected:
	struct SortBin {
		AABBd bounds = {};
		uint32_t objCount = 0;
	};

	// 12 bins to sort triangles into provides a decent quality to performance
	// tradeoff, especially as this is run once for each axis.
	// This provides split lines at 1/2, 1/3, 1/4, and 1/6ths, which has been
	// experimentally shown to provide a better SAH for human-authored models.
	static constexpr size_t NUM_BINS = 12;

	virtual void BuildNode(Node *node, SortKey *keys, uint32_t numKeys, const AABBd *objAabbs, uint32_t height) override final;
	uint32_t Partition(SortKey *keys, uint32_t numKeys, const AABBd &aabb, const AABBd *objAabbs);
	float FindPivot(SortKey *keys, uint32_t numKeys, const AABBd &aabb, const AABBd *objAabbs, uint32_t axis, float &outCost) const;
};

#endif /* _BVHTREE_H */
