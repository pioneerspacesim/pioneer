// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _FRUSTUM_H
#define _FRUSTUM_H

#include "math/Plane.h"
#include "matrix4x4.h"
#include "vector3.h"

namespace Graphics {

	// Frustum can be used for projecting points (3D to 2D) and testing
	// if a point lies inside the visible area
	// Its' internal projection matrix should, but does not have to, match
	// the one used for rendering
	class Frustum {
	public:
		// create for specified values
		Frustum(float width, float height, float fovAng, float nearClip, float farClip);
		Frustum(const matrix4x4d &modelview, const matrix4x4d &projection);

		// test if point (sphere) is in the frustum
		bool TestPoint(const vector3d &p, double radius) const;
		// test if point (sphere) is in the frustum, ignoring the far plane
		bool TestPointInfinite(const vector3d &p, double radius) const;

	private:
		Frustum(){};

		void InitFromMatrix(const matrix4x4d &m);

		Plane<double> m_planes[6];
	};

} // namespace Graphics

#endif
