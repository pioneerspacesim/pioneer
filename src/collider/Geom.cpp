// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Geom.h"

#include "BVHTree.h"
#include "CollisionContact.h"
#include "CollisionSpace.h"
#include "GeomTree.h"

#include "profiler/Profiler.h"

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

/*matrix4x4d Geom::GetRotation() const
{
	PROFILE_SCOPED()
	matrix4x4d m = GetTransform();
	m[12] = 0; m[13] = 0; m[14] = 0;
	return m;
}*/

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
void Geom::Collide(Geom *b, void (*callback)(CollisionContact *)) const
{
	PROFILE_SCOPED()
	int max_contacts = MAX_CONTACTS;
	matrix4x4d transTo;
	//unsigned int t = SDL_GetTicks();
	/* Collide this geom's edges against tri-mesh of geom b */
	transTo = b->m_invOrient * m_orient;
	this->CollideEdgesWithTrisOf(max_contacts, b, transTo, callback);

	/* Collide b's edges against this geom's tri-mesh */
	if (max_contacts > 0) {
		transTo = m_invOrient * b->m_orient;
		b->CollideEdgesWithTrisOf(max_contacts, this, transTo, callback);
	}

	//	t = SDL_GetTicks() - t;
	//	int numEdges = GetGeomTree()->GetNumEdges() + b->GetGeomTree()->GetNumEdges();
	//	Output("%d 'rays' in %dms (%f rps)\n", numEdges, t, 1000.0*numEdges / (double)t);
}

static bool rotatedAabbIsectsNormalOne(Aabb &a, const matrix4x4d &transA, Aabb &b)
{
	Aabb arot;
	vector3d p[8];
	p[0] = transA * vector3d(a.min.x, a.min.y, a.min.z);
	p[1] = transA * vector3d(a.min.x, a.min.y, a.max.z);
	p[2] = transA * vector3d(a.min.x, a.max.y, a.min.z);
	p[3] = transA * vector3d(a.min.x, a.max.y, a.max.z);
	p[4] = transA * vector3d(a.max.x, a.min.y, a.min.z);
	p[5] = transA * vector3d(a.max.x, a.min.y, a.max.z);
	p[6] = transA * vector3d(a.max.x, a.max.y, a.min.z);
	p[7] = transA * vector3d(a.max.x, a.max.y, a.max.z);
	arot.min = arot.max = p[0];
	for (int i = 1; i < 8; i++)
		arot.Update(p[i]);
	return b.Intersects(arot);
}

/*
 * Intersect this Geom's edge BVH tree with geom b's triangle BVH tree.
 * Generate collision contacts.
 */
void Geom::CollideEdgesWithTrisOf(int &maxContacts, const Geom *b, const matrix4x4d &transTo, void (*callback)(CollisionContact *)) const
{
	PROFILE_SCOPED()
	struct stackobj {
		BVHNode *edgeNode;
		BVHNode *triNode;
	} stack[32];
	int stackpos = 0;

	stack[0].edgeNode = GetGeomTree()->GetEdgeTree()->GetRoot();
	stack[0].triNode = b->GetGeomTree()->GetTriTree()->GetRoot();

	while ((stackpos >= 0) && (maxContacts > 0)) {
		BVHNode *edgeNode = stack[stackpos].edgeNode;
		BVHNode *triNode = stack[stackpos].triNode;
		stackpos--;

		// does the edgeNode (with its aabb described in 6 planes transformed and rotated to
		// b's coordinates) intersect with one or other of b's child nodes?
		if (triNode->triIndicesStart || edgeNode->triIndicesStart) {
			// reached triangle leaf node or edge leaf node.
			// Intersect all edges under edgeNode with this leaf
			CollideEdgesTris(maxContacts, edgeNode, transTo, b, triNode, callback);
		} else {
			BVHNode *left = triNode->kids[0];
			BVHNode *right = triNode->kids[1];
			bool edgeNodeIsectsLeftChild = rotatedAabbIsectsNormalOne(edgeNode->aabb, transTo, left->aabb);
			bool edgeNodeIsectsRightChild = rotatedAabbIsectsNormalOne(edgeNode->aabb, transTo, right->aabb);
			//edgeNodeIsectsRightChild = edgeNodeIsectsLeftChild = true;
			if (edgeNodeIsectsRightChild) {
				if (edgeNodeIsectsLeftChild) {
					// isects both. split edgeNode and try again
					++stackpos;
					stack[stackpos].edgeNode = edgeNode->kids[0];
					stack[stackpos].triNode = triNode;
					++stackpos;
					stack[stackpos].edgeNode = edgeNode->kids[1];
					stack[stackpos].triNode = triNode;
				} else {
					// hits only right child. go down into that
					// side with same edge node
					++stackpos;
					stack[stackpos].edgeNode = edgeNode;
					stack[stackpos].triNode = triNode->kids[1];
				}
			} else if (edgeNodeIsectsLeftChild) {
				// hits only left child
				++stackpos;
				stack[stackpos].edgeNode = edgeNode;
				stack[stackpos].triNode = triNode->kids[0];
			} else {
				// hits none
			}
		}
	}
}

