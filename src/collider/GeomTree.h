// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GEOMTREE_H
#define _GEOMTREE_H

#include "CollisionContact.h"
#include "Serializer.h"
#include "libs.h"

struct isect_t {
	// triIdx = -1 if no intersection
	int triIdx;
	float dist;
};

class BVHTree;
struct BVHNode;

class GeomTree {
public:
	GeomTree(const int numVerts, const int numTris, const std::vector<vector3f> &vertices, const Uint32 *indices, const Uint32 *triflags);
	GeomTree(Serializer::Reader &rd);
	~GeomTree();

	const Aabb &GetAabb() const { return m_aabb; }
	// dir should be unit length,
	// isect.dist should be ray length
	// isect.triIdx should be -1 unless repeat calls with same isect_t
	void TraceRay(const vector3f &start, const vector3f &dir, isect_t *isect) const;
	void TraceRay(const BVHNode *startNode, const vector3f &a_origin, const vector3f &a_dir, isect_t *isect) const;
	vector3f GetTriNormal(int triIdx) const;
	Uint32 GetTriFlag(int triIdx) const { return m_triFlags[triIdx]; }
	double GetRadius() const { return m_radius; }
	struct Edge {
		int v1i, v2i;
		float len;
		vector3f dir;
		// edge triFlag can be weird since edges may get merged and
		// intended flag lost
		int triFlag;

		void Save(Serializer::Writer &wr) const
		{
			PROFILE_SCOPED()
			wr.Int32(v1i);
			wr.Int32(v2i);
			wr.Float(len);
			wr.Vector3f(dir);
			wr.Int32(triFlag);
		}
		void Load(Serializer::Reader &rd)
		{
			PROFILE_SCOPED()
			v1i = rd.Int32();
			v2i = rd.Int32();
			len = rd.Float();
			dir = rd.Vector3f();
			triFlag = rd.Int32();
		}
	};
	const Edge *GetEdges() const { return &m_edges[0]; }
	int GetNumEdges() const { return m_numEdges; }

	BVHTree *GetTriTree() const { return m_triTree.get(); }
	BVHTree *GetEdgeTree() const { return m_edgeTree.get(); }

	const std::vector<vector3f> &GetVertices() const { return m_vertices; }
	const Uint32 *GetIndices() const { return &m_indices[0]; }
	const Uint32 *GetTriFlags() const { return &m_triFlags[0]; }
	int GetNumVertices() const { return m_numVertices; }
	int GetNumTris() const { return m_numTris; }

	void Save(Serializer::Writer &wr) const;

private:
	void RayTriIntersect(int numRays, const vector3f &origin, const vector3f *dirs, int triIdx, isect_t *isects) const;

	int m_numVertices;
	int m_numEdges;
	int m_numTris;

	double m_radius;
	Aabb m_aabb;
	std::vector<Aabb> m_aabbs;

	std::unique_ptr<BVHTree> m_triTree;
	std::unique_ptr<BVHTree> m_edgeTree;

	std::vector<Edge> m_edges;

	std::vector<vector3f> m_vertices;
	std::vector<Uint32> m_indices;
	std::vector<Uint32> m_triFlags;
};

#endif /* _GEOMTREE_H */
