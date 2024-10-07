// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "BVHTree.h"

#include "Aabb.h"
#include "MathUtil.h"
#include "core/Log.h"
#include "core/macros.h"
#include "profiler/Profiler.h"

SingleBVHTreeBase::SingleBVHTreeBase() :
	m_treeHeight(0),
	m_boundsCenter(0.0, 0.0, 0.0),
	m_inv_scale_factor(1.0)
{
	Clear();
}

SingleBVHTreeBase::~SingleBVHTreeBase()
{
}

void SingleBVHTreeBase::Clear()
{
	m_nodes.clear();

	// Build a default / invalid node for this BVHTree
	Node invalid = {};
	invalid.aabb = AABBd::Invalid();

	m_treeHeight = 1;
	m_nodes.push_back(invalid);
}

void SingleBVHTreeBase::Build(const AABBd &bounds, AABBd *objAabbs, uint32_t numObjs)
{
	PROFILE_SCOPED()

	// bound on a fully-populated perfect binary tree: 2 * 2^(log2(n)) - 1
	// reserve less than that, assuming this won't be a perfect tree
	// experimental data shows generally 2 nodes per leaf object
	m_nodes.clear();
	m_nodes.reserve(2 * numObjs + 1);

	// compute a remapping term to express object positions with highest precision using single floating point
	// use the average of the bounding volume to remap into [-1 .. 1] space
	m_boundsCenter = (bounds.max + bounds.min) * 0.5;
	vector3d inv_scale = 1.0 / (bounds.max - m_boundsCenter);
	m_inv_scale_factor = (inv_scale.x + inv_scale.y + inv_scale.z) / 3.0;

	// Build a vector of sort keys for individual nodes (position within system)
	std::vector<SortKey> sortKeys(numObjs);
	for (size_t idx = 0; idx < numObjs; idx++) {
		sortKeys[idx].center = ToSortSpace(objAabbs[idx].max * 0.5 + objAabbs[idx].min * 0.5);
		sortKeys[idx].index = idx;
	}

	BuildNode(&m_nodes.emplace_back(), sortKeys.data(), numObjs, objAabbs, 0);
}

void SingleBVHTreeBase::ComputeOverlap(uint32_t nodeId, const AABBd &nodeAabb, std::vector<std::pair<uint32_t, uint32_t>> &out_isect, uint32_t startNode) const
{
	PROFILE_SCOPED()

	int32_t stackLevel = 0;
	uint32_t *stack = stackalloc(uint32_t, m_treeHeight + 1);
	// Push the root node
	stack[stackLevel++] = startNode;

	while (stackLevel > 0) {
		const SingleBVHTreeBase::Node *node = &m_nodes[stack[--stackLevel]];

		if (!nodeAabb.Intersects(node->aabb))
			continue;

		if (node->kids[0] == 0) {
			out_isect.push_back({ nodeId, node->leafIndex });
			continue;
		}

		// Push in reverse order for pre-order traversal
		stack[stackLevel++] = node->kids[1];
		stack[stackLevel++] = node->kids[0];
	}
}

void SingleBVHTreeBase::TraceRay(const vector3d &start, const vector3d &inv_dir, double len, std::vector<uint32_t> &out_isect, uint32_t startNode) const
{
	// PROFILE_SCOPED() // Called often, only enable when needed

	int32_t stackLevel = 0;
	uint32_t *stack = stackalloc(uint32_t, m_treeHeight + 1);
	stack[stackLevel++] = startNode;

	while (stackLevel > 0) {
		uint32_t nodeIdx = stack[--stackLevel];
		const SingleBVHTreeBase::Node *node = &m_nodes[nodeIdx];

		// Didn't intersect with the node, ignore it
		if (!node->aabb.IntersectsRay(start, inv_dir, len))
			continue;

		// Leaf node - mark intersection and continue
		if (node->kids[0] == 0) {
			out_isect.push_back(node->leafIndex);
			continue;
		}

		stack[stackLevel++] = node->kids[1];
		stack[stackLevel++] = node->kids[0];
	}
}

