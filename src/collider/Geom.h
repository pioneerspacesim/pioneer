// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GEOM_H
#define _GEOM_H

#include "../matrix4x4.h"
#include "../vector3.h"
#include "CollisionContact.h"
#include <list>
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
	 * their owning ship)
	*/
	bool IsAChild(Geom* child) {
		return (std::find(m_children.begin(), m_children.end(), child) != m_children.end());
	}
	void AddChild(Geom* child) { if (!IsAChild(child)) {m_children.push_back(child); child->m_parent = this;} }
	void RemoveChild(Geom* child) { child->m_parent = nullptr; m_children.remove(child); }
	void RemoveAllChildren() {
		for (std::list<Geom*>::iterator it=m_children.begin(); it!=m_children.end(); ++it)
			(*it)->m_parent = nullptr;
		m_children.clear();
	}
	Geom* GetParent() { return m_parent; }

	/* If a Geom have a central hole (Aka: orbital
	 * SpaceStation) then you could skip deep collision
	 * test if objects are inside a cylinder and
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
	void CollideEdgesWithTrisOf(int &maxContacts, Geom *b, const matrix4x4d &transTo, void (*callback)(CollisionContact*));
	void CollideEdgesTris(int &maxContacts, const BVHNode *edgeNode, const matrix4x4d &transToB,
		Geom *b, const BVHNode *btriNode, void (*callback)(CollisionContact*));
	int m_mailboxIndex; // used to avoid duplicate collisions
	// double-buffer position so we can keep previous position
	matrix4x4d m_orient, m_invOrient;
	bool m_active;
	const GeomTree *m_geomtree;
	void *m_data;
	int m_group;

	Geom* m_parent;
	std::list<Geom*> m_children;
	float m_central_hole_diameter;
	float m_central_hole_minz, m_central_hole_maxz;
	bool m_have_central_hole, m_central_hole_dock;
};

#endif /* _GEOM_H */
