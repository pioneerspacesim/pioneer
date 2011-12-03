#ifndef _MATHUTIL_H
#define _MATHUTIL_H

#include "vector3.h"

namespace MathUtil {

// random point on a sphere, distributed uniformly by area
vector3d RandomPointOnSphere(double minRadius, double maxRadius);
inline vector3d RandomPointOnSphere(double radius) { return RandomPointOnSphere(radius, radius); }

}

#endif