double SingleBVHTreeBase::CalculateSAH() const
{
	double outSAH = 0.0;
	auto *rootNode = GetNode(0);
	std::vector<uint32_t> m_nodeStack = { 0 };

	while (!m_nodeStack.empty()) {
		auto *node = GetNode(m_nodeStack.back());
		m_nodeStack.pop_back();

		if (node->kids[0])
			m_nodeStack.push_back(node->kids[0]);
		if (node->kids[1])
			m_nodeStack.push_back(node->kids[1]);

		// Cost function according to https://users.aalto.fi/~laines9/publications/aila2013hpg_paper.pdf Eq. 1
		outSAH += (node->kids[0] ? 1.2 : 1.0) * node->aabb.SurfaceArea();
	}

	double rootArea = rootNode->aabb.SurfaceArea();

	// Perform 1 / Aroot * ( SAH sums )
	outSAH /= rootArea;

	// Remove the (normalized) SAH cost of the root node from the result
	// The SAH metric doesn't include the cost of the root node
	outSAH -= 1.0;

	return outSAH;
}

// ============================================================================


void SingleBVHTree::BuildNode(Node *node, SortKey *keys, uint32_t numKeys, const AABBd *objAabbs, uint32_t height)
{
	// PROFILE_SCOPED()

	AABBd aabb = AABBd::Invalid();
	m_treeHeight = std::max(++height, m_treeHeight);

	// Compute the AABB for this node
	for (size_t idx = 0; idx < numKeys; idx++) {
		aabb.Update(objAabbs[keys[idx].index]);
	}

	node->aabb = aabb;
	node->leafIndex = 0;
	node->treeHeight = height;

	// With only a single item, this is a leaf node. Set both child pointers to 0
	if (numKeys == 1) {
		node->kids[0] = node->kids[1] = 0;
		node->leafIndex = keys[0].index;
		return;
	}

	uint32_t startIdx = Partition(keys, numKeys, aabb, objAabbs);

	// If partitioning multiple nodes failed, just split the list down the middle
	// This should never occur except in extreme degenerate cases
	if (std::min(startIdx, numKeys - startIdx) == 0)
		startIdx = numKeys >> 1;

	// Allocate both child nodes together for cache optimization
	node->kids[0] = m_nodes.size();
	Node *leftNode = &m_nodes.emplace_back();

	node->kids[1] = m_nodes.size();
	Node *rightNode = &m_nodes.emplace_back();

	// Descend into left/right node trees
	// This could potentially be made into a non-recursive stack-based algorithm
	BuildNode(leftNode, keys, startIdx, objAabbs, height);
	BuildNode(rightNode, keys + startIdx, numKeys - startIdx, objAabbs, height);
}

uint32_t SingleBVHTree::Partition(SortKey *keys, uint32_t numKeys, const AABBd &aabb, const AABBd *objAabbs)
{
	// PROFILE_SCOPED()

	// divide by longest axis
	uint32_t axis = 2;
	const vector3d axislen = aabb.max - aabb.min;
	if ((axislen.x > axislen.y) && (axislen.x > axislen.z))
		axis = 0;
	else if (axislen.y > axislen.z)
		axis = 1;

	const float pivot = (0.5 * (aabb.max[axis] + aabb.min[axis]) - m_boundsCenter[axis]) * m_inv_scale_factor;

	// Simple O(n) sort algorithm to sort all objects according to side of pivot
	uint32_t startIdx = 0;
	uint32_t endIdx = numKeys;

	// It is ~10% faster to sort the indices than to sort the whole AABB array
	// (cache hit rate vs. memory bandwidth)
	// Sorting in general is extremely fast.
	while (startIdx < endIdx) {
		if (keys[startIdx].center[axis] < pivot) {
			startIdx++;
		} else {
			endIdx--;
			std::swap(keys[startIdx], keys[endIdx]);
		}
	}

	return startIdx;
}


// ============================================================================


