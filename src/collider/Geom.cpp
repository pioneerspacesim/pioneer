// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Geom.h"

#include "BVHTree.h"
#include "CollisionContact.h"
#include "CollisionSpace.h"
#include "GeomTree.h"

#include "core/Log.h"
#include "profiler/Profiler.h"

#pragma GCC optimize("O3")

static const unsigned int MAX_CONTACTS = 8;

Geom::Geom(const GeomTree *geomtree, const matrix4x4d &m, const vector3d &pos, void *data) :
	m_pos(pos),
	m_geomtree(geomtree),
	m_orient(m),
	m_data(data),
	m_group(0),
	m_mailboxIndex(0),
	m_active(true)
{
	m_orient.SetTranslate(pos);
	m_invOrient = m_orient.Inverse();
}

void Geom::MoveTo(const matrix4x4d &m)
{
	m_orient = m;
	m_pos = m_orient.GetTranslate();
	m_invOrient = m.Inverse();
}

void Geom::MoveTo(const matrix4x4d &m, const vector3d &pos)
{
	m_orient = m;
	m_pos = pos;
	m_orient.SetTranslate(pos);
	m_invOrient = m_orient.Inverse();
}

void Geom::CollideSphere(Sphere &sphere, void (*callback)(CollisionContact *)) const
{
	PROFILE_SCOPED()
	/* if the geom is actually within the sphere, create a contact so
	 * that we can't fall into spheres forever and ever */
	vector3d v = GetPosition() - sphere.pos;
	CollisionContact contact;
	const double len = v.Length();
	if (len < sphere.radius) {
		contact.pos = GetPosition();
		contact.normal = (1.0 / len) * v;
		contact.depth = sphere.radius - len;
		contact.triIdx = 0;
		contact.userData1 = this->m_data;
		contact.userData2 = sphere.userData;
		contact.geomFlag = 0;
		callback(&contact);
		return;
	}
}

/*
 * This geom has moved, causing a possible collision with geom b.
 * Collide meshes to see.
 */
std::vector<CollisionContact> Geom::Collide(Geom *b) const
{
	PROFILE_SCOPED()
	size_t max_contacts = MAX_CONTACTS;
	matrix4x4d transTo;

	std::vector<CollisionContact> contacts;
	contacts.reserve(max_contacts);

	/* Collide this geom's edges against tri-mesh of geom b */
	transTo = b->m_invOrient * m_orient;
	this->CollideEdgesWithTrisOf(contacts, max_contacts, b, transTo);

	/* Collide b's edges against this geom's tri-mesh */
	if (contacts.size() < max_contacts) {
		transTo = m_invOrient * b->m_orient;
		b->CollideEdgesWithTrisOf(contacts, max_contacts, this, transTo);
	}

	return contacts;
}

static AABBd rotateAaabb(const AABBd &a, const matrix4x4d transA)
{
	AABBd arot = AABBd::Invalid();
	arot.Update(transA * vector3d(a.min.x, a.min.y, a.min.z));
	arot.Update(transA * vector3d(a.min.x, a.min.y, a.max.z));
	arot.Update(transA * vector3d(a.min.x, a.max.y, a.min.z));
	arot.Update(transA * vector3d(a.min.x, a.max.y, a.max.z));
	arot.Update(transA * vector3d(a.max.x, a.min.y, a.min.z));
	arot.Update(transA * vector3d(a.max.x, a.min.y, a.max.z));
	arot.Update(transA * vector3d(a.max.x, a.max.y, a.min.z));
	arot.Update(transA * vector3d(a.max.x, a.max.y, a.max.z));
	return arot;
}

/*
 * Intersect this Geom's edge BVH tree with geom b's triangle BVH tree.
 * Generate collision contacts.
 */
