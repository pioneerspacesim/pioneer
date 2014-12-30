// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GEOMTREE_H
#define _GEOMTREE_H

#include "libs.h"
#include "CollisionContact.h"
#include "Serializer.h"

struct isect_t {
	// triIdx = -1 if no intersection
	int triIdx;
	float dist;
};

class BVHTree;
struct BVHNode;

class GeomTree {
public:
	GeomTree(int numVerts, int numTris, float *vertices, Uint16 *indices, unsigned int *triflags);
	GeomTree(Serializer::Reader &rd);
	~GeomTree();

	const Aabb &GetAabb() const { return m_aabb; }
	// dir should be unit length,
	// isect.dist should be ray length
	// isect.triIdx should be -1 unless repeat calls with same isect_t
	void CollideEdgesWithTrisOf(const GeomTree *other, const matrix4x4d &transTo, void (*callback)(CollisionContact*)) const;
	void TraceRay(const vector3f &start, const vector3f &dir, isect_t *isect) const;
	void TraceRay(const BVHNode *startNode, const vector3f &a_origin, const vector3f &a_dir, isect_t *isect) const;
	void TraceCoherentRays(int numRays, const vector3f &a_origin, const vector3f *a_dirs, isect_t *isects) const;
	void TraceCoherentRays(const BVHNode *startNode, int numRays, const vector3f &a_origin, const vector3f *a_dirs, isect_t *isects) const;
	vector3f GetTriNormal(int triIdx) const;
	unsigned int GetTriFlag(int triIdx) const { return m_triFlags[triIdx]; }
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
			wr.Int32(v1i);
			wr.Int32(v2i);
			wr.Float(len);
			wr.Vector3f(dir);
			wr.Int32(triFlag);
		}
		void Load(Serializer::Reader &rd)
		{
			v1i = rd.Int32();
			v2i = rd.Int32();
			len = rd.Float();
			dir = rd.Vector3f();
			triFlag = rd.Int32();
		}
	};
	const Edge *GetEdges() const { return m_edges.get(); }
	int GetNumEdges() const { return m_numEdges; }

	BVHTree* GetTriTree() const { return m_triTree.get(); }
	BVHTree* GetEdgeTree() const { return m_edgeTree.get(); }

	const float *GetVertices() const { return m_vertices.get(); }
	const Uint16 *GetIndices() const { return m_indices.get(); }
	const unsigned int *GetTriFlags() const { return m_triFlags.get(); }
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
	std::unique_ptr<Aabb[]> m_aabbs;

	std::unique_ptr<BVHTree> m_triTree;
	std::unique_ptr<BVHTree> m_edgeTree;

	std::unique_ptr<Edge[]> m_edges;

	std::unique_ptr<float[]> m_vertices;
	std::unique_ptr<Uint16[]> m_indices;
	std::unique_ptr<unsigned int[]> m_triFlags;

	static int stats_rayTriIntersections;
};

#endif /* _GEOMTREE_H */
