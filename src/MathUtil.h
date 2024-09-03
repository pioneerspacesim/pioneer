// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _MATHUTIL_H
#define _MATHUTIL_H

#include "matrix3x3.h"
#include "matrix4x4.h"
#include "vector3.h"
#include "fixed.h"

#include <cmath>

template <class T>
inline const T &Clamp(const T &x, const T &min, const T &max) { return x > max ? max : (x < min ? min : x); }

inline constexpr double DEG2RAD(double x) { return x * (M_PI / 180.); }
inline constexpr float DEG2RAD(float x) { return x * (float(M_PI) / 180.f); }
inline constexpr double RAD2DEG(double x) { return x * (180. / M_PI); }
inline constexpr float RAD2DEG(float x) { return x * (180.f / float(M_PI)); }

static inline Sint64 isqrt(Sint64 a)
{
	// replace with cast from sqrt below which is between x7.3 (win32, Debug) & x15 (x64, Release) times faster
	return static_cast<int64_t>(sqrt(static_cast<double>(a)));
}

static inline Sint64 isqrt(fixed v)
{
	Sint64 ret = 0;
	Sint64 s;
	Sint64 ret_sq = -v.v - 1;
	for (s = 62; s >= 0; s -= 2) {
		Sint64 b;
		ret += ret;
		b = ret_sq + ((2 * ret + 1) << s);
		if (b < 0) {
			ret_sq = b;
			ret++;
		}
	}
	return ret;
}

// add a few things that MSVC is missing
#if defined(_MSC_VER) && (_MSC_VER < 1800)

// round & roundf. taken from http://cgit.freedesktop.org/mesa/mesa/tree/src/gallium/auxiliary/util/u_math.h
static inline double round(double x)
{
	return x >= 0.0 ? floor(x + 0.5) : ceil(x - 0.5);
}

static inline float roundf(float x)
{
	return x >= 0.0f ? floorf(x + 0.5f) : ceilf(x - 0.5f);
}
#endif /* _MSC_VER < 1800 */

static inline Uint32 ceil_pow2(Uint32 v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}

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
