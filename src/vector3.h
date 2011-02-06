#ifndef _VECTOR3_H
#define _VECTOR3_H

#include <math.h>
#include <stdio.h>

template <typename T>
class vector3 {
	public:
	T x,y,z;

	vector3 () {}
	template<typename Q> vector3 (const vector3<Q> &v): x((T)v.x), y((T)v.y), z((T)v.z) {}
	vector3 (T val): x(val), y(val), z(val) {}
	vector3 (T _x, T _y, T _z): x(_x), y(_y), z(_z) {}
	template<typename Q> vector3 (const Q vals[3]): x((T)vals[0]), y((T)vals[1]), z((T)vals[2]) {}

	const T& operator [] (const size_t i) const { return ((const T *)this)[i]; }
	T& operator [] (const size_t i) { return ((T *)this)[i]; }
	vector3 operator+ (const vector3 a) const { return vector3 (a.x+x, a.y+y, a.z+z); }
	vector3 &operator+= (const vector3 a) { x+=a.x; y+=a.y; z+=a.z; return *this; }
	vector3 &operator-= (const vector3 a) { x-=a.x; y-=a.y; z-=a.z; return *this; }
	vector3 &operator*= (const float a) { x*=a; y*=a; z*=a; return *this; }
	vector3 &operator*= (const double a) { x*=a; y*=a; z*=a; return *this; }
	vector3 &operator/= (const float a) { const T inva = (T)(1.0/a); x*=inva; y*=inva; z*=inva; return *this; }
	vector3 &operator/= (const double a) { const T inva = (T)(1.0/a); x*=inva; y*=inva; z*=inva; return *this; }
	vector3 operator- (const vector3 a) const { return vector3 (x-a.x, y-a.y, z-a.z); }
	vector3 operator- () const { return vector3 (-x, -y, -z); }
	bool operator== (const vector3 a) const { return ((a.x==x)&&(a.y==y)&&(a.z==z)); }
	bool operator!= (const vector3 a) const { return ((a.x!=x)||(a.y!=y)||(a.z!=z)); }
//	friend vector3 operator* (const vector3 a, const vector3 b) 
//	{ return vector3 (a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x); }
	template<typename Q> friend vector3 operator* (const vector3 a, const Q scalar) { return vector3 ((T)(a.x*scalar), (T)(a.y*scalar), (T)(a.z*scalar)); }
	template<typename Q> friend vector3 operator* (const Q scalar, const vector3 a) { return a*scalar; }
	template<typename Q> friend vector3 operator/ (const vector3 a, const Q scalar) { const T inv = (T)(1.0/scalar); return vector3 (a.x*inv, a.y*inv, a.z*inv); }
	// why did i ever make these awful static functions...
	static vector3 Cross (const vector3 a, const vector3 b)
		{ return vector3 (a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x); }
	static T Dot (const vector3 a, const vector3 b) { return a.x*b.x + a.y*b.y + a.z*b.z; }

	vector3 Cross(const vector3 b) const {
		return vector3 (y*b.z - z*b.y, z*b.x - x*b.z, x*b.y - y*b.x);
	}
	T Dot(const vector3 b) const {
		return x*b.x + y*b.y + z*b.z;
	}

	T Length () const {
		return sqrt (x*x + y*y + z*z);
	}

	vector3 Normalized() const {
		T l = 1.0f / sqrt (x*x + y*y + z*z);
		return vector3(x*l, y*l, z*l);
	}

	/* Rotate this vector about point o, in axis defined by v. */
	void ArbRotateAroundPoint (const vector3 &o, const vector3 &__v, T ang)
	{
		vector3 t;
		T a = o.x;
		T b = o.y;
		T c = o.z;
		T u = __v.x;
		T v = __v.y;
		T w = __v.z;
		T cos_a = (T) cos (ang);
		T sin_a = (T) sin (ang);
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
	void ArbRotate (const vector3 &__v, T ang)
	{
		vector3 t;
		T u = __v.x;
		T v = __v.y;
		T w = __v.z;
		T cos_a = (T) cos (ang);
		T sin_a = (T) sin (ang);
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

	void Print() const {
		printf("v(%f,%f,%f)\n", x, y, z);
	}
};


typedef vector3<float> vector3f;
typedef vector3<double> vector3d;

#endif /* _VECTOR3_H */
