// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "BVHTree.h"

#include "Aabb.h"
#include "MathUtil.h"
#include "core/Log.h"
#include "core/macros.h"
#include "profiler/Profiler.h"

const int MAX_SPLITPOS_RETRIES = 15;

BVHTree::BVHTree(int numObjs, const objPtr_t *objPtrs, const Aabb *objAabbs)
{
	PROFILE_SCOPED()
	Profiler::Timer timer;
	timer.Start();

	std::vector<int> activeObjIdxs(numObjs);
	for (int i = 0; i < numObjs; i++)
		activeObjIdxs[i] = i;

	m_objPtrAlloc = new objPtr_t[numObjs];
	m_objPtrAllocPos = 0;
	m_objPtrAllocMax = numObjs;

	m_bvhNodes = new BVHNode[numObjs * 2 + 1];
	m_nodeAllocPos = 0;
	m_nodeAllocMax = numObjs * 2 + 1;

	m_root = AllocNode();
	BuildNode(m_root, objPtrs, objAabbs, activeObjIdxs);

	timer.Stop();
	//Log::Debug(" - - - BVHTree::BVHTree took: {} milliseconds\n", timer.millicycles());
}

void BVHTree::MakeLeaf(BVHNode *node, const objPtr_t *objPtrs, std::vector<objPtr_t> &objs)
{
	const size_t numTris = objs.size();
	if (numTris <= 0)
		Log::Error("MakeLeaf called with no elements in objs.");

	if (numTris > m_objPtrAllocMax - m_objPtrAllocPos) {
		Log::Error("Out of space in m_objPtrAlloc. Left: {}; required: {}.", m_objPtrAllocMax - m_objPtrAllocPos, numTris);
	}

	node->numTris = numTris;
	node->triIndicesStart = &m_objPtrAlloc[m_objPtrAllocPos];
	//if (objs.size()>3) Log::Debug("fat node {}\n", objs.size());

	// copy tri indices to the stinking flat array
	for (int i = numTris - 1; i >= 0; i--) {
		m_objPtrAlloc[m_objPtrAllocPos++] = objPtrs[objs[i]];
	}
}

