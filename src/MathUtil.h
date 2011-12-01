#ifndef _MATHUTIL_H
#define _MATHUTIL_H

#include "vector3.h"

class MathUtil {
public:

	// random point on a sphere, distributed uniformly by area
	static vector3d RandomPointOnSphere(double minRadius, double maxRadius);
	static inline vector3d RandomPointOnSphere(double radius) { return RandomPointOnSphere(radius, radius); }

};

#endif
