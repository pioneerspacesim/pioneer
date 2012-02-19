#ifndef _VECTOR3_H
#define _VECTOR3_H

#include <math.h>
#include <stdio.h>
#include "FloatComparison.h"

// Need this pragma due to operator[] implementation.
#pragma pack(4)

template <typename T> struct other_floating_type {};
template <> struct other_floating_type<float> { typedef double type; };
template <> struct other_floating_type<double> { typedef float type; };

template <typename T>
class vector3 {
public:
	T x,y,z;

	// Constructor definitions are outside class declaration to enforce that
	// only float and double versions are possible.
	vector3();
	vector3(const vector3<T> &v);
	vector3(const T  vals[3]);
	vector3(T val);
	vector3(T _x, T _y, T _z);

	// disallow implicit conversion between floating point sizes
	explicit vector3(const vector3<typename other_floating_type<T>::type> &v);
	explicit vector3(const typename other_floating_type<T>::type vals[3]);

	const T& operator[](const size_t i) const { return (const_cast<const T *>(&x))[i]; }
	T& operator[](const size_t i) { return (&x)[i]; }

	vector3 operator+(const vector3 &a) const { return vector3 (a.x+x, a.y+y, a.z+z); }
	vector3 &operator+=(const vector3 &a) { x+=a.x; y+=a.y; z+=a.z; return *this; }
	vector3 &operator-=(const vector3 &a) { x-=a.x; y-=a.y; z-=a.z; return *this; }
	vector3 &operator*=(const float a) { x*=a; y*=a; z*=a; return *this; }
	vector3 &operator*=(const double a) { x*=a; y*=a; z*=a; return *this; }
	vector3 &operator/=(const float a) { const T inva = T(1.0/a); x*=inva; y*=inva; z*=inva; return *this; }
	vector3 &operator/=(const double a) { const T inva = T(1.0/a); x*=inva; y*=inva; z*=inva; return *this; }
	vector3 operator-(const vector3 &a) const { return vector3(x-a.x, y-a.y, z-a.z); }
	vector3 operator-() const { return vector3(-x, -y, -z); }

	bool ExactlyEqual(const vector3 &a) const {
		return is_equal_exact(a.x, x) && is_equal_exact(a.y, y) && is_equal_exact(a.z, z);
	}

	friend vector3 operator*(const vector3 &a, const float  scalar) { return vector3(T(a.x*scalar), T(a.y*scalar), T(a.z*scalar)); }
	friend vector3 operator*(const vector3 &a, const double scalar) { return vector3(T(a.x*scalar), T(a.y*scalar), T(a.z*scalar)); }
	friend vector3 operator*(const float  scalar, const vector3 &a) { return a*scalar; }
	friend vector3 operator*(const double scalar, const vector3 &a) { return a*scalar; }
	friend vector3 operator/(const vector3 &a, const float  scalar) { const T inv = 1.0/scalar; return vector3(a.x*inv, a.y*inv, a.z*inv); }
	friend vector3 operator/(const vector3 &a, const double scalar) { const T inv = 1.0/scalar; return vector3(a.x*inv, a.y*inv, a.z*inv); }

	vector3 Cross(const vector3 &b) const { return vector3 (y*b.z - z*b.y, z*b.x - x*b.z, x*b.y - y*b.x); }
	T Dot(const vector3 &b) const { return x*b.x + y*b.y + z*b.z; }
	T Length() const { return sqrt (x*x + y*y + z*z); }
	T LengthSqr() const { return x*x + y*y + z*z; }
	vector3 Normalized() const { const T l = 1.0f / sqrt(x*x + y*y + z*z); return vector3(x*l, y*l, z*l); }
	vector3 NormalizedSafe() const {
		const T lenSqr = x*x + y*y + z*z;
		if (lenSqr < 1e-18) // sqrt(lenSqr) < 1e-9
			return vector3(1,0,0);
		else {
			const T l = sqrt(lenSqr);
			return vector3(x/l, y/l, z/l);
		}
	}

	void Print() const { printf("v(%f,%f,%f)\n", x, y, z); }