void Geom::CollideEdgesWithTrisOf(std::vector<CollisionContact> &contacts, size_t maxContacts, const Geom *b, const matrix4x4d &transTo) const
{
	PROFILE_SCOPED()
	struct stackobj {
		uint32_t edgeNode;
		uint32_t triNode;
	};

	SingleBVHTreeBase *edgeBvh = GetGeomTree()->GetEdgeTree();
	SingleBVHTreeBase *triBvh = b->GetGeomTree()->GetTriTree();

	// Allocate space for BVH trace intersections (to avoid heap-allocating inside CollideEdgesTris)
	std::vector<uint32_t> isect_buf;
	isect_buf.reserve(16);

	int stackpos = 0;

	// Reserve a conservative worst-case amount of memory to store node pairs
	stackobj *stack = new stackobj[std::max(edgeBvh->GetHeight(), triBvh->GetHeight()) * 2 + 1];
	stack[0].edgeNode = 0;
	stack[0].triNode = 0;

	while ((stackpos >= 0) && (contacts.size() < maxContacts)) {
		stackobj curr = stack[stackpos--];
		const SingleBVHTreeBase::Node *edgeNode = edgeBvh->GetNode(curr.edgeNode);
		const SingleBVHTreeBase::Node *triNode = triBvh->GetNode(curr.triNode);

		// NOTE:
		// This behavior is preserved almost verbatim from existing collision
		// code. It's very possible there are subtle bugs and inaccuracies in
		// the BVH traversal logic that result in missed collision contacts.
		// However, this is the most performant version of this code by a large
		// margin, with seemingly no impact on collision robustness.

		if (!edgeNode->kids[0]) {
			// Reached edge leaf node, perform edge-triangle ray intersection
			CollideEdgeTris(contacts, transTo, b, edgeNode->leafIndex, curr.triNode, isect_buf);

			if (contacts.size() >= maxContacts) break;
		} else if (!triNode->kids[0]) {
			// Reached triangle leaf node, recursively descend through edge nodes
			stack[++stackpos] = stackobj { edgeNode->kids[1], curr.triNode };
			stack[++stackpos] = stackobj { edgeNode->kids[0], curr.triNode };
		} else {
			// does the edgeNode (with its aabb described in 6 planes transformed and rotated to
			// b's coordinates) intersect with one or other of b's child nodes?
			AABBd rotAabb = rotateAaabb(edgeNode->aabb, transTo);
			const bool left = rotAabb.Intersects(triBvh->GetNode(triNode->kids[0])->aabb);
			const bool right = rotAabb.Intersects(triBvh->GetNode(triNode->kids[1])->aabb);

			if (left & right) {
				// Recurse into edge nodes until we find one that's smaller than the tri node
				// (or hit a single edge leaf)
				stack[++stackpos] = stackobj { edgeNode->kids[1], curr.triNode };
				stack[++stackpos] = stackobj { edgeNode->kids[0], curr.triNode };
			} else if (left) {
				// Triangle BVH node is larger than the edge, split it and try again
				stack[++stackpos] = stackobj { curr.edgeNode, triNode->kids[0] };
			} else if (right) {
				// Triangle BVH node is larger than the edge, split it and try again
				stack[++stackpos] = stackobj { curr.edgeNode, triNode->kids[1] };
			}
		}
	}

	delete[] stack;
}

/*
 * Collide one edgeNode (all edges below it) of this Geom with the triangle
 * BVH of another geom (b), starting from btriNode.
 */
void Geom::CollideEdgeTris(std::vector<CollisionContact> &contacts, const matrix4x4d &transToB, const Geom *b, uint32_t edgeIdx, uint32_t triNode, std::vector<uint32_t> &isect_buf) const
{
	// PROFILE_SCOPED() // verbose profiling only, this gets called a LOT
	const GeomTree::Edge *edges = GetGeomTree()->GetEdges();
	const std::vector<vector3f> &rVertices = GetGeomTree()->GetVertices();

	const GeomTree::Edge &edge = edges[edgeIdx];
	const vector3d v1 = transToB * vector3d(rVertices[edge.v1i]);
	const vector3f _from(float(v1.x), float(v1.y), float(v1.z));

	vector3d dir = transToB.ApplyRotationOnly(vector3d(edge.dir));
	isect_t isect;

	isect.dist = edge.len;
	isect.triIdx = -1;

	isect_buf.clear();
	// Taking the reciprocal of the direction computes the inverse of the vector.
	// Division by zero is intended and correct in this situation.
	b->GetGeomTree()->GetTriTree()->TraceRay(v1, 1.0 / dir, edge.len, isect_buf, triNode);

	// TODO
	// This is subtly dependent on overwriting intersections with the last triangle to be processed
	// Ideally all contacts here would be reported...
	vector3f traceDir = vector3f(dir);
	for (uint32_t tri : isect_buf) {
		b->GetGeomTree()->RayTriIntersect(1, _from, &traceDir, tri * 3, &isect);
	}

	if (isect.triIdx == -1)
		return;

	// in world coords
	CollisionContact contact = {};
	contact.pos = b->GetTransform() * (v1 + dir * double(isect.dist));
	contact.normal = vector3d(b->m_geomtree->GetTriNormal(isect.triIdx));
	contact.normal = b->GetTransform().ApplyRotationOnly(contact.normal);
	contact.distance = isect.dist;

	contact.depth = edge.len - isect.dist;
	contact.triIdx = isect.triIdx;
	contact.userData1 = m_data;
	contact.userData2 = b->m_data;
	// contact geomFlag is bitwise OR of triangle's and edge's flags
	contact.geomFlag = b->m_geomtree->GetTriFlag(isect.triIdx) | edge.triFlag;

	contacts.push_back(contact);
}
