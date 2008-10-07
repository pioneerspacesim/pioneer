#ifndef _GEOM_H
#define _GEOM_H

#include "../matrix4x4.h"
#include "../vector3.h"

class GeomTree;
class isect_t;
class CollisionContact;

class Geom {
public:
	Geom(GeomTree *);
	void MoveTo(const matrix4x4d &m);
	void MoveTo(const matrix4x4d &m, const vector3d pos);
	const matrix4x4d &GetInvTransform() { return m_invOrient; }
	const matrix4x4d &GetTransform() { return m_orient[m_orientIdx]; }
	void Enable() { m_active = true; }
	void Disable() { m_active = false; }
	bool IsEnabled() { return m_active; }
	GeomTree *GetGeomTree() { return m_geomtree; }
	void Collide(Geom *b, void (*callback)(CollisionContact*));
	void SetUserData(void *d) { m_data = d; }
	void *GetUserData() { return m_data; }
private:
	// double-buffer position so we can keep previous position
	matrix4x4d m_orient[2], m_invOrient;
	int m_orientIdx;
	bool m_active;
	GeomTree *m_geomtree;
	void *m_data;
};

#endif /* _GEOM_H */
