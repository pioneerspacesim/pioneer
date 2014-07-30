// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _MATHUTIL_H
#define _MATHUTIL_H

#include "vector3.h"

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

}

#endif
