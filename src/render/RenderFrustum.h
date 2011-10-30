#ifndef _FRUSTUM_H
#define _FRUSTUM_H

#include "libs.h"
#include "vector3.h"

namespace Render {

class Frustum {
public:
	static Frustum FromGLState();

	// create for specified values
	Frustum(float width, float height, float fovAng);

	// apply the saved frustum
	void Enable();

	// restore the previous state
	void Disable();

	// test if point (sphere) is in the frustum
	bool TestPoint(const vector3d &p, double radius) const;
	// test if point (sphere) is in the frustum, ignoring the far plane
	bool TestPointInfinite(const vector3d &p, double radius) const;

	// project a point onto the near plane (typically the screen)
	bool ProjectPoint(const vector3d &in, vector3d &out) const;

private:
	// create from current gl state
	Frustum();

	void InitFromMatrix(const matrix4x4d &m);
	void InitFromGLState();

	struct Plane {
		double a, b, c, d;
		double DistanceToPoint(const vector3d &p) const {
			return a*p.x + b*p.y + c*p.z + d;
		}
	};

	matrix4x4d m_projMatrix;
	matrix4x4d m_modelMatrix;
	Plane m_planes[6];
};

}

#endif
