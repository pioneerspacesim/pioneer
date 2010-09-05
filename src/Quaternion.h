#ifndef _QUATERNION_H
#define _QUATERNION_H

#include <math.h>
#include "vector3.h"
#include "matrix4x4.h"

#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif /* MAX */

template <typename T>
class Quaternion {
public:
	T w,x,y,z;
	Quaternion() {}
	Quaternion(T w, T x, T y, T z): w(w), x(x), y(y), z(z) {}
	// from angle and axis
	Quaternion(T ang, vector3<T> axis) {
		const T halfAng = ang*(T)0.5;
		const T sinHalfAng = sin(halfAng);
		w = cos(halfAng);
		x = axis.x * sinHalfAng;
		y = axis.y * sinHalfAng;
		z = axis.z * sinHalfAng;
	}
	void GetAxisAngle(T &angle, vector3<T> &axis) {
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
	Quaternion Normalized() const {
		T l = 1.0 / sqrt (w*w + x*x + y*y + z*z);
		return Quaternion(w*l, x*l, y*l, z*l);
	}
	static T Dot (const Quaternion &a, const Quaternion &b) { return a.w*b.w + a.x*b.x + a.y*b.y + a.z*b.z; }

	template <typename U>
	static Quaternion FromMatrix4x4(const matrix4x4<U> &m) {
		Quaternion r;
		U t = m[0] + m[5] + m[10];
		if (t > 0) {
			U s = 0.5 / sqrt(t+1.0);
			r.w = 0.25 / s;
			r.x = (m[6] - m[9]) * s;
			r.y = (m[8] - m[2]) * s;
			r.z = (m[1] - m[4]) * s;
		} else if ((m[0] > m[5]) && (m[0] > m[10])) {
			U s = sqrt(1.0 + m[0] - m[5] - m[10]) * 2.0;
			r.w = (m[9] - m[6]) / s;
			r.x = 0.25 * s;
			r.y = (m[4] + m[1]) / s;
			r.z = (m[8] + m[2]) / s;
		} else if (m[5] > m[10]) {
			U s = sqrt(1.0 + m[5] - m[0] - m[10]) * 2.0;
			r.w = (m[8] - m[2]) / s;
			r.x = (m[4] + m[1]) / s;
			r.y = 0.25 * s;
			r.z = (m[9] + m[6]) / s;
		} else {
			U s = sqrt(1.0 + m[10] - m[0] - m[5]) * 2.0;
			r.w = (m[4] - m[1]) / s;
			r.x = (m[8] + m[2]) / s;
			r.y = (m[6] + m[9]) / s;
			r.z = 0.25 * s;
		}
		return r;
	}

	template <typename U>
	matrix4x4<U> ToMatrix4x4() const {
		matrix4x4<U> m;
		U x2 = 2.0f * x,  y2 = 2.0f * y,  z2 = 2.0f * z;
		U xy = x2 * y,  xz = x2 * z;
		U yy = y2 * y,  yw = y2 * w;
		U zw = z2 * w,  zz = z2 * z;

		m[ 0] = 1.0f - ( yy + zz );
		m[ 4] = ( xy - zw );
		m[ 8] = ( xz + yw );
		m[12] = 0.0f;

		U xx = x2 * x,  xw = x2 * w,  yz = y2 * z;
		m[ 1] = ( xy +  zw );
		m[ 5] = 1.0f - ( xx + zz );
		m[ 9] = ( yz - xw );
		m[13] = 0.0f;

		m[ 2] = ( xz - yw );
		m[ 6] = ( yz + xw );
		m[10] = 1.0f - ( xx + yy );  
		m[14] = 0.0f;  

		m[ 3] = 0.0f;  
		m[ 7] = 0.0f;   
		m[11] = 0.0f;   
		m[15] = 1.0f;

#if 0
		m[0] = w*w+x*x-y*y-z*z;
		m[1] = 2.0*x*y+2.0*w*z;
		m[2] = 2.0*x*z-2.0*w*y;
		m[3] = 0;
		m[4] = 2.0*x*y - 2.0*w*z;
		m[5] = w*w-x*x+y*y-z*z;
		m[6] = 2.0*y*z - 2.0*w*x;
		m[7] = 0;
		m[8] = 2.0*x*z + 2.0*w*y;
		m[9] = 2.0*y*z - 2.0*w*x;
		m[10] = w*w-x*x-y*y+z*z;
		m[11] = m[12] = m[13] = m[14] = 0;
		m[15] = 1.0;
#endif
		return m;
	}
	/* spherical linear interpolation between 2 quaternions */
	static Quaternion Slerp(const Quaternion &a, const Quaternion &b, T t) {
		T w1, w2;
		T cosTheta = Quaternion::Dot(a,b);
		T theta    = (T)acos(cosTheta);
		T sinTheta = (T)sin(theta);

		if (sinTheta > 0.001) {
			w1 = sin((1.0f-t)*theta) / sinTheta;
			w2 = sin(t*theta) / sinTheta;
		} else {
			// a ~= b
			w1 = 1.0f - t;
			w2 = t;
		}
		return a*w1 + b*w2;
	}

	//void Print() const {
	//	printf("%f,%f,%f,%f\n", w, x, y, z);
	//}
};
typedef Quaternion<float> Quaternionf;
typedef Quaternion<double> Quaterniond;

#endif /* _QUATERNION_H */
