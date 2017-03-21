// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GEOM_H
#define _GEOM_H

#include "../matrix4x4.h"
#include "../vector3.h"
#include "CollisionContact.h"
#include <assert.h>
#include <algorithm>

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

	/* If a Geom is child of another Geom, then
	 * you must skip this collision (eg: ships docked
	 * with their starport or "subsystem" (eg guns) with
	 * their owning ship), so handle of child should be
	 * stored until you not tell this geom is no longer
	 * a child
	*/
	bool IsAChildOf(Geom* child) {
		return child->GetGroup()==GetGroup();
	}

	void AddChild(Geom* child) {
		// Check if is already a child
		// PS: assert is needed because if called, it means
		// you are calling this twice: if you get here trying to
		// understand why this assert fail, then: the problem
		// is NOT here
		assert(child->m_old_group==0);
		// Store old group on child itself
		child->StoreOldGroup(child->GetGroup());
		// Use the group of parent (...this)
		child->SetGroup(GetGroup());
	}

	void RemoveChild(Geom* child) {
		// Check if is already a child
		assert(child->GetGroup()==GetGroup());
		// Restore olg group
		child->SetGroup(child->m_old_group);
		// Zero old group
		child->m_old_group=0;
	}

	/* If a Geom have a central hole (Aka: orbital
	 * SpaceStation) then you could skip deep collision
	 * test if objects are inside a cylinder;
	 * you could also specify to trigger docking
	 * sequence if dock is true.
	 * Simple math (also because orbitals are Frame, so
	 * their pos is origin ;) ) against the whole check
	 * TODO: Set direction, pos and number. This could
	 * lead to more math than the whole test... but
	 * this could be good for huge (and "void") models
	*/
	void SetCentralHole(float diameter, int minz, int maxz, bool dock);
	bool HaveHole() { return m_have_central_hole; }
	float GetCentralHole() { return m_central_hole_diameter; }
	bool CheckInsideHole(Geom* b, void (*callback)(CollisionContact*));

private:
	void StoreOldGroup(int old_g) { m_old_group=old_g; }
	void CollideEdgesWithTrisOf(int &maxContacts, Geom *b, const matrix4x4d &transTo, void (*callback)(CollisionContact*));
	void CollideEdgesTris(int &maxContacts, const BVHNode *edgeNode, const matrix4x4d &transToB,
		Geom *b, const BVHNode *btriNode, void (*callback)(CollisionContact*));
	int m_mailboxIndex; // used to avoid duplicate collisions
	// double-buffer position so we can keep previous position
	matrix4x4d m_orient, m_invOrient;
	bool m_active;
	const GeomTree *m_geomtree;
	void *m_data;
	int m_group, m_old_group;

	float m_central_hole_diameter;
	float m_central_hole_minz, m_central_hole_maxz;
	bool m_have_central_hole, m_central_hole_dock;
};

#endif /* _GEOM_H */
