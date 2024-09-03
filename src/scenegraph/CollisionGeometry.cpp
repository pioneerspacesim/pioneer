// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "CollisionGeometry.h"
#include "BaseLoader.h"
#include "NodeCopyCache.h"
#include "NodeVisitor.h"
#include "Serializer.h"
#include "profiler/Profiler.h"

namespace SceneGraph {

	CollisionGeometry::CollisionGeometry(Graphics::Renderer *r, const std::vector<vector3f> &vts, const std::vector<Uint32> &idx,
		unsigned int geomflag) :
		Node(r),
		m_triFlag(geomflag),
		m_dynamic(false),
		m_geomTree(0),
		m_geom(0)
	{
		PROFILE_SCOPED()
		CopyData(vts, idx);
	}

	CollisionGeometry::CollisionGeometry(const CollisionGeometry &cg, NodeCopyCache *cache) :
		Node(cg, cache),
		m_vertices(cg.m_vertices),
		m_indices(cg.m_indices),
		m_triFlag(cg.m_triFlag),
		m_dynamic(cg.m_dynamic),
		m_geomTree(cg.m_geomTree),
		m_geom(cg.m_geom){
			PROFILE_SCOPED()
		}

		CollisionGeometry::~CollisionGeometry()
	{
	}

	Node *CollisionGeometry::Clone(NodeCopyCache *cache)
	{
		PROFILE_SCOPED()
		//static collgeoms are shared,
		//dynamic geoms are copied (they should be tiny)
		if (IsDynamic())
			return cache->Copy<CollisionGeometry>(this);
		else
			return this;
	}

	void CollisionGeometry::Accept(NodeVisitor &nv)
	{
		PROFILE_SCOPED()
		nv.ApplyCollisionGeometry(*this);
	}

	void CollisionGeometry::Save(NodeDatabase &db)
	{
		PROFILE_SCOPED()
		Node::Save(db);
		db.wr->Int32(m_vertices.size());
		for (const auto &pos : m_vertices)
			db.wr->Vector3f(pos);
		db.wr->Int32(m_indices.size());
		for (const auto idx : m_indices)
			db.wr->Int32(idx);
		db.wr->Int32(m_triFlag);
		db.wr->Bool(m_dynamic);
	}

	CollisionGeometry *CollisionGeometry::Load(NodeDatabase &db)
	{
		PROFILE_SCOPED()
		std::vector<vector3f> pos;
		std::vector<Uint32> idx;
		Serializer::Reader &rd = *db.rd;

		Uint32 n = rd.Int32();
		pos.reserve(n);
		for (Uint32 i = 0; i < n; i++)
			pos.push_back(rd.Vector3f());

		n = rd.Int32();
		idx.reserve(n);
		for (Uint32 i = 0; i < n; i++)
			idx.push_back(rd.Int32());

		const Uint32 flag = rd.Int32();
		const bool dynamic = rd.Bool();

		CollisionGeometry *cg = new CollisionGeometry(db.loader->GetRenderer(), pos, idx, flag);
		cg->SetDynamic(dynamic);

		return cg;
	}

	void CollisionGeometry::CopyData(const std::vector<vector3f> &vts, const std::vector<Uint32> &idx)
	{
		PROFILE_SCOPED()
		//copy vertices and indices from surface. Add flag for every three indices.
		using std::vector;

		for (vector<vector3f>::const_iterator it = vts.begin(); it != vts.end(); ++it)
			m_vertices.push_back(*it);

		for (vector<Uint32>::const_iterator it = idx.begin(); it != idx.end(); ++it)
			m_indices.push_back(*it);
	}
} // namespace SceneGraph
