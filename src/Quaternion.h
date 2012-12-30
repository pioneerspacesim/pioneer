// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _QUATERNION_H
#define _QUATERNION_H

#include <math.h>
#include "vector3.h"
#include "matrix4x4.h"

template <typename T>
class Quaternion {
public:
	T w,x,y,z;

	// Constructor definitions are outside class declaration to enforce that
	// only float and double versions are possible.
	Quaternion();
	Quaternion(T w, T x, T y, T z);
	// from angle and axis
	Quaternion(T ang, vector3<T> axis);
	Quaternion(const Quaternion<float > &o);
	Quaternion(const Quaternion<double> &o);

	void GetAxisAngle(T &angle, vector3<T> &axis) const {
		if (w > 1.0) *this = Normalized(); // if w>1 acos and sqrt will produce errors, this cant happen if quaternion is normalised
		angle = 2.0 * acos(w);
		double s = sqrt(1.0-w*w); // assuming quaternion normalised then w is less than 1, so term always positive.
		if (s < 0.001) { // test to avoid divide by zero, s is always positive due to sqrt
			// if s close to zero then direction of axis not important
			axis.x = x; // if it is important that axis is normalised then replace with x=1; y=z=0;
			axis.y = y;
			axis.z = z;
		} else {
			axis.x = x / s; // normalise axis
			axis.y = y / s;
			axis.z = z / s;
		}
	}
	// conjugate (inverse)
	friend Quaternion operator~ (const Quaternion &a) {
		Quaternion r;
		r.w = a.w;
		r.x = -a.x;
		r.y = -a.y;
		r.z = -a.z;
		return r;
	}
	friend Quaternion operator* (const Quaternion &a, const Quaternion &b) {
		Quaternion r;
		r.w = a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z;
		r.x = a.w*b.x + a.x*b.w + a.y*b.z - a.z*b.y;
		r.y = a.w*b.y - a.x*b.z + a.y*b.w + a.z*b.x;
		r.z = a.w*b.z + a.x*b.y - a.y*b.x + a.z*b.w;
		return r;
	}
	friend Quaternion operator* (const T s, const Quaternion &a) { return a*s; }
	friend Quaternion operator* (const Quaternion &a, const T s) {
		Quaternion r;
		r.w = a.w*s;
		r.x = a.x*s;
		r.y = a.y*s;
		r.z = a.z*s;
		return r;
	}
	friend Quaternion operator+ (const Quaternion &a, const Quaternion &b) {
		Quaternion r;
		r.w = a.w+b.w;
		r.x = a.x+b.x;
		r.y = a.y+b.y;
		r.z = a.z+b.z;
		return r;
	}
	friend Quaternion operator- (const Quaternion &a, const Quaternion &b) {
		Quaternion r;
		r.w = a.w-b.w;
		r.x = a.x-b.x;
		r.y = a.y-b.y;
		r.z = a.z-b.z;
		return r;
	}

	Quaternion Normalized() const {
		T l = 1.0 / sqrt (w*w + x*x + y*y + z*z);
		return Quaternion(w*l, x*l, y*l, z*l);
	}
	static T Dot (const Quaternion &a, const Quaternion &b) { return a.w*b.w + a.x*b.x + a.y*b.y + a.z*b.z; }