/*
 * Collide one edgeNode (all edges below it) of this Geom with the triangle
 * BVH of another geom (b), starting from btriNode.
 */
void Geom::CollideEdgesTris(int &maxContacts, const BVHNode *edgeNode, const matrix4x4d &transToB,
	const Geom *b, const BVHNode *btriNode, void (*callback)(CollisionContact *)) const
{
	// PROFILE_SCOPED() // verbose profiling only, this gets called a LOT
	if (maxContacts <= 0) return;
	if (edgeNode->triIndicesStart) {
		const GeomTree::Edge *edges = this->GetGeomTree()->GetEdges();
		int numContacts = 0;
		vector3f dir;
		isect_t isect;
		const std::vector<vector3f> &rVertices = GetGeomTree()->GetVertices();
		for (int i = 0; i < edgeNode->numTris; i++) {
			const int vtxNum = edges[edgeNode->triIndicesStart[i]].v1i;
			const vector3d v1 = transToB * vector3d(rVertices[vtxNum]);
			const vector3f _from(float(v1.x), float(v1.y), float(v1.z));

			vector3d _dir(
				double(edges[edgeNode->triIndicesStart[i]].dir.x),
				double(edges[edgeNode->triIndicesStart[i]].dir.y),
				double(edges[edgeNode->triIndicesStart[i]].dir.z));
			_dir = transToB.ApplyRotationOnly(_dir);
			dir = vector3f(&_dir.x);
			isect.dist = edges[edgeNode->triIndicesStart[i]].len;
			isect.triIdx = -1;

			b->GetGeomTree()->TraceRay(btriNode, _from, dir, &isect);

			if (isect.triIdx == -1) continue;
			numContacts++;
			const double depth = edges[edgeNode->triIndicesStart[i]].len - isect.dist;
			// in world coords
			CollisionContact contact;
			contact.pos = b->GetTransform() * (v1 + vector3d(&dir.x) * double(isect.dist));
			vector3f n = b->m_geomtree->GetTriNormal(isect.triIdx);
			contact.normal = vector3d(n.x, n.y, n.z);
			contact.normal = b->GetTransform().ApplyRotationOnly(contact.normal);
			contact.distance = isect.dist;

			contact.depth = depth;
			contact.triIdx = isect.triIdx;
			contact.userData1 = m_data;
			contact.userData2 = b->m_data;
			// contact geomFlag is bitwise OR of triangle's and edge's flags
			contact.geomFlag = b->m_geomtree->GetTriFlag(isect.triIdx) |
				edges[edgeNode->triIndicesStart[i]].triFlag;
			callback(&contact);
			if (--maxContacts <= 0) return;
		}
	} else {
		CollideEdgesTris(maxContacts, edgeNode->kids[0], transToB, b, btriNode, callback);
		CollideEdgesTris(maxContacts, edgeNode->kids[1], transToB, b, btriNode, callback);
	}
}
