// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _MATHUTIL_H
#define _MATHUTIL_H

#include "matrix3x3.h"
#include "matrix4x4.h"
#include "vector3.h"

namespace MathUtil {

	// random point on a sphere, distributed uniformly by area
	vector3d RandomPointOnSphere(double minRadius, double maxRadius);
	inline vector3d RandomPointOnSphere(double radius) { return RandomPointOnSphere(radius, radius); }

	vector3d RandomPointInCircle(double minRadius, double maxRadius);
	inline vector3d RandomPointInCircle(double radius) { return RandomPointInCircle(0.0, radius); }
	inline vector3d RandomPointOnCircle(double radius) { return RandomPointInCircle(radius, radius); }

	// interpolation, glsl style naming "mix"
	template <class T, class F>
	inline T mix(const T &v1, const T &v2, const F t)
	{
		return t * v2 + (F(1.0) - t) * v1;
	}
	template <class T, class F>
	inline T Lerp(const T &v1, const T &v2, const F t)
	{
		return mix(v1, v2, t);
	}

	inline float Dot(const vector3f &a, const vector3f &b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

	// unit vector orthogonal to given vector
	template <typename T>
	vector3<T> OrthogonalDirection(const vector3<T> &a)
	{
		vector3<T> b;
		if (std::abs(a.x) > std::abs(a.y)) {
			if (std::abs(a.y) > std::abs(a.z))
				b = vector3<T>(-a.y, a.x, 0);
			else
				b = vector3<T>(a.z, 0, -a.x);
		} else {
			if (std::abs(a.x) > std::abs(a.z))
				b = vector3<T>(-a.y, a.x, 0);
			else
				b = vector3<T>(0, -a.z, a.y);
		}
		return b.Normalized();
	}

	// matrix4x4f utility functions
	matrix4x4f Inverse(const matrix4x4f &);
	matrix4x4f InverseSlow(const matrix4x4f &);
	matrix4x4f Transpose(const matrix4x4f &);

	// matrix3x3f utility functions
	matrix3x3f Inverse(const matrix3x3f &);
	matrix3x3f Transpose(const matrix3x3f &);

	// distance from a line segment:
	float DistanceFromLineSegment(const vector3f &start, const vector3f &end, const vector3f &pos, bool &isWithinLineSegment);
	float DistanceFromLine(const vector3f &start, const vector3f &end, const vector3f &pos);

	inline static matrix3x3d LookAt(const vector3d eye, const vector3d target, const vector3d up)
	{
		const vector3d z = (eye - target).NormalizedSafe();
		const vector3d x = (up.Cross(z)).NormalizedSafe();
		const vector3d y = (z.Cross(x)).NormalizedSafe();

		return matrix3x3d::FromVectors(x, y, z);
	}

//#define TEST_MATHUTIL
#ifdef TEST_MATHUTIL
	bool TestDistanceFromLine();
#endif
} // namespace MathUtil

#endif
