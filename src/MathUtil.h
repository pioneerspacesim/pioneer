// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _MATHUTIL_H
#define _MATHUTIL_H

#include "vector3.h"
#include "matrix3x3.h"
#include "matrix4x4.h"

namespace MathUtil {

	// random point on a sphere, distributed uniformly by area
	vector3d RandomPointOnSphere(double minRadius, double maxRadius);
	inline vector3d RandomPointOnSphere(double radius) { return RandomPointOnSphere(radius, radius); }

	vector3d RandomPointInCircle(double minRadius, double maxRadius);
	inline vector3d RandomPointInCircle(double radius) { return RandomPointInCircle(0.0, radius); }
	inline vector3d RandomPointOnCircle(double radius) { return RandomPointInCircle(radius, radius); }

	// interpolation, glsl style naming "mix"
	template< class T, class F >
	T mix(const T& v1, const T& v2, const F t){ 
		return t*v2 + (F(1.0)-t)*v1;
	}

	inline float Dot(const vector3f &a, const vector3f &b) { return a.x*b.x + a.y*b.y + a.z*b.z; }

	// matrix4x4f utility functions
	matrix4x4f Inverse(const matrix4x4f &);
	matrix4x4f InverseSlow(const matrix4x4f &);
	matrix4x4f Transpose(const matrix4x4f &);
	
	// matrix3x3f utility functions
	matrix3x3f Inverse(const matrix3x3f &);
	matrix3x3f Transpose(const matrix3x3f &);
}

#endif
