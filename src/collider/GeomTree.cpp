
/*
 * Bounding interval hierarchy tree building algorithm.
 *
 * These things get used in interactive raytracers. They are nice because:
 *   n log n builders are easy to write
 *   memory requirement is more predictable than kd-trees
 *   you don't need a 'mailbox' as objects only appear once in the hierarchy
 *   single ray traversal performance is comparable to kd-tree and way faster than bvh
 */

#define MAX_LEAF_PRIMS	2
#define MAX_DEPTH	20
#define MAX_SPLITPOS_RETRIES	32
#define MIN_SPACE_CUTOFF	0.33
#define EPSILON 0.00001

#include <float.h>
#include <stdio.h>
#include <assert.h>
#include <alloca.h>
#include "../Aabb.h"
#include "GeomTree.h"

#define MIN(a,b) ((a)<(b) ? (a) : (b))
#define MAX(a,b) ((a)>(b) ? (a) : (b))

class DisplayList;

struct tri_t {
	int triIdx;
	tri_t *next;
	tri_t *GetNext() { return next; }
};

class BIHNode {
	public:
	BIHNode () { m_isleaf = 1; m_axis = 0; m_list = 0; m_left = 0; }
	void Add (tri_t *tri) {
		tri->next = GetList ();
		SetList (tri);
	}
	void SetAxis (int axis) { m_axis = axis; }
	int GetAxis () const { return m_axis; };
	void SetSplitPos1 (float p) { splitPos1 = p; }
	void SetSplitPos2 (float p) { splitPos2 = p; }
	float GetSplitPos1 () { return splitPos1; }
	float GetSplitPos2 () { return splitPos2; }
	void AllocKids (GeomTree *geomTree);
	void SetLeaf (bool isLeaf) { m_isleaf = isLeaf; }
	bool IsLeaf () { return m_isleaf; }
	tri_t *GetList () { return m_list; }
	BIHNode *GetLeft () { return m_left; };
	BIHNode *GetRight () { return GetLeft()+1; }
	void SetLeft (BIHNode *left) { m_left = left; }
	void SetList (tri_t *t) { m_list = t; }

	private:
	short m_axis;
	short m_isleaf;
	union {
		BIHNode *m_left;
		tri_t *m_list;
	};
	float splitPos1, splitPos2;
};

void BIHNode::AllocKids(GeomTree *geomTree)
{
	m_left = geomTree->AllocNode();
	geomTree->AllocNode();
	// right child is implicitly left+1
}

BIHNode *GeomTree::AllocNode()
{
	assert(m_nodesAllocPos+1 < m_nodesAllocSize);
	return &m_nodes[m_nodesAllocPos++];
}

GeomTree::~GeomTree()
{
	delete [] m_nodes;
	delete [] m_triAlloc;
}

GeomTree::GeomTree(int numVerts, int numTris, float *vertices, int *indices, int *triflags): m_numVertices(numVerts)
{
	m_vertices = vertices;
	m_indices = indices;
	m_triFlags = triflags;
	m_aabb.min = vector3d(FLT_MAX,FLT_MAX,FLT_MAX);
	m_aabb.max = vector3d(-FLT_MAX,-FLT_MAX,-FLT_MAX);

	m_triAlloc = new tri_t[numTris];
	for (int i=0; i<numTris; i++) {
		m_aabb.Update(vector3d(vertices[3*i], vertices[3*i+1], vertices[3*i+2]));
		m_triAlloc[i].triIdx = 3*i;
		m_triAlloc[i].next = m_triAlloc+i+1;
	}
	m_triAlloc[numTris-1].next = 0;

	// make big rotation aabb
	{
		vector3d cent = 0.5*(m_aabb.min+m_aabb.max);
		double mdim = (cent - m_aabb.min).Length();
		mdim = MAX(mdim, (m_aabb.max - cent).Length());
		m_maxAabb.min = vector3d(cent.x - mdim, cent.y - mdim, cent.z - mdim);
		m_maxAabb.max = vector3d(cent.x + mdim, cent.y + mdim, cent.z + mdim);
	}

	printf("Building BIHTree of %d triangles\n", numTris);
	printf("Aabb: %f,%f,%f -> %f,%f,%f\n",
		m_aabb.min.x,
		m_aabb.min.y,
		m_aabb.min.z,
		m_aabb.max.x,
		m_aabb.max.y,
		m_aabb.max.z);
	printf("MaxAabb: %f,%f,%f -> %f,%f,%f\n",
		m_maxAabb.min.x,
		m_maxAabb.min.y,
		m_maxAabb.min.z,
		m_maxAabb.max.x,
		m_maxAabb.max.y,
		m_maxAabb.max.z);
	m_nodes = new BIHNode[numTris*4];
	m_nodesAllocSize = numTris*4;
	m_nodesAllocPos = 0;
	m_triAllocPos = 0;

	BIHNode *root = AllocNode();
	root->SetList(m_triAlloc);

	Aabb splitBox = m_aabb;
	BihTreeGhBuild(root, m_aabb, splitBox, 0, numTris);
}

