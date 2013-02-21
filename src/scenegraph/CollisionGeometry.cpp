// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "CollisionGeometry.h"
#include "NodeVisitor.h"
#include "graphics/Surface.h"

namespace SceneGraph {

CollisionGeometry::CollisionGeometry(Graphics::Renderer *r, Graphics::Surface *s, unsigned int geomflag)
: Node(r)
, m_triFlag(geomflag)
{
	CopyData(s->GetVertices()->position, s->GetIndices());
}

CollisionGeometry::CollisionGeometry(Graphics::Renderer *r, const std::vector<vector3f> &vts, const std::vector<unsigned short> &idx,
	unsigned int geomflag)
: Node(r)
, m_triFlag(geomflag)
{
	CopyData(vts, idx);
}

CollisionGeometry::CollisionGeometry(const CollisionGeometry &cg, NodeCopyCache *cache)
: Node(cg, cache)
, m_vertices(cg.m_vertices)
, m_indices(cg.m_indices)
, m_triFlag(cg.m_triFlag)
{
}

CollisionGeometry::~CollisionGeometry()
{
}

Node* CollisionGeometry::Clone(NodeCopyCache *cache)
{
	return this; //collgeoms are shared
}

void CollisionGeometry::Accept(NodeVisitor &nv)
{
	nv.ApplyCollisionGeometry(*this);
}

void CollisionGeometry::CopyData(const std::vector<vector3f> &vts, const std::vector<unsigned short> &idx)
{
	//copy vertices and indices from surface. Add flag for every three indices.
	using std::vector;

	for (vector<vector3f>::const_iterator it = vts.begin(); it != vts.end(); ++it)
		m_vertices.push_back(*it);

	for (vector<unsigned short>::const_iterator it = idx.begin(); it != idx.end(); ++it)
		m_indices.push_back(*it);
}

}