	/* Rotate this vector about point o, in axis defined by v. */
	void ArbRotateAroundPoint(const vector3 &o, const vector3 &__v, T ang) {
		vector3 t;
		T a = o.x;
		T b = o.y;
		T c = o.z;
		T u = __v.x;
		T v = __v.y;
		T w = __v.z;
		T cos_a = cos (ang);
		T sin_a = sin (ang);
		T inv_poo = 1.0f/(u*u+v*v+w*w);
		t.x = a*(v*v+w*w)+u*(-b*v-c*w+u*x+v*y+w*z)+(-a*(v*v+w*w)+u*(b*v+c*w-v*y-w*z)+(v*v+w*w)*x)*cos_a+
			sqrtf (u*u+v*v+w*w)*(-c*v+b*w-w*y+v*z)*sin_a;
		t.x *= inv_poo;
		t.y = b*(u*u+w*w)+v*(-a*u-c*w+u*x+v*y+w*z)+(-b*(u*u+w*w)+v*(a*u+c*w-u*x-w*z)+(u*u+w*w)*y)*cos_a+
			sqrtf (u*u+v*v+w*w)*(-c*u-a*w+w*x-u*z)*sin_a;
		t.y *= inv_poo;
		t.z = c*(u*u+v*v)+w*(-a*u+b*v+u*x+v*y+w*z)+(-c*(u*u+v*v)+w*(a*u+b*v-u*x-v*y)+(u*u+v*v)*z)*cos_a+
			sqrtf (u*u+v*v+w*w)*(-b*u+a*v-v*x+u*y)*sin_a;
		t.z *= inv_poo;
		*this = t;
	}

	/* Rotate this vector about origin, in axis defined by v. */
	void ArbRotate(const vector3 &__v, T ang) {
		vector3 t;
		T u = __v.x;
		T v = __v.y;
		T w = __v.z;
		T cos_a = cos(ang);
		T sin_a = sin(ang);
		T inv_poo = 1.0f/(u*u+v*v+w*w);
		t.x = u*(u*x+v*y+w*z)+(u*(-v*y-w*z)+(v*v+w*w)*x)*cos_a+
			sqrtf (u*u+v*v+w*w)*(-w*y+v*z)*sin_a;
		t.x *= inv_poo;
		t.y = v*(u*x+v*y+w*z)+(v*(-u*x-w*z)+(u*u+w*w)*y)*cos_a+
			sqrtf (u*u+v*v+w*w)*(w*x-u*z)*sin_a;
		t.y *= inv_poo;
		t.z = w*(u*x+v*y+w*z)+(w*(-u*x-v*y)+(u*u+v*v)*z)*cos_a+
			sqrtf (u*u+v*v+w*w)*(-v*x+u*y)*sin_a;
		t.z *= inv_poo;
		*this = t;
	}
};

// These are here in this manner to enforce that only float and double versions are possible.
template<> inline vector3<float >::vector3() {}
template<> inline vector3<double>::vector3() {}
template<> inline vector3<float >::vector3(const vector3<float > &v): x(v.x), y(v.y), z(v.z) {}
template<> inline vector3<float >::vector3(const vector3<double> &v): x(float(v.x)), y(float(v.y)), z(float(v.z)) {}
template<> inline vector3<double>::vector3(const vector3<float > &v): x(v.x), y(v.y), z(v.z) {}
template<> inline vector3<double>::vector3(const vector3<double> &v): x(v.x), y(v.y), z(v.z) {}
template<> inline vector3<float >::vector3(float  val): x(val), y(val), z(val) {}
template<> inline vector3<double>::vector3(double val): x(val), y(val), z(val) {}
template<> inline vector3<float >::vector3(float  _x, float  _y, float  _z): x(_x), y(_y), z(_z) {}
template<> inline vector3<double>::vector3(double _x, double _y, double _z): x(_x), y(_y), z(_z) {}
template<> inline vector3<float >::vector3(const float  vals[3]): x(vals[0]), y(vals[1]), z(vals[2]) {}
template<> inline vector3<float >::vector3(const double vals[3]): x(float(vals[0])), y(float(vals[1])), z(float(vals[2])) {}
template<> inline vector3<double>::vector3(const float  vals[3]): x(vals[0]), y(vals[1]), z(vals[2]) {}
template<> inline vector3<double>::vector3(const double vals[3]): x(vals[0]), y(vals[1]), z(vals[2]) {}

#pragma pack()

typedef vector3<float > vector3f;
typedef vector3<double> vector3d;

#endif /* _VECTOR3_H */