void GeomTree::BihTreeGhBuild(BIHNode* a_node, Aabb &a_box, Aabb &a_splitBox, int a_depth, int a_prims)
{
	tri_t **prim_lump = (tri_t**)alloca(sizeof(tri_t*)*a_prims);
	int num = 0;

	for (tri_t *kdprim = a_node->GetList (); kdprim != NULL; kdprim = kdprim->GetNext ()) {
		prim_lump[num++] = kdprim;
	}

	// simple master split pos picking for the mo
	float splitPos;
	float splitPos1;
	float splitPos2;

	a_node->SetLeaf (false);
	a_node->AllocKids (this);
	BIHNode *left = a_node->GetLeft ();
	BIHNode *right = a_node->GetRight ();

	int s1count, s2count, splitAxis, attempt;
	attempt = 0;

	Aabb realAabb;
	realAabb.min = vector3d(FLT_MAX, FLT_MAX, FLT_MAX);
	realAabb.max = vector3d(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	// make actual objects aabb, to see if we can usefully cut off some empty space
	for (int i=0; i<a_prims; i++) {
		vector3d v0 = vector3d(&m_vertices[3*m_indices[prim_lump[i]->triIdx]]);
		vector3d v1 = vector3d(&m_vertices[3*m_indices[prim_lump[i]->triIdx+1]]);
		vector3d v2 = vector3d(&m_vertices[3*m_indices[prim_lump[i]->triIdx+2]]);
		realAabb.Update(v0);
		realAabb.Update(v1);
		realAabb.Update(v2);
	}
/*	printf("Empty space: (%f,%f,%f), (%f,%f,%f)\n",
		(realAabb.min-a_box.min).x, 
		(realAabb.min-a_box.min).y, 
		(realAabb.min-a_box.min).z, 
		(a_box.max - realAabb.max).x,
		(a_box.max - realAabb.max).y,
		(a_box.max - realAabb.max).z);
*/	{
		vector3d boxSize = a_box.max - a_box.min;
		vector3d lowSpace = realAabb.min - a_box.min;
		vector3d highSpace = a_box.max - realAabb.max;
		// pick best
		float bestCost = 0;
		int axis = -1;
		int isTop = false;
		for (int i=0; i<3; i++) {
			float cost = (lowSpace[i]+EPSILON) / boxSize[i];
			if (cost > bestCost) {
				bestCost = cost;
				axis = i;
				isTop = false;
			}
			cost = (highSpace[i]+EPSILON) / boxSize[i];
			if (cost > bestCost) {
				bestCost = cost;
				axis = i;
				isTop = true;
			}
		}
//		if (axis != -1) printf("Best is axis %d on %s (cost %f%%)\n", axis, (isTop ? "top" : "bottom"), bestCost*100);
		// cut off whitespace
		if ((bestCost > MIN_SPACE_CUTOFF) && (bestCost < 1.0f)) {
			a_node->SetLeaf (false);
			a_node->SetAxis (axis);

			Aabb newAabb = a_box;
			if (isTop) {
				newAabb.max[axis] = realAabb.max[axis]+EPSILON;
				a_node->SetSplitPos1 (realAabb.max[axis]);
				a_node->SetSplitPos2 (newAabb.max[axis]);
				
				Aabb splitBox = newAabb;
				left->SetList(prim_lump[0]);
				BihTreeGhBuild(left, newAabb, splitBox, a_depth+1, a_prims);
				right->SetLeaf(true);
			} else {
				newAabb.min[axis] = realAabb.min[axis] - EPSILON;
				a_node->SetSplitPos1 (a_box.min[axis]);
				a_node->SetSplitPos2 (newAabb.min[axis]);
				
				Aabb splitBox = newAabb;
				right->SetList(prim_lump[0]);
				BihTreeGhBuild(right, newAabb, splitBox, a_depth+1, a_prims);
				left->SetLeaf(true);
			}
			return;
		}
	}

	for (;;) {
		splitAxis = 0;
		vector3d splitBoxSize = a_splitBox.max - a_splitBox.min;
		if (splitBoxSize.y > splitBoxSize.x) splitAxis = 1;
		if ((splitBoxSize.z > splitBoxSize.y) && (splitBoxSize.z > splitBoxSize.x)) splitAxis = 2;

		// split pos in middle of a_splitBox
		splitPos = 0.5f * (a_splitBox.min[splitAxis] + a_splitBox.max[splitAxis]);

		//printf ("\n%d: %f ", attempt, splitPos);
		splitPos1 = a_box.min[splitAxis];
		splitPos2 = a_box.max[splitAxis];
		
		s1count = 0, s2count = 0;
		float crudSum = 0.0f;

		left->SetList (0);
		right->SetList (0);

		for (int i=0; i<a_prims; i++) {
			const int v0 = m_indices[prim_lump[i]->triIdx];
			const int v1 = m_indices[prim_lump[i]->triIdx+1];
			const int v2 = m_indices[prim_lump[i]->triIdx+2];
		
			float p0, p1, p2;
			float mid;

			p0 = m_vertices[3*v0 + splitAxis];
			p1 = m_vertices[3*v1 + splitAxis];
			p2 = m_vertices[3*v2 + splitAxis];

			mid = (p0 + p1 + p2)*0.3333333333333333333f;

			float p_min, p_max;
	
			p_min = MIN (p0, MIN (p1, p2));
			p_max = MAX (p0, MAX (p1, p2));
		
			tri_t *poo = prim_lump[i];

			crudSum += mid;

			if (mid < splitPos) {
				left->Add (poo);
				s1count++;
				if (p_max > splitPos1) splitPos1 = p_max;
			} else {
				right->Add (poo);
				s2count++;
				if (p_min < splitPos2) splitPos2 = p_min;
			}
		}

		if (s1count == a_prims) {
			// if one side takes up the whole damn parent aabb then
			// just give up trying to split
			if (splitPos1 >= a_box.max[splitAxis]) {
				if (attempt < MAX_SPLITPOS_RETRIES) {
					// try splitting at average point
				//	printf ("YES!!!!!!!!!!!!!!!!!!!!!! %d, %f\n", attempt, splitPos);
					a_splitBox.max = splitPos;
					attempt++;
					continue;
				}

				printf ("Warning: Fat node with %d primitives\n", a_prims);

				a_node->SetLeaf (true);
				a_node->SetList (left->GetList ());
				return;
			}
		} else if (s2count == a_prims) {
			if (splitPos2 <= a_box.min[splitAxis]) {
				if (attempt < MAX_SPLITPOS_RETRIES) {
					// try splitting at average point
				//	printf ("YES!!!!!!!!!!!!!!!!!!!!!! %d, %f\n", attempt, splitPos);
					a_splitBox.min[splitAxis] = splitPos;
					attempt++;
					continue;
				}
				printf ("Warning: Fat node with %d primitives\n", a_prims);
				
				a_node->SetLeaf (true);
				a_node->SetList (right->GetList ());
				return;
			}
		}
		break;
	}
	// Traversal algo can't handle completely flat cells..
	splitPos1 += EPSILON;
	splitPos2 -= EPSILON;
	
	a_node->SetLeaf (false);
	a_node->SetAxis (splitAxis);

	Aabb b1, b2;

	b1 = a_box;
	b1.max[splitAxis] = splitPos1;
	b2 = a_box;
	b2.min[splitAxis] = splitPos2;

	a_node->SetSplitPos1 (splitPos1);
	a_node->SetSplitPos2 (splitPos2);

	if (a_depth > MAX_DEPTH) return;

	if (s1count > MAX_LEAF_PRIMS) {
		Aabb splitBox = a_splitBox;
		splitBox.max[splitAxis] = splitPos;
		BihTreeGhBuild (left, b1, splitBox, a_depth+1, s1count);
	}
	else left->SetLeaf (true);
	
	if (s2count > MAX_LEAF_PRIMS) {
		Aabb splitBox = a_splitBox;
		splitBox.min[splitAxis] = splitPos;
		BihTreeGhBuild (right, b2, splitBox, a_depth+1, s2count);
	}
	else right->SetLeaf (true);
}

void GeomTree::TraceRay(const vector3f &start, const vector3f &dir, isect_t *isect)
{
	TraverseRay(start, dir, isect);
}

inline void GeomTree::TraverseRay(const vector3f &a_origin, const vector3f &a_dir, isect_t *isect)
{
	float entry_t = 0, exit_t = isect->dist;
	vector3f rcpD = vector3f(1.0f/a_dir.x, 1.0f/a_dir.y, 1.0f/a_dir.z);
	int Dneg[3];

	Dneg[0] = (a_dir.x < 0 ? 1 : 0);
	Dneg[1] = (a_dir.y < 0 ? 1 : 0);
	Dneg[2] = (a_dir.z < 0 ? 1 : 0);


	for ( int i = 0; i < 3; i++ ) {
		if (Dneg[i]) {
			if (a_origin[i] < m_aabb.min[i]) return;
		}
		else if (a_origin[i] > m_aabb.max[i]) return;
	}

	// clip ray segment to box
	for (int i = 0; i < 3; i++ )
	{
		float clip_min = (m_aabb.min[i] - a_origin[i]) * rcpD[i];
		float clip_max = (m_aabb.max[i] - a_origin[i]) * rcpD[i];
		if (a_dir[i] > 0.0f) {
			entry_t = MAX (entry_t, clip_min);
			exit_t = MIN (exit_t, clip_max);
		} else {
			entry_t = MAX (entry_t, clip_max);
			exit_t = MIN (exit_t, clip_min);
		}
		if (entry_t > exit_t) return;
	}
#if 0
	/* from final kd-tree version */
	// init stack
	int entrypoint = 0, exitpoint = 1;
	// init traversal
	KDNode* farchild, *currnode;
	currnode = obj.m_dlist->sceneTree;
	m_Stack[entrypoint].t = entry_t;
	m_Stack[entrypoint].pb = O + D * entry_t;
	m_Stack[exitpoint].t = exit_t;
	m_Stack[exitpoint].pb = O + D * exit_t;
	m_Stack[exitpoint].node = 0;
#endif

	// init stack
	int stackpos = -1;
	// init traversal
	BIHNode *currnode = &m_nodes[0];
	
	struct bihstack
	{
		BIHNode* node;
		float entry_t, exit_t;
	} bihstack[32];
	

	// traverse bih-tree
	while (currnode)
	{
		while (!currnode->IsLeaf())
		{
			const int axis = currnode->GetAxis();
			
			float d[2];
			d[0] = (currnode->GetSplitPos1() - a_origin[axis]) * rcpD[axis];
			d[1] = (currnode->GetSplitPos2() - a_origin[axis]) * rcpD[axis];

			const int dir = Dneg[axis];
			const int dir1 = 1-Dneg[axis];
			const float d1 = d[dir];
			const float d2 = d[dir1];

			if (d1 >= entry_t) {
				// front side
				if (d2 >= exit_t) {
					// and not backside
					currnode = currnode->GetLeft ()+dir;
					exit_t = MIN (d1, exit_t);
					continue;
				}
				// both...
				stackpos++;
				bihstack[stackpos].node = currnode->GetLeft ()+dir1;
				bihstack[stackpos].entry_t = MAX (d2, entry_t);
				bihstack[stackpos].exit_t = exit_t;
				
				currnode = currnode->GetLeft ()+dir;
				exit_t = MIN (d1, exit_t);
			}	
			else if (d2 < exit_t) {
				// back side only
				currnode = currnode->GetLeft ()+dir1;
				entry_t = MAX (d2, entry_t);
			} else {
				goto pop_bstack;
			}
		}
		// early termination
		if (isect->dist < entry_t) goto pop_bstack;

		// yay we are a leaf node 
		for (tri_t *p = currnode->GetList (); p != NULL; p = p->next) {
			RayTriIntersect (a_origin, a_dir, p->triIdx, isect);
		}
pop_bstack:
		if (stackpos < 0) break;
		
		currnode = bihstack[stackpos].node;
		entry_t = bihstack[stackpos].entry_t;
		exit_t = bihstack[stackpos].exit_t;
		stackpos--;
	}
}

void GeomTree::RayTriIntersect(const vector3f &origin, const vector3f &dir, int triIdx, isect_t *isect)
{
	const vector3f a(&m_vertices[3*m_indices[triIdx]]);
	const vector3f b(&m_vertices[3*m_indices[triIdx+1]]);
	const vector3f c(&m_vertices[3*m_indices[triIdx+2]]);

	vector3f v0_cross, v1_cross, v2_cross;

	v0_cross = vector3f::Cross(c-origin, b-origin);
	v1_cross = vector3f::Cross(b-origin, a-origin);
	v2_cross = vector3f::Cross(a-origin, c-origin);

	const float v0d = vector3f::Dot(v0_cross,dir);
	const float v1d = vector3f::Dot(v1_cross,dir);
	const float v2d = vector3f::Dot(v2_cross,dir);

	if (((v0d > 0) && (v1d > 0) && (v2d > 0)) ||
	    ((v0d < 0) && (v1d < 0) && (v2d < 0))) {
		const vector3f n = vector3f::Cross(c-a, b-a);
		const float nominator = vector3f::Dot(n, (a-origin));
		const float dist = nominator / vector3f::Dot(dir,n);
		if ((dist > EPSILON) && (dist < isect->dist)) {
			isect->dist = dist;
			isect->triIdx = triIdx/3;
		}
	}
}

vector3f GeomTree::GetTriNormal(int triIdx) const
{
	const vector3f a(&m_vertices[3*m_indices[3*triIdx]]);
	const vector3f b(&m_vertices[3*m_indices[3*triIdx+1]]);
	const vector3f c(&m_vertices[3*m_indices[3*triIdx+2]]);
	
	return vector3f::Normalize(vector3f::Cross(b-a, c-a));
}
