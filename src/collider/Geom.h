#ifndef _GEOM_H
#define _GEOM_H

#include "../matrix4x4.h"
#include "../vector3.h"

class GeomTree;

class Geom {
public:
	Geom(GeomTree *);
	void SetPosition(vector3d pos);
	void SetOrientation(const matrix4x4d &rot);
	const matrix4x4d &GetInvTransform() { return m_invOrient; }
	void Enable() { m_active = true; }
	void Disable() { m_active = false; }
	bool IsEnabled() { return m_active; }
	GeomTree *GetGeomTree() { return m_geomtree; }
private:
	matrix4x4d m_orient, m_invOrient;
	bool m_active;
	GeomTree *m_geomtree;
};

#endif /* _GEOM_H */