void BVHTree::BuildNode(BVHNode *node,
	const objPtr_t *objPtrs,
	const Aabb *objAabbs,
	std::vector<objPtr_t> &activeObjIdx)
{
	const int numTris = activeObjIdx.size();
	if (numTris <= 0)
		Log::Error("BuildNode called with no elements in activeObjIndex.");

	std::vector<int> splitSides(numTris);

	Aabb aabb;
	aabb.min = vector3d(FLT_MAX, FLT_MAX, FLT_MAX);
	aabb.max = vector3d(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (int i = 0; i < numTris; i++) {
		const int idx = activeObjIdx[i];
		aabb.Update(objAabbs[idx].min);
		aabb.Update(objAabbs[idx].max);
	}
	node->numTris = numTris;
	node->aabb = aabb;

	if (numTris == 1) {
		MakeLeaf(node, objPtrs, activeObjIdx);
		return;
	}

	Aabb splitBox = aabb;
	double splitPos;
	int splitAxis;
	int s1count, s2count;
	int attempt = 0;

	for (;;) {
		splitAxis = 0;

		const vector3d boxSize = splitBox.max - splitBox.min;

		if (boxSize[1] > boxSize[0]) splitAxis = 1;
		if ((boxSize[2] > boxSize[1]) && (boxSize[2] > boxSize[0])) splitAxis = 2;

		// split pos in middle of splitBox
		splitPos = 0.5f * (splitBox.min[splitAxis] + splitBox.max[splitAxis]);
		s1count = 0, s2count = 0;

		for (int i = 0; i < numTris; i++) {
			const int idx = activeObjIdx[i];
			const double mid = 0.5 * (objAabbs[idx].min[splitAxis] + objAabbs[idx].max[splitAxis]);
			if (mid < splitPos) {
				splitSides[i] = 0;
				s1count++;
			} else {
				splitSides[i] = 1;
				s2count++;
			}
		}

		if (s1count == numTris) {
			if (attempt < MAX_SPLITPOS_RETRIES) {
				// try splitting at average point
				splitBox.max[splitAxis] = splitPos;
				attempt++;
				continue;
			} else {
				MakeLeaf(node, objPtrs, activeObjIdx);
				return;
			}
		} else if (s2count == numTris) {
			if (attempt < MAX_SPLITPOS_RETRIES) {
				// try splitting at average point
				splitBox.min[splitAxis] = splitPos;
				attempt++;
				continue;
			} else {
				MakeLeaf(node, objPtrs, activeObjIdx);
				return;
			}
		}
		break;
	}

	std::vector<int> side[2];
	side[0].reserve(numTris);
	side[1].reserve(numTris);

	for (int i = 0; i < numTris; i++) {
		side[splitSides[i]].push_back(activeObjIdx[i]);
	}

	// recurse!
	node->triIndicesStart = 0;
	node->kids[0] = AllocNode();
	node->kids[1] = AllocNode();

	BuildNode(node->kids[0], objPtrs, objAabbs, side[0]);
	BuildNode(node->kids[1], objPtrs, objAabbs, side[1]);
}

BVHNode *BVHTree::AllocNode()
{
	if (m_nodeAllocPos >= m_nodeAllocMax)
		Log::Error("Out of space in m_bvhNodes.");
	return &m_bvhNodes[m_nodeAllocPos++];
}

double BVHTree::CalculateSAH() const
{
	double outSAH = 0.0;
	std::vector<BVHNode *> m_nodeStack = { m_root };

	while (!m_nodeStack.empty()) {
		BVHNode *node = m_nodeStack.back();
		m_nodeStack.pop_back();

		if (!node->IsLeaf() && node->kids[0])
			m_nodeStack.push_back(node->kids[0]);
		if (!node->IsLeaf() && node->kids[1])
			m_nodeStack.push_back(node->kids[1]);

		// surface area = 2 * width * depth * height
		vector3d size = node->aabb.max - node->aabb.min;
		double area = 2.0 * size.x * size.y * size.z;

		// Cost function according to https://users.aalto.fi/~laines9/publications/aila2013hpg_paper.pdf Eq. 1
		if (node->IsLeaf())
			outSAH += node->numTris * area;
		else
			outSAH += 1.2 * area;
	}

	vector3d rootSize = m_root->aabb.max - m_root->aabb.min;
	double rootArea = 2.0 * rootSize.x * rootSize.y * rootSize.z;

	// Perform 1 / Aroot * ( SAH sums )
	outSAH /= rootArea;

	// Remove the (normalized) SAH cost of the root node from the result
	// The SAH metric doesn't include the cost of the root node
	outSAH -= 1.0;

	return outSAH;
}

// ===================================================================

SingleBVHTree::SingleBVHTree() :
	m_treeHeight(0),
	m_boundsCenter(0.0, 0.0, 0.0),
	m_inv_scale_factor(1.0)
{
	Clear();
}

void SingleBVHTree::Clear()
{
	m_nodes.clear();

	// Build a default / invalid node for this BVHTree
	AABBd aabb = AABBd::Invalid();
	SortKey key = { vector3f(0.f, 0.f, 0.f), 0 };
	BuildNode(&m_nodes.emplace_back(), &key, 1, &aabb, 0);
}

void SingleBVHTree::Build(const AABBd &bounds, AABBd *objAabbs, uint32_t numObjs)
{
	PROFILE_SCOPED()

	// bound on a fully-populated perfect binary tree: 2 * 2^(log2(n)) - 1
	// reserve less than that, assuming this won't be a perfect tree
	// experimental data shows generally 2 nodes per leaf object
	m_nodes.clear();
	m_nodes.reserve(2 * numObjs + 1);

	// compute a remapping term to express object positions with highest precision using single floating point
	// use the average of the bounding volue to remap into [-1 .. 1] space
	m_boundsCenter = (bounds.max + bounds.min) * 0.5;
	vector3d inv_scale = 1.0 / (bounds.max - m_boundsCenter);
	m_inv_scale_factor = (inv_scale.x + inv_scale.y + inv_scale.z) / 3.0;

	// Build a vector of sort keys for individual nodes (position within system)
	std::vector<SortKey> sortKeys(numObjs);
	for (size_t idx = 0; idx < numObjs; idx++) {
		vector3d center = (objAabbs[idx].max + objAabbs[idx].min) * 0.5 - m_boundsCenter;
		sortKeys[idx].center = vector3f(center * m_inv_scale_factor);
		sortKeys[idx].index = idx;
	}

	BuildNode(&m_nodes.emplace_back(), sortKeys.data(), numObjs, objAabbs, 0);
}

void SingleBVHTree::BuildNode(Node *node, SortKey *keys, uint32_t numKeys, AABBd *objAabbs, uint32_t height)
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

uint32_t SingleBVHTree::Partition(SortKey *keys, uint32_t numKeys, const AABBd &aabb, AABBd *objAabbs)
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

void SingleBVHTree::ComputeOverlap(uint32_t nodeId, const AABBd &nodeAabb, std::vector<std::pair<uint32_t, uint32_t>> &out_isect) const
{
	PROFILE_SCOPED()

	int32_t stackLevel = 0;
	uint32_t *stack = stackalloc(uint32_t, m_treeHeight + 1);
	// Push the root node
	stack[stackLevel++] = 0;

	while (stackLevel > 0) {
		const SingleBVHTree::Node *node = &m_nodes[stack[--stackLevel]];

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

void SingleBVHTree::TraceRay(const vector3d &start, const vector3d &inv_dir, double len, std::vector<uint32_t> &out_isect) const
{
	PROFILE_SCOPED()

	int32_t stackLevel = 0;
	uint32_t *stack = stackalloc(uint32_t, m_treeHeight + 1);
	stack[stackLevel++] = 0;

	while (stackLevel > 0) {
		uint32_t nodeIdx = stack[--stackLevel];
		const SingleBVHTree::Node *node = &m_nodes[nodeIdx];

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

double SingleBVHTree::CalculateSAH() const
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

		// surface area = 2 * width * depth * height
		vector3d size = node->aabb.max - node->aabb.min;
		double area = 2.0 * size.x * size.y * size.z;

		// Cost function according to https://users.aalto.fi/~laines9/publications/aila2013hpg_paper.pdf Eq. 1
		outSAH += (node->kids[0] ? 1.2 : 1.0) * area;
	}

	vector3d rootSize = rootNode->aabb.max - rootNode->aabb.min;
	double rootArea = 2.0 * rootSize.x * rootSize.y * rootSize.z;

	// Perform 1 / Aroot * ( SAH sums )
	outSAH /= rootArea;

	// Remove the (normalized) SAH cost of the root node from the result
	// The SAH metric doesn't include the cost of the root node
	outSAH -= 1.0;

	return outSAH;
}