void BinnedAreaBVHTree::BuildNode(Node *node, SortKey *keys, uint32_t numKeys, const AABBd *objAabbs, uint32_t height)
{
	// PROFILE_SCOPED()

	AABBd aabb = AABBd::Invalid();
	m_treeHeight = std::max(++height, m_treeHeight);

	// Compute the AABB for this node
	for (size_t idx = 0; idx < numKeys; idx++) {
		aabb.Update(objAabbs[keys[idx].index]);
	}

	node->aabb = aabb;
	node->leafIndex = 0;
	node->treeHeight = height;

	// With only a single item, this is a leaf node. Set both child pointers to 0
	if (numKeys == 1) {
		node->kids[0] = node->kids[1] = 0;
		node->leafIndex = keys[0].index;
		return;
	}

	uint32_t startIdx = Partition(keys, numKeys, aabb, objAabbs);

	// If partitioning multiple nodes failed, just split the list down the middle
	// This should never occur except in extreme degenerate cases
	if (std::min(startIdx, numKeys - startIdx) == 0)
		startIdx = numKeys >> 1;

	// Allocate both child nodes together for cache optimization
	node->kids[0] = m_nodes.size();
	Node *leftNode = &m_nodes.emplace_back();

	node->kids[1] = m_nodes.size();
	Node *rightNode = &m_nodes.emplace_back();

	// Descend into left/right node trees
	// This could potentially be made into a non-recursive stack-based algorithm
	BuildNode(leftNode, keys, startIdx, objAabbs, height);
	BuildNode(rightNode, keys + startIdx, numKeys - startIdx, objAabbs, height);
}

uint32_t BinnedAreaBVHTree::Partition(SortKey *keys, uint32_t numKeys, const AABBd &aabb, const AABBd *objAabbs)
{
	uint32_t axis = 0;
	float pivot = 0.0;
	float cost = FLT_MAX;

	for (uint32_t i = 0; i < 3; i++) {
		float newCost = FLT_MAX;
		float newPivot = FindPivot(keys, numKeys, aabb, objAabbs, i, newCost);
		if (newCost < cost) {
			axis = i;
			pivot = newPivot;
			cost = newCost;
		}
	}

	// Simple O(n) sort algorithm to sort all objects according to side of pivot
	uint32_t startIdx = 0;
	uint32_t endIdx = numKeys;

	// It is ~10% faster to sort the indices than to sort the whole AABB array
	// (cache hit rate vs. memory bandwidth)
	// Sorting in general is extremely fast.
	while (startIdx < endIdx) {
		if (keys[startIdx].center[axis] < pivot) {
			startIdx++;
		} else {
			endIdx--;
			std::swap(keys[startIdx], keys[endIdx]);
		}
	}

	return startIdx;
}

float BinnedAreaBVHTree::FindPivot(SortKey *keys, uint32_t numKeys, const AABBd &aabb, const AABBd *objAabbs, uint32_t axis, float &outCost) const
{
	// Uses binned surface-area-heuristic construction:
	// https://jacco.ompf2.com/2022/04/21/how-to-build-a-bvh-part-3-quick-builds/

	SortBin bins[NUM_BINS] = {};

	float boundsMin = ToSortSpace(aabb.min)[axis];
	float boundsMax = ToSortSpace(aabb.max)[axis];

	float scale = NUM_BINS / (boundsMax - boundsMin);
	float invScale = (boundsMax - boundsMin) / NUM_BINS;

	// Bin each object, using the sort key as its centroid
	for (size_t i = 0; i < numKeys; i++) {
		size_t bin_idx = std::min(NUM_BINS - 1, size_t((keys[i].center[axis] - boundsMin) * scale));
		bins[bin_idx].objCount++;
		bins[bin_idx].bounds.Update(objAabbs[keys[i].index]);
	}

	constexpr int NUM_PLANES = NUM_BINS - 1;

	float planeCost[NUM_PLANES];
	float bestCost = FLT_MAX;
	float pivot = 0.0;

	AABBd leftBox = AABBd::Invalid();
	AABBd rightBox = AABBd::Invalid();
	uint32_t leftSum = 0;
	uint32_t rightSum = 0;

	// Calculate the left-side SAH cost of each splitting plane
	for (int i = 0; i < NUM_PLANES; i++) {
		leftSum += bins[i].objCount;
		leftBox.Update(bins[i].bounds);
		planeCost[i] = leftSum * leftBox.SurfaceArea();
	}

	// Calculate the right-side SAH cost of each splitting plane
	for (int i = NUM_PLANES - 1; i >= 0; i--) {
		rightSum += bins[i + 1].objCount;
		rightBox.Update(bins[i + 1].bounds);
		planeCost[i] += rightSum * rightBox.SurfaceArea();

		if (planeCost[i] < bestCost) {
			pivot = boundsMin + invScale * (i + 1);
			bestCost = planeCost[i];
		}
	}

	outCost = bestCost;
	return pivot;
}
