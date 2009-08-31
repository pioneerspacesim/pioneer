#ifndef _BVHTREE_H
#define _BVHTREE_H

#define MIN(a,b) ((a)<(b) ? (a) : (b))
#define MAX(a,b) ((a)>(b) ? (a) : (b))

#define MAX_SPLITPOS_RETRIES 15

struct BVHNode {
	Aabb aabb;
	
	/* if triIndicesStart == 0 then not leaf,
	 * kids[] valid */
	int numTris;
	int *triIndicesStart;
	
	BVHNode *kids[2];

	BVHNode() {
		kids[0] = 0;
		triIndicesStart = 0;
	}
	bool IsLeaf() {
		return triIndicesStart != 0;
	}

	bool SlabsRayAabbTest(const vector3f &start, const vector3f &invDir, isect_t *isect)
	{
		float
                l1      = (aabb.min.x - start.x) * invDir.x,
                l2      = (aabb.max.x - start.x) * invDir.x,
                lmin    = MIN(l1,l2),
                lmax    = MAX(l1,l2);

		l1      = (aabb.min.y - start.y) * invDir.y;
		l2      = (aabb.max.y - start.y) * invDir.y;
		lmin    = MAX(MIN(l1,l2), lmin);
		lmax    = MIN(MAX(l1,l2), lmax);

		l1      = (aabb.min.z - start.z) * invDir.z;
		l2      = (aabb.max.z - start.z) * invDir.z;
		lmin    = MAX(MIN(l1,l2), lmin);
		lmax    = MIN(MAX(l1,l2), lmax);

		return ((lmax >= 0.f) & (lmax >= lmin) & (lmin < isect->dist));
	}

};

class BVHTree {
public:
	typedef int objPtr_t;
	BVHTree(int numObjs, const objPtr_t *objPtrs, const Aabb *objAabbs);
	~BVHTree() {
		delete [] m_objPtrAlloc;
		delete [] m_bvhNodes;
	}
	BVHNode *GetRoot() { return m_root; }
private:
	void BuildNode(BVHNode *node,
			const objPtr_t *objPtrs,
			const Aabb *objAabbs,
			std::vector<objPtr_t> &activeObjIdxs);
	void MakeLeaf(BVHNode *node, const objPtr_t *objPtrs, std::vector<objPtr_t> &objs);
	BVHNode *AllocNode() {
		assert(m_nodeAllocPos < m_nodeAllocMax);
		return &m_bvhNodes[m_nodeAllocPos++];
	}
	BVHNode *m_root;
	objPtr_t *m_objPtrAlloc;
	int m_objPtrAllocPos;
	int m_objPtrAllocMax;

	BVHNode *m_bvhNodes;
	int m_nodeAllocPos;
	int m_nodeAllocMax;
};

BVHTree::BVHTree(int numObjs, const objPtr_t *objPtrs, const Aabb *objAabbs)
{
	std::vector<int> activeObjIdxs(numObjs);
	for (int i=0; i<numObjs; i++) activeObjIdxs[i] = i;

	m_objPtrAlloc = new objPtr_t[numObjs];
	m_objPtrAllocPos = 0;
	m_objPtrAllocMax = numObjs;

	m_bvhNodes = new BVHNode[numObjs*2];
	m_nodeAllocPos = 0;
	m_nodeAllocMax = numObjs*2;

	m_root = AllocNode();
	BuildNode(m_root, objPtrs, objAabbs, activeObjIdxs);
}

void BVHTree::MakeLeaf(BVHNode *node, const objPtr_t *objPtrs, std::vector<objPtr_t> &objs)
{
	node->numTris = objs.size();
	node->triIndicesStart = &m_objPtrAlloc[m_objPtrAllocPos];
	if (objs.size()>3) printf("fat node %d\n", objs.size());

	// copy tri indices to the stinking flat array
	for (int i=objs.size()-1; i>=0; i--) {
		m_objPtrAlloc[m_objPtrAllocPos++] = objPtrs[objs[i]];
	}
}

void BVHTree::BuildNode(BVHNode *node,
			const objPtr_t *objPtrs,
			const Aabb *objAabbs,
			std::vector<objPtr_t> &activeObjIdx)
{
	const int numTris = activeObjIdx.size();
	Aabb aabb;
	aabb.min = vector3d(FLT_MAX, FLT_MAX, FLT_MAX);
	aabb.max = vector3d(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (int i=0; i<numTris; i++) {
		int idx = activeObjIdx[i];
		aabb.Update(objAabbs[idx].min);
		aabb.Update(objAabbs[idx].max);
	}
	Aabb splitBox = aabb;
	float splitPos;
	int splitAxis;
	int s1count, s2count;
	int attempt = 0;

	for (;;) {
		splitAxis = 0;
	
		vector3d boxSize = splitBox.max - splitBox.min;
	
		if (boxSize[1] > boxSize[0]) splitAxis = 1;
		if ((boxSize[2] > boxSize[1]) && (boxSize[2] > boxSize[0])) splitAxis = 2;

		// split pos in middle of splitBox
		splitPos = 0.5f * (splitBox.min[splitAxis] + splitBox.max[splitAxis]);
		s1count = 0, s2count = 0;

		for (int i=0; i<numTris; i++) {
			int idx = activeObjIdx[i];
			float mid = 0.5 * (objAabbs[idx].min[splitAxis] + objAabbs[idx].max[splitAxis]);
			if (mid < splitPos) s1count++;
			else s2count++;
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

	for (int i=0; i<numTris; i++) {
		int idx = activeObjIdx[i];
		float mid = 0.5 * (objAabbs[idx].min[splitAxis] + objAabbs[idx].max[splitAxis]);

		if (mid < splitPos) {
			side[0].push_back(idx);
		} else {
			side[1].push_back(idx);
		}
	}

	node->numTris = numTris;
	node->aabb = aabb;

//	// side 1 has all nodes. just make a fucking child 
	if ((side[0].size() == 0) || (side[1].size() == 0)) {
		MakeLeaf(node, objPtrs, activeObjIdx);
	} else {
		// recurse!
		node->triIndicesStart = 0;
		node->kids[0] = AllocNode();
		node->kids[1] = AllocNode();

		BuildNode(node->kids[0], objPtrs, objAabbs, side[0]);
		BuildNode(node->kids[1], objPtrs, objAabbs, side[1]);
	}
}


#endif /* _BVHTREE_H */
