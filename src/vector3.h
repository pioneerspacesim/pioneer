#ifndef _VECTOR3_H
#define _VECTOR3_H

#include <math.h>

template <typename T>
class vector3 {
	public:
	T x,y,z;

	vector3 () {}
	vector3 (T val): x(val), y(val), z(val) {}
	vector3 (T _x, T _y, T _z): x(_x), y(_y), z(_z) {}
	vector3 (const T vals[3]): x(vals[0]), y(vals[1]), z(vals[2]) {}

	T& operator [] (const int i) { return ((T *)this)[i]; }
	vector3 operator+ (const vector3 a) const { return vector3 (a.x+x, a.y+y, a.z+z); }
	vector3 &operator+= (const vector3 a) { x+=a.x; y+=a.y; z+=a.z; return *this; }
	vector3 &operator-= (const vector3 a) { x-=a.x; y-=a.y; z-=a.z; return *this; }
	vector3 &operator*= (const T a) { x*=a; y*=a; z*=a; return *this; }
	vector3 operator- (const vector3 a) const { return vector3 (x-a.x, y-a.y, z-a.z); }
	vector3 operator- () const { return vector3 (-x, -y, -z); }
	bool operator== (const vector3 a) const { return ((a.x==x)&&(a.y==y)&&(a.z==z)); }
	bool operator!= (const vector3 a) const { return ((a.x!=x)||(a.y!=y)||(a.z!=z)); }
	friend vector3 operator* (const vector3 a, const vector3 b) { return vector3 (a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x); }
	friend vector3 operator* (const vector3 a, const T scalar) { return vector3 (a.x*scalar, a.y*scalar, a.z*scalar); }
	friend vector3 operator* (const T scalar, const vector3 a) { return a*scalar; }

	static vector3 Cross (const vector3 a, const vector3 b) { return a*b; }
	static T Dot (const vector3 a, const vector3 b) { return a.x*b.x + a.y*b.y + a.z*b.z; }

	T Length () const {
		return sqrt (x*x + y*y + z*z);
	}

	void Normalize () {
		T l = 1.0f / sqrtf(x*x + y*y + z*z);
		x *= l;	y *= l;	z *= l;
	}

	static vector3 Normalize (const vector3 v) {
		vector3 r;
		T l = 1.0f / sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
		r.x = v.x * l;
		r.y = v.y * l;
		r.z = v.z * l;
		return r;
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
	void ArbRotate (const vector3 &__v, T ang)
	{
		vector3 t;
		T u = __v.x;
		T v = __v.y;
		T w = __v.z;
		T cos_a = cos (ang);
		T sin_a = sin (ang);
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

typedef vector3<float> vector3f;
typedef vector3<double> vector3d;

#endif /* _VECTOR3_H */
