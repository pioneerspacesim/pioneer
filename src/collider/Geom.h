// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GEOM_H
#define _GEOM_H

#include "../matrix4x4.h"
#include "../vector3.h"
#include "CollisionContact.h"

class GeomTree;
struct isect_t;
struct Sphere;
struct BVHNode;

class Geom {
public:
	Geom(const GeomTree *);
	void MoveTo(const matrix4x4d &m);
	void MoveTo(const matrix4x4d &m, const vector3d &pos);
	const matrix4x4d &GetInvTransform() const { return m_invOrient; }
	const matrix4x4d &GetTransform() const { return m_orient; }
	matrix4x4d GetRotation() const;
	vector3d GetPosition() const;
	void Enable() { m_active = true; }
	void Disable() { m_active = false; }
	bool IsEnabled() { return m_active; }
	const GeomTree *GetGeomTree() { return m_geomtree; }
	void Collide(Geom *b, void (*callback)(CollisionContact*));
	void CollideSphere(Sphere &sphere, void (*callback)(CollisionContact*));
	void SetUserData(void *d) { m_data = d; }
	void *GetUserData() { return m_data; }
	void SetMailboxIndex(int idx) { m_mailboxIndex = idx; }
	int GetMailboxIndex() const { return m_mailboxIndex; }
	void SetGroup(int g) { m_group = g; }
	int GetGroup() const { return m_group; }

	matrix4x4d m_animTransform;

private:
	void CollideEdgesWithTrisOf(int &maxContacts, Geom *b, const matrix4x4d &transTo, void (*callback)(CollisionContact*));
	void CollideEdgesTris(int &maxContacts, const BVHNode *edgeNode, const matrix4x4d &transToB,
		Geom *b, const BVHNode *btriNode, void (*callback)(CollisionContact*));
	int m_mailboxIndex; // used to avoid duplicate collisions
	void CollideEdges(const matrix4x4d &transToB, Geom *b, void (*callback)(CollisionContact*));
	// double-buffer position so we can keep previous position
	matrix4x4d m_orient, m_invOrient;
	bool m_active;
	const GeomTree *m_geomtree;
	void *m_data;
	int m_group;
};

#endif /* _GEOM_H */