	template <typename U>
	static Quaternion FromMatrix3x3(const matrix3x3<U> &m) {
		Quaternion r;
		if (m[0] + m[4] + m[8] > 0.0f) {
			U t = m[0] + m[4] + m[8] + 1.0;
			U s = 0.5 / sqrt(t);
			r.w = s * t;
			r.z = (m[3] - m[1]) * s;
			r.y = (m[2] - m[6]) * s;
			r.x = (m[7] - m[5]) * s;
		} else if ((m[0] > m[4]) && (m[0] > m[8])) {
			U t = m[0] - m[4] - m[8] + 1.0;
			U s = 0.5 / sqrt(t);
			r.x = s * t;
			r.y = (m[1] + m[3]) * s;
			r.z = (m[2] + m[6]) * s;
			r.w = (m[7] - m[5]) * s;
		} else if (m[4] > m[8]) {
			U t = -m[0] + m[4] - m[8] + 1.0;
			U s = 0.5 / sqrt(t);
			r.w = (m[2] - m[6]) * s;
			r.x = (m[1] + m[3]) * s;
			r.y = s * t;
			r.z = (m[5] + m[7]) * s;
		} else {
			U t = -m[0] - m[4] + m[8] + 1.0;
			U s = 0.5 / sqrt(t);
			r.w = (m[3] - m[1]) * s;
			r.x = (m[2] + m[6]) * s;
			r.y = (m[7] + m[5]) * s;
			r.z = s * t;
		}
		return r;
	}

	template <typename U>
	matrix3x3<U> ToMatrix3x3() const {
		matrix3x3<U> m;
		U xx = x * x;
		U xy = x * y;
		U xz = x * z;
		U xw = x * w;

		U yy = y * y;
		U yz = y * z;
		U yw = y * w;

		U zz = z * z;
		U zw = z * w;

		m[0] = 1.0 - 2.0 * (yy + zz);
		m[1] =       2.0 * (xy - zw);
		m[2] =       2.0 * (xz + yw);

		m[3] =       2.0 * (xy + zw);
		m[4] = 1.0 - 2.0 * (xx + zz);
		m[5] =       2.0 * (yz - xw);

		m[6] =       2.0 * (xz - yw);
		m[7] =       2.0 * (yz + xw);
		m[8] = 1.0 - 2.0 * (xx + yy);
		return m;
	}
	/* normalized linear interpolation between 2 quaternions */
	static Quaternion Nlerp(const Quaternion &a, const Quaternion &b, T t) {
		//printf("a: %f,%f,%f,%f\n", a.x, a.y, a.z, a.w);
		//printf("b: %f,%f,%f,%f\n", b.x, b.y, b.z, b.w);
		return (a + t*(b-a)).Normalized();
	}

	//void Print() const {
	//	printf("%f,%f,%f,%f\n", w, x, y, z);
	//}
};

template<> inline Quaternion<float >::Quaternion() {}
template<> inline Quaternion<double>::Quaternion() {}
template<> inline Quaternion<float >::Quaternion(float  w_, float  x_, float  y_, float  z_): w(w_), x(x_), y(y_), z(z_) {}
template<> inline Quaternion<double>::Quaternion(double w_, double x_, double y_, double z_): w(w_), x(x_), y(y_), z(z_) {}

template<> inline Quaternion<float >::Quaternion(float  ang, vector3<float > axis) {
	const float halfAng = ang*0.5f;
	const float sinHalfAng = sin(halfAng);
	w = cos(halfAng);
	x = axis.x * sinHalfAng;
	y = axis.y * sinHalfAng;
	z = axis.z * sinHalfAng;
}
template<> inline Quaternion<double>::Quaternion(double ang, vector3<double> axis) {
	const double halfAng = ang*0.5;
	const double sinHalfAng = sin(halfAng);
	w = cos(halfAng);
	x = axis.x * sinHalfAng;
	y = axis.y * sinHalfAng;
	z = axis.z * sinHalfAng;
}

template<> inline Quaternion<float >::Quaternion(const Quaternion<float > &o): w(o.w), x(o.x), y(o.y), z(o.z) {}
template<> inline Quaternion<float >::Quaternion(const Quaternion<double> &o): w(float(o.w)), x(float(o.x)), y(float(o.y)), z(float(o.z)) {}
template<> inline Quaternion<double>::Quaternion(const Quaternion<float > &o): w(o.w), x(o.x), y(o.y), z(o.z) {}
template<> inline Quaternion<double>::Quaternion(const Quaternion<double> &o): w(o.w), x(o.x), y(o.y), z(o.z) {}

typedef Quaternion<float > Quaternionf;
typedef Quaternion<double> Quaterniond;

#endif /* _QUATERNION_H */
